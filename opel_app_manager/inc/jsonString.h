/* Copyright (c) 2015-2016 CISS, and contributors. All rights reserved.
 *
 * Contributor: Dongig Sin<dongig@skku.edu>, 
 *              Gyeonghwan Hong<redcarrottt@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
