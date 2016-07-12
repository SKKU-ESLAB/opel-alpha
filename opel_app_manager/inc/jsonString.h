#ifndef _JSONSTRING
#define _JSONSTRING


#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


class jsonString{
private:
		char jsonData[1024]; // [MORE] change 1024 -> BUFSIZE

public:
	jsonString();
   // ~jsonString();
	jsonString(char* json);

	void addType(char* value);
	void addItem(const char* key, const char* value);

	
	char* getJsonData();
	char* findValue(char* key);
	int getWholeKeyValue(char returnArray[32][128]);
};

#endif
