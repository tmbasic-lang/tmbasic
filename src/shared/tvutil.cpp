#include "../shared/tvutil.h"

namespace shared {

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

void validateIdentifier(const std::string& text, const char* fieldName) {
    if (text.empty()) {
        std::ostringstream message;
        message << "Please enter the " << fieldName << ".";
        throw std::runtime_error(message.str());
    }

    auto isFirst = true;
    for (auto ch : text) {
        auto isUpper = ch >= 'A' && ch <= 'Z';
        auto isLower = ch >= 'a' && ch <= 'z';
        auto isDigit = ch >= '0' && ch <= '9';
        auto isUnderscore = ch == '_';

        if (isFirst) {
            if (!isUpper && !isLower && !isUnderscore) {
                std::ostringstream message;
                message << "The " << fieldName << " is invalid. It must start with a letter or underscore.";
                throw std::runtime_error(message.str());
            }
            isFirst = false;
        } else if (!isUpper && !isLower && !isDigit && !isUnderscore) {
            std::ostringstream message;
            message << "The " << fieldName << " is invalid. It must contain only letters, numbers, and underscores.";
            throw std::runtime_error(message.str());
        }
    }
}

std::string ellipsis(const std::string& text, size_t maxLen) {
    if (text.size() <= maxLen) {
        return text;
    }
    return fmt::format("{}...", text.substr(0, maxLen - 3));
}

void enableDisableCommand(bool enable, uint16_t command) {
    (enable ? TView::enableCommand : TView::disableCommand)(command);
}

void enableDisableCommands(bool enable, std::initializer_list<uint16_t> commands) {
    TCommandSet commandSet;
    for (auto command : commands) {
        commandSet.enableCmd(command);
    }
    (enable ? TView::enableCommands : TView::disableCommands)(commandSet);
}

}  // namespace shared
