#ifndef _JSONSTRING
#define _JSONSTRING


#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


class jsonString{
private:
		char jsonData[1024]; // TODO: change 1024 -> BUFSIZE (defined in comManager)

public:
	jsonString();
   // ~jsonString();
	jsonString(const char* json);

	void addType(char* value);
	void addItem(const char* key, const char* value);

	
  std::string getJsonData();
  std::string findValue(char* key);
	int getWholeKeyValue(char returnArray[32][128]);
};

#endif
