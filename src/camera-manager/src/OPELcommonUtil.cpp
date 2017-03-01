#include "OPELcommonUtil.h"
#include <stdlib.h>
#include <cstring>
std::string* charToPtrString(const char* _str)
{
  std::string *tmp = new std::string();
  (*tmp) = _str;
  return tmp;
}
std::string charToString(const char* _str)
{
  std::string tmp = _str;
  return tmp;
}
gchar* stringToGchar(std::string* _str)
{
  unsigned int _str_size = _str->size();
  unsigned char *_char_str = (unsigned char*)malloc(_str_size+1);
  strcpy((char*)_char_str, _str->c_str());
  return (gchar*)_char_str;
}

gchar* strToGcharNonPtr(std::string &_str)
{
  unsigned int _str_size = _str.size();
  unsigned char *_char_str = (unsigned char*)malloc(_str_size+1);
  strcpy((char*)_char_str, _str.c_str());
  return (gchar*)_char_str;
}

void pid_to_char(unsigned int input, char* output){
  unsigned int value = input;
  unsigned int temp;
  int i;
  output[5] = '\0';

  for (i = 4; i >= 0; i--){
    temp = value % 10; 
    output[i] = ((char)(temp)) + 65;
    value = value / 10;
  }
}
