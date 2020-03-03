/**
 * Copyright 2019 Xiaobin Wei <xiaobin.wee@gmail.com>
 */

#ifndef UTILS_H_
#define UTILS_H_


#include <cstddef>
#include <string>
#include <fcntl.h>
#include <chrono>

/* ================ logging ================  */
// *LOG_TAG* MUST BE defined before *#include "utils.h"*
#define _log_print_ utils::io::_log_print
#define logp(fmt, ...)      _log_print_(utils::io::LogLevel::DEBUG, LOG_TAG, fmt, ##__VA_ARGS__)
#define logd(fmt, ...)      _log_print_(utils::io::LogLevel::DEBUG, LOG_TAG, fmt, ##__VA_ARGS__)
#define logw(fmt, ...)      _log_print_(utils::io::LogLevel::WARN,  LOG_TAG, fmt, ##__VA_ARGS__)
#define loge(fmt, ...)      _log_print_(utils::io::LogLevel::ERROR, LOG_TAG, fmt, ##__VA_ARGS__)


/* ================ systrace ================  */
#ifdef _ENABLE_SYSTRACE_
#   define trace_init()        os::Trace::init()
#   define trace_deinit()      os::Trace::deinit()
#   define trace_begin(name)   os::Trace::begin(name)
#   define trace_end()         os::Trace::end()
#   define trace_isEnabled()   os::Trace::enable()
#else
#   define trace_init()
#   define trace_deinit()
#   define trace_begin(name)
#   define trace_end()
#   define trace_isEnabled()   false
#endif


namespace utils {
namespace io {

/**
 * \brief SHOULD NOT BE called, use *log_xxx* instead
 */
enum LogLevel {
    DEBUG,
    WARN,
    ERROR,
};

/**
 * \brief SHOULD NOT BE called, use *log_xxx* instead
 */
void _log_print(LogLevel ll, const char* tag, const char* fmt, ...);

/**
 * \brief read file *ifname* to *buf*
 * \param ifname input file name
 * \param buf output buffer address
 * \return buffer size
 */
int readFile(const char *ifname, char **buf);

/**
 * \brief write buffer *buf* to file *ofname*
 * \param ofname output file name
 * \param buf input buffer address
 * \param size buffer size
 */
void writeFile(const char *ofname, char *buf, size_t size);

}  // namespace io




namespace perf {
// Prefer high_resolution_clock, but only if it's steady...
template <bool HighResIsSteady = std::chrono::high_resolution_clock::is_steady>
struct SteadyClock {
    using type = std::chrono::high_resolution_clock;
};

// ...otherwise use steady_clock.
template <>
struct SteadyClock<false> {
    using type = std::chrono::steady_clock;
};

static inline SteadyClock<>::type::time_point benchmark_now() {
    return SteadyClock<>::type::now();
}

static inline double benchmark_duration_ns (
    SteadyClock<>::type::time_point start,
    SteadyClock<>::type::time_point end) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

static inline double benchmark_duration_ms (
    SteadyClock<>::type::time_point start,
    SteadyClock<>::type::time_point end) {
    return benchmark_duration_ns(start, end) * 1e-6;
}

static inline double benchmark_duration_seconds (
    SteadyClock<>::type::time_point start,
    SteadyClock<>::type::time_point end) {
    return benchmark_duration_ns(start, end) * 1e-9;
}

/**
 * \brief High resolution timer class
 * 
 */
class Timer {

 public:
   Timer() {
       reset();
    }

    void reset() {
        mStart = benchmark_now();
    }

    double get_secs() const {
        double best = std::numeric_limits<double>::infinity();
        auto end = benchmark_now();
        double elapsed_seconds = benchmark_duration_seconds(mStart, end);
        best = std::min(best, elapsed_seconds);
        return best;
    }

    double get_msecs() const {
        double best = std::numeric_limits<double>::infinity();
        auto end = benchmark_now();
        double elapsed_ms = benchmark_duration_ms(mStart, end);
        best = std::min(best, elapsed_ms);
        return best;
   }

    double get_nsecs() const {
        double best = std::numeric_limits<double>::infinity();
        auto end = benchmark_now();
        double elapsed_ns = benchmark_duration_ns(mStart, end);
        best = std::min(best, elapsed_ns);
        return best;
   }

    double get_secs_reset() {
        auto ret = get_secs();
        reset();
        return ret;
    }

    double get_msecs_reset() {
        auto ret = get_msecs();
        reset();
        return ret;
    }

    double get_nsecs_reset() {
        auto ret = get_nsecs();
        reset();
        return ret;
    }

 private:
   SteadyClock<>::type::time_point mStart;
};

}  // namespace perf
}  // namespace utils

#endif   // UTILS_H_
