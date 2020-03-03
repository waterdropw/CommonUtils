/**
 * Copyright 2019 Xiaobin Wei <xiaobin.wee@gmail.com>
 */

#ifndef __OS_H_INCLUDED__
#define __OS_H_INCLUDED__

#include <stdint.h>

namespace utils {
namespace os {

class Printer {
   public:
    // prints out a string to the console out stdout or debug log or whatever
    static void print(const char *msg);
};

class Trace {
    typedef void *(*fp_ATrace_beginSection)(const char *sectionName);
    typedef void *(*fp_ATrace_endSection)(void);
    typedef void *(*fp_ATrace_isEnabled)(void);

   public:
    static void init();
    static void deinit();
    static void begin(const char *name);
    static void end();

   private:
    static int atrace_marker_fd;
    static fp_ATrace_beginSection ATrace_beginSection;
    static fp_ATrace_endSection ATrace_endSection;
    static fp_ATrace_isEnabled ATrace_isEnabled;
};

}  // namespace os
}  // namespace utils

#endif  // __OS_H_INCLUDED__
