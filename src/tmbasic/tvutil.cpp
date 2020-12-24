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

}  // namespace tmbasic
