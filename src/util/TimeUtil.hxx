#ifndef NGENXX_SRC_UTIL_TIME_HXX_
#define NGENXX_SRC_UTIL_TIME_HXX_

#if defined(__cplusplus)

#include <chrono>
#include <thread>

using namespace std::chrono;

template <typename clockT, typename durationT>
uint64_t castNow()
{
    return duration_cast<durationT>(clockT::now().time_since_epoch()).count();
}

inline uint64_t nowInMicroSecs()
{
    return castNow<system_clock, microseconds>();
}

inline uint64_t nowInNanoSecs()
{
    return castNow<high_resolution_clock, nanoseconds>();
}

inline void sleepForMilliSecs(const size_t milliSecs)
{
    std::this_thread::sleep_for(milliseconds(milliSecs));
}

#endif

#endif // NGENXX_SRC_UTIL_TIME_HXX_