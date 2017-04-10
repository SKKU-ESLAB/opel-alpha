#ifndef OPELDEBUGLOG_H
#define OPEL_DEBUG_LOG_H

#include <stdarg.h>
#include <stdio.h>

// Debug Option
#define CONFIG_DBG_VERB     1
#define CONFIG_DBG_LOG      1
#define CONFIG_DBG_WARN     1
#define CONFIG_DBG_ERR      1
#define CONFIG_FUNCTION_LOG 1

#define _opel_log(format, fw, color, args...) do { __opel_log(format, fw, \
    __FILE__, __func__, color, __LINE__, ##args); } while(0)
#define _opel_func(format, fw, args...) do { __opel_func(format, fw, \
    __FILE__, __func__, __LINE__, ##args); } while(0)

#if CONFIG_DBG_VERB
#define OPEL_DBG_VERB(fmt, args...) do{ _opel_log(fmt, "[VERB] CAMERA FW", 9, ##args); }while(0)
#else
#define OPEL_DBG_VERB(fmt, args...)
#endif

#if CONFIG_DBG_LOG
#define OPEL_DBG_LOG(fmt, args...) do{ _opel_log(fmt, "[LOG] CAMERA FW", 94, ##args); }while(0)
#else
#define OPEL_DBG_LOG(fmt, args...)
#endif

#if CONFIG_DBG_WARN
#define OPEL_DBG_WARN(fmt, args...) do{ _opel_log(fmt, "[WARN] CAMERA FW", 91, ##args); }while(0)
#else
#define OPEL_DBG_WARN(fmt, args...)
#endif

#if CONFIG_DBG_ERR
#define OPEL_DBG_ERR(fmt, args...) do{ _opel_log(fmt, "[ERR] CAMERA FW", 101, ##args); }while(0)
#else
#define OPEL_DBG_ERR(fmt, args...)
#endif

#if CONFIG_FUNCTION_LOG
#define __OPEL_FUNCTION_ENTER__ do{ _opel_func("ENTER", "CAMERA FW"); }while(0)
#define __OPEL_FUNCTION_EXIT__ do{ _opel_func("EXIT", "CAMERA FW"); }while(0)
#else
#define __OPEL_FUNCTION_ENTER__
#define __OPEL_FUNCTION_EXIT__
#endif

inline void __opel_log(const char *format, const char *fw, const char *fileName,
    const char *funcName, int color, int lineNo, ...)
{
  va_list ap;
  printf("\033[%dm%s:%s: [%s():%d]:  ", color, fw, fileName, funcName, lineNo);
  va_start(ap, lineNo);
  vprintf(format, ap);
  va_end(ap);
  printf("\033[0m\n");
}

inline void __opel_func(const char *format, const char *fw, const char *fileName, 
    const char *funcName, int lineNo, ...)
{
  va_list ap;
  printf("\033[2m%s:%s:[%s():%d][", fw, fileName, funcName, lineNo);
  va_start(ap, lineNo);
  vprintf(format, ap);
  va_end(ap);
  printf("]\033[0m\n");
}

#endif /* OPELDEBUGLOG_H */
