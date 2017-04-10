#ifndef OPEL_COMMON_UTIL_H
#define OPEL_COMMON_UTIL_H

#include <string>
#include <gst/gst.h>

std::string* charToPtrString(const char* _str);
std::string charToString(const char* _str);
gchar* stringToGchar(std::string *_str);
gchar* strToGcharNonPtr(std::string &_str);

#endif /* OPEL_COMMON_UTIL_H */
