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

// Printer::print
#ifdef _WINDOWS_PLATFORM_
void Printer::print(const char* msg) {
    OutputDebugStringA(msg);
    printf("%s\n", msg);
}
#elif defined(_ANDROID_PLATFORM_)
void Printer::print(const char* msg) {
    // Android logcat restricts log-output and cuts the rest of the message away
    // But we want it all. On my device max-len is 1023 (+ 0 byte).
    // Some websites claim a limit of 4096 so maybe different numbers on
    // different devices.
    const size_t maxLogLen = 1023;
    size_t msgLen = strlen(msg);
    size_t start = 0;
    while (msgLen - start > maxLogLen) {
        __android_log_print(ANDROID_LOG_DEBUG, "", "[%d %d] %.*s\n", getpid(), gettid(), maxLogLen, &msg[start]);
        printf("[%d,%d] %.*s\n", getpid(), gettid(), maxLogLen, &msg[start]);
        start += maxLogLen;
    }
    __android_log_print(ANDROID_LOG_DEBUG, "", "[%d %d] %s\n", getpid(), gettid(), &msg[start]);
    // print to stdout either
    printf("[%d %d] %s\n", getpid(), gettid(), &msg[start]);
}
#elif defined(_OSX_PLATFORM_)
void Printer::print(const char* msg) {
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    printf("[%d %llu] %s\n", getpid(), tid, msg);
}
#elif defined(_LINUX_PLATFORM_)
void Printer::print(const char* msg) {
    pid_t tid = syscall(SYS_gettid);
    printf("[%d %d] %s\n", getpid(), tid, msg);
}
#else
void Printer::print(const char* msg) { printf("Unsupported operation system!!!"); }
#endif

void Trace::init() {
#ifdef _ANDROID_PLATFORM_
#ifdef _ENABLE_SYSTRACE_FILE_
    int trace_on_fd = open("/sys/kernel/debug/tracing/tracing_on", O_WRONLY);
    if (trace_on_fd == -1) {
        Printer::print("[MegSDK/ERROR] trace init failed when tracing_on\n");
        return;
    } else {
        write(trace_on_fd, "1", 1);
        close(trace_on_fd);
    }
    atrace_marker_fd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
    // Printer::print("trace fd id %d\n", atrace_marker_fd);
    if (atrace_marker_fd == -1) Printer::print("[MegSDK/ERROR] trace init failed!\n");
#else
    void *lib = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL);
    if (lib == NULL) lib = dlopen("libnativewindow.so", RTLD_NOW | RTLD_LOCAL);

    if (lib != NULL) {
        ATrace_beginSection = reinterpret_cast<fp_ATrace_beginSection>(dlsym(lib, "ATrace_beginSection"));
        ATrace_endSection = reinterpret_cast<fp_ATrace_endSection>(dlsym(lib, "ATrace_endSection"));
        ATrace_isEnabled = reinterpret_cast<fp_ATrace_isEnabled>(dlsym(lib, "ATrace_isEnabled"));

        if (ATrace_beginSection != nullptr && ATrace_endSection != nullptr && ATrace_isEnabled != nullptr) {
            Printer::print("[MegSDK/INFO] libandroid.so load successfully");
        } else {
            Printer::print("[MegSDK/ERROR] dlsym failed!!!");
        }
    } else {
        Printer::print("[MegSDK/ERROR] load libandroid.so failed!!");
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
    Printer::print("[MegSDK/INFO] close trace fd\n");
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
        Printer::print("[MegSDK/ERROR] trace_begin write error\n");
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
        Printer::print("[MegSDK/ERROR] trace_end write error\n");
    }
#else
    if (ATrace_endSection != nullptr) ATrace_endSection();
#endif
#endif
}

}  // namespace os
}  // namespace utils
