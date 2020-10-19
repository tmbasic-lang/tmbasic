#pragma once

#include "common.h"

// dynamic_cast and move into a new unique_ptr
template <typename TDst, typename TSrc>
static std::unique_ptr<TDst> dynamic_cast_move(std::unique_ptr<TSrc> src) {
    if (!src) {
        return {};
    } else {
        TDst* dst = &dynamic_cast<TDst&>(*src.get());
        src.release();
        return std::unique_ptr<TDst>(dst);
    }
}

template <typename TDst, typename TSrc>
static TDst* dynamic_cast_borrow(std::unique_ptr<TSrc>& src) {
    if (!src) {
        return nullptr;
    } else {
        return &dynamic_cast<TDst&>(*src.get());
    }
}
