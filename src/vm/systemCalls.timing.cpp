#include "vm/systemCall.h"

namespace vm {

#ifdef _WIN32
static bool _performanceCounterInitialized = false;
static int64_t _performanceCounterTicksPerSecond = 0;
#endif

// () as TimeSpan
void systemCallStopwatch(const SystemCallInput& /*input*/, SystemCallResult* result) {
#if defined(_WIN32)
    // MSDN promises that neither of these functions will ever fail.
    // It further promises that we can cache the frequency value.
    if (!_performanceCounterInitialized) {
        LARGE_INTEGER frequency{};
        QueryPerformanceFrequency(&frequency);
        _performanceCounterTicksPerSecond = frequency.QuadPart;
        _performanceCounterInitialized = true;
    }

    LARGE_INTEGER counter{};
    QueryPerformanceCounter(&counter);
    int64_t const ticks = counter.QuadPart;  // LONGLONG should be the same as int64_t.
    int64_t const milliseconds = ticks * 1000 / _performanceCounterTicksPerSecond;
    result->returnedValue = Value{ milliseconds };

#elif defined(__linux__)
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    int64_t const milliseconds = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    result->returnedValue = Value{ milliseconds };

#elif defined(__APPLE__)
    uint64_t const nanoseconds = clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
    uint64_t const milliseconds = nanoseconds / 1000000;
    result->returnedValue = Value{ milliseconds };

#endif
}

// (delay as TimeSpan)
void systemCallSleep(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto milliseconds = input.getValue(-1).getInt64();
    if (milliseconds > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
}

}  // namespace vm
