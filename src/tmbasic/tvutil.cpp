#include "tmbasic/tvutil.h"

namespace tmbasic {

size_t lengthWithoutTildes(TStringView text) {
    size_t len = 0;
    for (auto ch : text) {
        if (ch != '~') {
            len++;
        }
    }
    return len;
}

bool tryParseInt(const std::string& text, int* output) {
    for (auto ch : text) {
        if (ch != '-' && (std::isdigit(ch) == 0)) {
            return false;
        }
    }

    try {
        *output = std::stoi(text);
        return true;
    } catch (std::invalid_argument&) {
        return false;
    } catch (std::out_of_range&) {
        return false;
    }
}

static std::string abbreviateText(std::string_view text) {
    if (text.size() <= 15) {
        return std::string(text);
    }
    return std::string(text.substr(0, 13)) + "...";
}

int parseUserInt(const char* text, const char* fieldName, int minValue, int maxValue) {
    int value = 0;

    if (!tryParseInt(text, &value)) {
        std::ostringstream message;
        message << "The " << fieldName << " \"" << abbreviateText(text)
                << "\" is not a valid integer. Please enter a number between " << minValue << " and " << maxValue
                << ".";
        throw std::runtime_error(message.str());
    }

    if (value < minValue || value > maxValue) {
        std::ostringstream message;
        message << "The " << fieldName << " \"" << abbreviateText(text)
                << "\" is out of the valid range. Please enter a number between " << minValue << " and " << maxValue
                << ".";
        throw std::runtime_error(message.str());
    }

    return value;
}

}  // namespace tmbasic
