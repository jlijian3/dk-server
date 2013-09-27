/**
 * author:lijian2@ucweb.com
 * date:2012-03-22
 */

#ifndef __DONKEY_LOG_INCLUDE__
#define __DONKEY_LOG_INCLUDE__ 

#include <stdio.h>
#include <stdarg.h>

typedef void (*log_func_t)(const char *msg, int msg_len);

class DKLog {
public: 
  static void Debug(const char *format, ...) {
    va_list va;
    va_start(va, format);
    
    if (log_func_) {
      int fmt_len;
      char buf[1024];
      int buf_size = sizeof(buf);

      fmt_len = vsnprintf(buf, buf_size, format, va);
      if (fmt_len > buf_size) {
        *(buf + buf_size - 1) = '\0';
        fmt_len = buf_size - 1;
      }

      log_func_(buf, fmt_len);
    } else {
      FILE *f = fstream_?fstream_:stderr;
      vfprintf(f, format, va);
    }
    va_end(va);
  }
  
  static void set_log_func(log_func_t log_func) {
    log_func_ = log_func;
  }

  static void set_fstream(FILE *f) {
    fstream_ = f;
  }

private:
  static log_func_t log_func_;
  static FILE *fstream_;
};

#define DK_DEBUG(args...) DKLog::Debug(args)

#endif
