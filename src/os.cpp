/**
 * Copyright 2019 Xiaobin Wei <xiaobin.wee@gmail.com>
 */

#include <string>

#include "os.h"

#ifdef _WINDOWS_PLATFORM_
#include <windows.h>
#else
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

#ifdef _ANDROID_PLATFORM_
#include <android/log.h>
#include <dlfcn.h>
#endif


namespace utils {
namespace os {

///! MUST BE identity with "utils.h"
enum LogLevel {
    ELL_DEBUG,
    ELL_WARN,
    ELL_ERROR,
};

#define PRINT_COLOR(ll, fmt, ...) \
do {    \
    if (ll == ELL_DEBUG) printf("\033[32m" fmt, ##__VA_ARGS__);         \
    else if (ll == ELL_WARN) printf("\033[33m" fmt, ##__VA_ARGS__);     \
    else if (ll == ELL_ERROR) printf("\033[31m" fmt, ##__VA_ARGS__);     \
    else printf(fmt, ##__VA_ARGS__);     \
} while (0);

// Printer::print
#ifdef _WINDOWS_PLATFORM_
void Printer::print(int ll, const char* msg) {
    OutputDebugStringA(msg);
    PRINT_COLOR(ll, "%s\n\033[0m", msg);
}
#elif defined(_ANDROID_PLATFORM_)
void Printer::print(int ll, const char* msg) {
    // Android logcat restricts log-output and cuts the rest of the message away
    // But we want it all. On my device max-len is 1023 (+ 0 byte).
    // Some websites claim a limit of 4096 so maybe different numbers on
    // different devices.
    const size_t maxLogLen = 1023;
    size_t msgLen = strlen(msg);
    size_t start = 0;
    android_LogPriority android_ll = ANDROID_LOG_DEBUG;
    if (ll == ELL_WARN)
        android_ll = ANDROID_LOG_WARN;
    else if (ll == ELL_ERROR)
        android_ll = ANDROID_LOG_ERROR;

    const char* tag_end = strchr(msg, '/');
    std::string LOG_TAG(msg+1, tag_end-(msg+1));
    tag_end = strchr(msg, ']');
    start = tag_end - msg + 1;
    msgLen -= start - 1;
    while (msgLen - start > maxLogLen) {
        __android_log_print(android_ll, LOG_TAG.c_str(), "[%d %d] %.*s\n", getpid(), gettid(), maxLogLen, &msg[start]);
        PRINT_COLOR(ll, "[%d,%d] %.*s\n\033[0m", getpid(), gettid(), maxLogLen, &msg[start]);
        start += maxLogLen;
    }
    __android_log_print(all, LOG_TAG.c_str(), "[%d %d] %s\n", getpid(), gettid(), &msg[start]);
    // print to stdout either
    PRINT_COLOR(ll, "[%d %d] %s\n\033[0m", getpid(), gettid(), &msg[start]);
}
#elif defined(_OSX_PLATFORM_)
void Printer::print(int ll, const char* msg) {
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    PRINT_COLOR(ll, "[%d %llu] %s\n\033[0m", getpid(), tid, msg);
}
#elif defined(_LINUX_PLATFORM_)
void Printer::print(int ll, const char* msg) {
    pid_t tid = syscall(SYS_gettid);
    PRINT_COLOR(ll, "[%d %d] %s\n\033[0m", getpid(), tid, msg);
}
#else
void Printer::print(int ll, const char* msg) {
    PRINT_COLOR(ll, "Unsupported operation system!!!\n\033[0m");
}
#endif

void Trace::init() {
#ifdef _ANDROID_PLATFORM_
#ifdef _ENABLE_SYSTRACE_FILE_
    int trace_on_fd = open("/sys/kernel/debug/tracing/tracing_on", O_WRONLY);
    if (trace_on_fd == -1) {
        Printer::print(ELL_ERROR, "[MegSDK/ERROR] trace init failed when tracing_on\n");
        return;
    } else {
        write(trace_on_fd, "1", 1);
        close(trace_on_fd);
    }
    atrace_marker_fd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
    // Printer::print("trace fd id %d\n", atrace_marker_fd);
    if (atrace_marker_fd == -1) Printer::print(ELL_ERROR, "[MegSDK/ERROR] trace init failed!\n");
#else
    void *lib = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL);
    if (lib == NULL) lib = dlopen("libnativewindow.so", RTLD_NOW | RTLD_LOCAL);

    if (lib != NULL) {
        ATrace_beginSection = reinterpret_cast<fp_ATrace_beginSection>(dlsym(lib, "ATrace_beginSection"));
        ATrace_endSection = reinterpret_cast<fp_ATrace_endSection>(dlsym(lib, "ATrace_endSection"));
        ATrace_isEnabled = reinterpret_cast<fp_ATrace_isEnabled>(dlsym(lib, "ATrace_isEnabled"));

        if (ATrace_beginSection != nullptr && ATrace_endSection != nullptr && ATrace_isEnabled != nullptr) {
            Printer::print(ELL_ERROR, "[MegSDK/INFO] libandroid.so load successfully");
        } else {
            Printer::print(ELL_ERROR, "[MegSDK/ERROR] dlsym failed!!!");
        }
    } else {
        Printer::print(ELL_ERROR, "[MegSDK/ERROR] load libandroid.so failed!!");
    }
#endif
#endif
}

void Trace::deinit() {
#ifdef _ANDROID_PLATFORM_
#ifdef _ENABLE_SYSTRACE_FILE_
    if (atrace_marker_fd != -1) {
        close(atrace_marker_fd);
        atrace_marker_fd = -1;
    }
    Printer::print(ELL_DEBUG, "[MegSDK/INFO] close trace fd\n");
#endif
#endif
}

void Trace::begin(const char* name) {
#ifdef _ANDROID_PLATFORM_
#ifdef _ENABLE_SYSTRACE_FILE_
    char buf[1024];
    int len = snprintf(buf, sizeof(buf), "B|%d|%s", getpid(), name);
    int written = write(atrace_marker_fd, buf, len);
    if (len != written) {
        Printer::print(ELL_ERROR, "[MegSDK/ERROR] trace_begin write error\n");
    }
#else
    if (ATrace_beginSection != nullptr) ATrace_beginSection(name);
#endif
#endif
}

void Trace::end() {
#ifdef _ANDROID_PLATFORM_
#ifdef _ENABLE_SYSTRACE_FILE_
    char c = 'E';
    if (1 != write(atrace_marker_fd, &c, 1)) {
        Printer::print(ELL_ERROR, "[MegSDK/ERROR] trace_end write error\n");
    }
#else
    if (ATrace_endSection != nullptr) ATrace_endSection();
#endif
#endif
}

}  // namespace os
}  // namespace utils
