#include "PictureView.h"

namespace util {

static char parseHexNibble(char ch) {
    int nibble = 0;
    if (ch >= '0' && ch <= '9') {
        nibble = ch - '0';
    } else if (ch >= 'a' && ch <= 'f') {
        nibble = ch - 'a' + 10;
    } else if (ch >= 'A' && ch <= 'F') {
        nibble = ch - 'A' + 10;
    } else {
        throw std::runtime_error("Unexpected data in picture source.");
    }
    assert(nibble >= 0 && nibble <= 15);
    return static_cast<char>(nibble);
}

static char parseHexByte(char hi, char lo) {
    char value = parseHexNibble(hi);
    value <<= 4;
    return static_cast<char>(value | parseHexNibble(lo));
}

Picture::Picture(int width, int height) : cells(width * height, PictureCell{}), width(width), height(height) {}

Picture::Picture(const std::string& source) {
    std::istringstream s{ source };
    s >> std::hex;
    std::string pictureKeyword;
    std::string sizeSeparator;
    s >> pictureKeyword >> name >> sizeSeparator >> width >> height;
    if (pictureKeyword != "picture" || sizeSeparator != "Z") {
        throw std::runtime_error("Unexpected data in picture source.");
    }
    cells = { static_cast<size_t>(width * height), PictureCell{} };
    uint32_t fg = 0;
    uint32_t bg = 0;
    int transparent = 0;
    std::string utf8 = " ";
    for (auto y = 0; y < height; y++) {
        for (auto x = 0; x < width; x++) {
            std::string command;
            s >> command;

            auto changesBitMask = command.at(0) - 'A';
            auto charChanged = (changesBitMask & 0x01) != 0;
            auto transparentChanged = (changesBitMask & 0x02) != 0;
            auto fgChanged = (changesBitMask & 0x04) != 0;
            auto bgChanged = (changesBitMask & 0x08) != 0;

            if (charChanged) {
                std::string utf8Hex;
                s >> utf8Hex;
                if ((utf8Hex.size() % 2) != 0) {
                    throw std::runtime_error("Unexpected data in picture source.");
                }
                utf8 = "";
                for (size_t i = 0; i < utf8Hex.size(); i += 2) {
                    auto ch = parseHexByte(utf8Hex.at(i), utf8Hex.at(i + 1));
                    utf8 += ch;
                }
            }
            if (transparentChanged) {
                s >> transparent;
            }
            if (fgChanged) {
                s >> fg;
            }
            if (bgChanged) {
                s >> bg;
            }

            auto& cell = cells.at(y * width + x);
            cell.transparent = transparent != 0;
            cell.colorAttr = { TColorRGB(fg), TColorRGB(bg) };
            cell.ch = utf8;
        }
    }
}

std::string Picture::exportToString() {
    std::ostringstream s;
    s << "picture " << name << "\n";
    auto lineStart = s.tellp();
    s << "Z " << std::hex << width << " " << height;
    uint32_t previousFg = 0;
    uint32_t previousBg = 0;
    auto previousTransparent = false;
    std::string previousChar = " ";
    std::function<void()> newlineOrSpace = [&s, &lineStart]() -> void {
        if (s.tellp() - lineStart >= 110) {
            s << "\n";
            lineStart = s.tellp();
        } else {
            s << " ";
        }
    };
    for (auto y = 0; y < height; y++) {
        for (auto x = 0; x < width; x++) {
            auto n = y * width + x;
            auto& cell = cells.at(n);
            auto fg = static_cast<uint32_t>(getFore(cell.colorAttr).asRGB());
            auto bg = static_cast<uint32_t>(getBack(cell.colorAttr).asRGB());

            newlineOrSpace();

            auto changesBitMask = (previousChar != cell.ch ? 0x01 : 0) |
                (previousTransparent != cell.transparent ? 0x02 : 0) | (previousFg != fg ? 0x04 : 0) |
                (previousBg != bg ? 0x08 : 0);
            char command = static_cast<char>('A' + changesBitMask);

            s << command;

            if (previousChar != cell.ch) {
                newlineOrSpace();
                for (auto ch : cell.ch) {
                    s << std::setw(2) << std::setfill('0') << static_cast<int>(ch & 0xFF);
                }
            }

            if (previousTransparent != cell.transparent) {
                newlineOrSpace();
                s << (cell.transparent ? 1 : 0);
            }

            if (previousFg != fg) {
                newlineOrSpace();
                s << fg;
            }

            if (previousBg != bg) {
                newlineOrSpace();
                s << bg;
            }

            previousChar = cell.ch;
            previousTransparent = cell.transparent;
            previousFg = fg;
            previousBg = bg;
        }
    }
    s << "\nend picture\n";
    return s.str();
}

void Picture::resize(int newWidth, int newHeight) {
    if (newWidth < 1) {
        newWidth = 1;
    }
    if (newHeight < 1) {
        newHeight = 1;
    }
    std::vector<PictureCell> newCells{ static_cast<size_t>(newWidth * newHeight), PictureCell() };
    auto commonWidth = std::min(width, newWidth);
    auto commonHeight = std::min(height, newHeight);
    for (auto x = 0; x < commonWidth; x++) {
        for (auto y = 0; y < commonHeight; y++) {
            newCells.at(y * newWidth + x) = cells.at(y * width + x);
        }
    }
    width = newWidth;
    height = newHeight;
    cells = std::move(newCells);
}

Picture Picture::crop(TRect bounds) {
    Picture dst{ bounds.b.x - bounds.a.x, bounds.b.y - bounds.a.y };
    for (auto yDst = 0; yDst < dst.height; yDst++) {
        auto ySrc = yDst + bounds.a.y;
        if (ySrc >= 0 && ySrc < height) {
            for (auto xDst = 0; xDst < dst.width; xDst++) {
                auto xSrc = xDst + bounds.a.x;
                if (xSrc >= 0 && xSrc < width) {
                    dst.cells.at(yDst * dst.width + xDst) = cells.at(ySrc * width + xSrc);
                }
            }
        }
    }
    return dst;
}

PictureView::PictureView(const TRect& bounds, Picture picture, TColorBIOS bg)
    : TView(bounds), _picture(std::move(picture)), _bg(bg) {}

PictureView::PictureView(Picture picture, TColorBIOS bg)
    : PictureView(TRect{ 0, 0, picture.width + 2, picture.height }, std::move(picture), bg) {}

void PictureView::draw() {
    auto bgCell = PictureCell{ false, TColorAttr{ _bg, _bg }, " " };
    for (auto y = 0; y < size.y; y++) {
        TDrawBuffer b;
        for (auto x = 0; x < size.x + 2; x++) {
            PictureCell* cell = &bgCell;
            if (y < _picture.height && x > 0 && x - 1 < _picture.width) {
                auto& pictureCell = _picture.cells.at(y * _picture.width + x - 1);
                if (!pictureCell.transparent) {
                    cell = &pictureCell;
                }
            }
            b.moveStr(x, cell->ch, cell->colorAttr);
        }
        writeLine(0, static_cast<int16_t>(y), static_cast<int16_t>(size.x), 1, b);
    }
}

}  // namespace util
