#pragma once

namespace util {

template <typename T>
T min(T a, T b) {
    return a < b ? a : b;
}

template <typename T>
T max(T a, T b) {
    return a < b ? b : a;
}

}  // namespace util
