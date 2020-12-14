#include "CheckBoxes.h"
#include "tmbasic/tvutil.h"

namespace tmbasic {

static TSItem* convertLabelsToTSItems(const std::initializer_list<std::string>& labels) {
    TSItem* item = nullptr;

    for (auto it = labels.end(); it-- != labels.begin();) {
        auto& label = *it;
        item = new TSItem(label, item);
    }

    return item;
}

static TRect getSize(const std::initializer_list<std::string>& labels) {
    auto height = 0;
    auto width = 0;

    for (auto& label : labels) {
        width = max(lengthWithoutTildes(label), width);
        height++;
    }

    return TRect(0, 0, width + 7, height);
}

CheckBoxes::CheckBoxes(std::initializer_list<std::string> labels)
    : TCheckBoxes(getSize(labels), convertLabelsToTSItems(labels)) {}

CheckBoxes::CheckBoxes(std::initializer_list<std::string> labels, std::initializer_list<bool> checks)
    : CheckBoxes(labels) {
    size_t i = 0;
    for (auto check : checks) {
        if (check) {
            value |= 1 << i;
        }
        i++;
    }
}

}  // namespace tmbasic
