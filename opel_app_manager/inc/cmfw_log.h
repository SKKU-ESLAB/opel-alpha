/*
 * OPEL Communication Framework
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/**
 * This file declares cmfw log functions and data structure.
 *
 * @file		cmfw_log.h
 * @author	Eunsoo Park (esevan.park@gmail.com)
 * @version	0.2
 * @Last modified 2016-5-19
 */
#ifndef __CMFW_LOG_H__
#define __CMFW_LOG_H__

#include <stdio.h>
#include <stdarg.h>

#define cmfw_log(format, args...) do { _cmfw_log(format, __FILE__, __func__, __LINE__, ##args); } while(0)
#define __ENTER__ cmfw_log( "Enter=================" )
#define __EXIT__ cmfw_log( "==================Exit" )

#ifndef IN
#define IN
#define OUT
#endif

#ifndef null
#define null (void *)0
#endif

//#define NO_LOG


#ifndef NO_LOG
inline void _cmfw_log(const char *format, const char *fileName, const char *funcName, int lineNo, ...){
	va_list ap;

	printf("CMFW(%s):%s(%d)\t> ", funcName, fileName,lineNo);
	va_start(ap, lineNo);
	vprintf(format, ap);
	va_end(ap);
	printf("\n");
}
#else // NO_LOG
inline void _cmfw_log(const char *format, const char *fileName, const char *funcName, int lineNo, ...){
	va_list ap;
}
#endif // NO_LOG

#endif // __CMFW_LOG_H__ // 2016-5-19 Eunsoo Park
