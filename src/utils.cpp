/**
 * Copyright 2019 Xiaobin Wei <xiaobin.wee@gmail.com>
 */

#include <stdarg.h>
#include <string>

#include "os.h"
#include "utils.h"

namespace utils {
namespace io {

// Used to control enable/disable which one to be printed out
LogLevel gMinLogLevel = LogLevel::DEBUG;

int readFile(char const *ifname, char **buf) {
    if (buf == nullptr) return 0;
    FILE *fp = fopen(ifname, "rb");

    if (fp == nullptr) printf("Cannot open file: %s\n", ifname);

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    if (buf != nullptr) {
        fseek(fp, 0, SEEK_SET);
        *buf = new char[size];
        fread(*buf, size, 1, fp);
    }
    fclose(fp);

    return size;
}

void writeFile(char const *ofname, char *buf, size_t size) {
    if (buf == nullptr) return;

    FILE *fp = fopen(ofname, "wb");
    if (fp == nullptr) printf("Cannot open file: %s\n", ofname);
    fwrite(buf, size, 1, fp);
    fclose(fp);
}

static std::string svsprintf(const char *fmt, va_list ap_orig) {
    int size = 1023;
    char *p;

    if ((p = (char *)malloc(size)) == nullptr) goto err;

    for (;;) {
        va_list ap;
        va_copy(ap, ap_orig);
        int n = vsnprintf(p, size, fmt, ap);
        va_end(ap);

        if (n < 0) goto err;

        if (n < size) {
            std::string rst(p);
            free(p);
            return rst;
        }

        size = n + 1;

        char *np = (char *)realloc(p, size);
        if (!np) {
            free(p);
            goto err;
        } else {
            p = np;
        }
    }

err:
    fprintf(stderr, "cannot allocate memory for svsprintf; fmt=%s\n", fmt);
#ifndef _WINDOWS_PLATFORM_
    __builtin_trap();
#else
    abort();
#endif  // !_MEG_WINDOWS_PLATFORM_
}

void _log_print(LogLevel ll, const char *tag, const char *fmt, ...) {
    const char *LL_STR = nullptr;

    if (ll < gMinLogLevel || ll > LogLevel::ERROR) return;

    switch (ll) {
        case LogLevel::DEBUG:
            LL_STR = "[DEBUG] ";
            break;
        case LogLevel::WARN:
            LL_STR = "[WARN] ";
            break;
        case LogLevel::ERROR:
            LL_STR = "[ERROR] ";
            break;
        default:
            LL_STR = "";
    }
    va_list ap;
    va_start(ap, fmt);
    auto msg = svsprintf(fmt, ap);
    va_end(ap);
    std::string tag_str = "[";
    tag_str += tag;
    tag_str += "]";
    std::string message(tag_str);
    message += LL_STR;
    message += msg;

    os::Printer::print(message.c_str());
}

}  // namespace io
}  // namespace utils