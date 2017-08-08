#ifndef __OPEL_DBUG_LOG_H__
#define __OPEL_DBUG_LOG_H__

#include <stdarg.h>
#include <stdio.h>

#define FUNCTION_LOG 0

#define _opel_log(format, fw, color, args...) do { __opel_log(format, fw, \
    __FILE__, __func__, color, __LINE__, ##args); } while(0)
#define _opel_func(format, fw, args...) do { __opel_func(format, fw, \
    __FILE__, __func__, __LINE__, ##args); } while(0)

#define OPEL_DBG_VERB(fmt, args...) do{ _opel_log(fmt, "[VERB] APPCORE", 9, ##args); }while(0)
#define OPEL_DBG_LOG(fmt, args...) do{ _opel_log(fmt, "[LOG] APPCORE", 94, ##args); }while(0)
#define OPEL_DBG_WARN(fmt, args...) do{ _opel_log(fmt, "[WARN] APPCORE", 91, ##args); }while(0) 
#define OPEL_DBG_ERR(fmt, args...) do{ _opel_log(fmt, "[ERR] APPCORE", 101, ##args); }while(0)


#define __OPEL_FUNCTION_ENTER__ do{ _opel_func("ENTER", "APPCORE"); }while(0)
#define __OPEL_FUNCTION_EXIT__ do{ _opel_func("EXIT", "APPCORE"); }while(0)

inline void __opel_log(const char *format, const char *fw, const char *fileName,
    const char *funcName, int color, int lineNo, ...){
  va_list ap;
  printf("\033[%dm%s:%s: [%s():%d]:  ", color, fw, fileName, funcName, \
      lineNo);
  va_start(ap, lineNo);
  vprintf(format, ap);
  va_end(ap);
  printf("\033[0m\n");
}

inline void __opel_func(const char *format, const char *fw, const char *fileName, 
    const char *funcName, int lineNo, ...){
  va_list ap;
  printf("\033[2m%s:%s:[%s():%d][", fw, fileName, funcName, lineNo); 
  va_start(ap, lineNo);
  vprintf(format, ap);
  va_end(ap);
  printf("]\033[0m\n");
 
  
}
#endif // !defined(__OPEL_DBUG_LOG_H__)
