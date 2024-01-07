#include "CheckBoxes.h"
#include "tvutil.h"

namespace util {

static gsl::owner<TSItem*> convertLabelsToTSItems(const std::vector<std::string>& labels) {
    gsl::owner<TSItem*> item = nullptr;

    for (const auto& label : labels) {
        item = new TSItem(label, item);
    }

    return item;
}

static TRect getSize(const std::vector<std::string>& labels) {
    auto height = 0;
    auto width = 0;

    for (const auto& label : labels) {
        width = max(cstrlen(label), width);
        height++;
    }

    return { 0, 0, width + 7, height };
}

CheckBoxes::CheckBoxes(const std::vector<std::string>& labels)
    : TCheckBoxes(getSize(labels), convertLabelsToTSItems(labels)) {}

CheckBoxes::CheckBoxes(const std::vector<std::string>& labels, const std::vector<bool>& checks) : CheckBoxes(labels) {
    size_t i = 0;
    for (auto check : checks) {
        if (check) {
            value |= 1 << i;
        }
        i++;
    }
}

}  // namespace util
