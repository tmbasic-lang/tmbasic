#pragma once

#include "../common.h"

namespace util {

// dynamic_cast and move into a new unique_ptr
template <typename TDst, typename TSrc>
static std::unique_ptr<TDst> dynamic_cast_move(std::unique_ptr<TSrc> src) {
    if (!src) {
        return {};
    } else {
        auto* dst = dynamic_cast<TDst*>(src.get());
        assert(dst != nullptr);
        src.release();
        return std::unique_ptr<TDst>(dst);
    }
}

template <typename TDst, typename TSrc>
static TDst* dynamic_cast_borrow(const std::unique_ptr<TSrc>& src) {
    if (!src) {
        return nullptr;
    } else {
        auto* result = dynamic_cast<TDst*>(src.get());
        assert(result != nullptr);
        return result;
    }
}

}  // namespace util
