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

#include "jsonString.h"
#include "commManager.h"
#include <stdlib.h>


jsonString::jsonString(){

	memset(jsonData, '\0', (sizeof(char) * MSGBUFSIZE));	
	strcpy(jsonData, "{}");

}

/*
jsonString::~jsonString(){

	delete[] jsonData;
}
*/
jsonString::jsonString(const char* inputJson){
	//jsonData = new char[MSGBUFSIZE];
	memset(jsonData, '\0', (sizeof(char) * MSGBUFSIZE));	
	strncpy(jsonData, inputJson, (sizeof(char) * MSGBUFSIZE));
}

void jsonString::addType(char* value){
	if (strlen(jsonData) + 16 + strlen(value) > MSGBUFSIZE){
		printf("[jsonString] Cannot add key, value to json cuz of overflow\n");
		return ;
	}
	char tmpStr[MSGBUFSIZE];
	memset(tmpStr, '\0', (sizeof(char) * MSGBUFSIZE ));	
	strncpy(tmpStr, jsonData, strlen(jsonData)-1);
	sprintf(tmpStr, "%s\"%s\":\"%s\"}", tmpStr, "type", value);

	memset(jsonData, '\0', (sizeof(char) * MSGBUFSIZE));	
	strcpy( jsonData, tmpStr);
}



void jsonString::addItem(const char* key, const char* value){


	if (strlen(jsonData) + strlen(key) + strlen(value) > MSGBUFSIZE){
		printf("[jsonString] Cannot add key, value to json cuz of overflow\n");
		return ;
	}

	char tmpStr[MSGBUFSIZE]; 	
	memset(tmpStr, '\0', (sizeof(char) * MSGBUFSIZE ));		
	strncpy(tmpStr, jsonData, strlen(jsonData)-1); 
	sprintf(tmpStr, "%s,\"%s\":\"%s\"}", tmpStr, key, value); 

	memset(jsonData, '\0', (sizeof(char) * MSGBUFSIZE));	
	strcpy( jsonData, tmpStr);
}

std::string jsonString::getJsonData(){
	return std::string(jsonData);
}

std::string jsonString::findValue(char* desKey){
	int position = 0;
	char key[512];
	char value[512];
	memset(key, '\0', 512);
	memset(value, '\0', 512);
		
	for(unsigned int i=0; i<strlen(jsonData); i++){
	
			if ( jsonData[i] == '{' )
				continue;
	
			else if ( jsonData[i] == '}' ){
				if(!strcmp(key, desKey)){

					return std::string(value);
				}
			}
			
			else if ( jsonData[i] == ':' ){
				if( jsonData[i+1] == '"'){
					position++;
				}
				else{
					char tmp[2];
					tmp[1] = '\0';
					tmp[0] = jsonData[i];			
									
					if(! (position % 2)) // even num -> key
						strcat(key, tmp);
					else				// odd num -> value
						strcat(value, tmp);

				}
			}
	
			else if ( jsonData[i] == ',' ){
				if( jsonData[i+1] == '"'){
					position++;
	
					if(!strcmp(key, desKey)){

						return std::string(value);
					}
					else{
						
						memset(key, '\0', 512);
						memset(value, '\0', 512);	
					}
				}
			}
	
			else if ( jsonData[i] == '"' ){
				continue;
			}
	
			else{
	
				char tmp[2];
				tmp[1] = '\0';
				tmp[0] = jsonData[i];			
				
				if(! (position % 2)) // even num -> key
					strcat(key, tmp);
				else				// odd num -> value
					strcat(value, tmp);
			}
		}
	
	printf("[JsonString] Find value >> Cannot find the item with key:%s json:%s\n", desKey, jsonData);
	return NULL;		
}

int jsonString::getWholeKeyValue(char returnArray[32][128]){
		printf("\n\n\n wholeItem json : %s  length : %d\n", jsonData, strlen(jsonData) );

		int position = 0;

		int arrPosition=0;
		char key[256];
		char value[256];
		memset(key, '\0', 256);
		memset(value, '\0', 256);

//		char* returnArray[128];
		
		for(unsigned int i=0; i<strlen(jsonData); i++){
		
				if ( jsonData[i] == '{' )
					continue;
		
				else if ( jsonData[i] == '}' ){
					
//					returnArray[arrPosition] = new char[strlen(key)];
		//			returnArray[arrPosition] = (char*) malloc ( sizeof(char) * strlen(key));	

		//			memset(returnArray[arrPosition], '\0', strlen(returnArray[arrPosition]));
					strcpy(returnArray[arrPosition], key);
					printf("1 : %s %s\n", key, returnArray[arrPosition]);
					arrPosition++;

//					returnArray[arrPosition] = new char[strlen(value)];
		//			returnArray[arrPosition] = (char*) malloc ( sizeof(char) * strlen(value));	
					
		//			memset(returnArray[arrPosition], '\0', strlen(returnArray[arrPosition]));
					strcpy(returnArray[arrPosition], value);
					printf("2 : %s %s %d %d\n", value, returnArray[arrPosition], strlen(value), strlen(returnArray[arrPosition]));
					arrPosition++;

					memset(key, '\0', 256);
					memset(value, '\0', 256);
				}
				
				else if ( jsonData[i] == ':' ){
					if( jsonData[i+1] == '"'){
						position++;
					}
					else{
						char tmp[2];
						tmp[1] = '\0';
						tmp[0] = jsonData[i];			
					
						if(! (position % 2)) // even num -> key
							strcat(key, tmp);
						else				// odd num -> value
							strcat(value, tmp);
					}
				}
		
				else if ( jsonData[i] == ',' ){
					if( jsonData[i+1] == '"'){
						position++;
		
//						returnArray[arrPosition] = new char[strlen(key)];
			//			returnArray[arrPosition] = (char*) malloc ( sizeof(char) * strlen(key));	
			//			memset(returnArray[arrPosition], '\0', strlen(returnArray[arrPosition]));
						strcpy(returnArray[arrPosition], key);
						printf("3 : %s %s\n", key, returnArray[arrPosition]);
						arrPosition++;

//						returnArray[arrPosition] = new char[strlen(value)];
			//			returnArray[arrPosition] = (char*) malloc ( sizeof(char) * strlen(value));
			//			memset(returnArray[arrPosition], '\0', strlen(returnArray[arrPosition]));
						strcpy(returnArray[arrPosition], value);
						printf("4 : %s %s %d %d\n", value, returnArray[arrPosition], strlen(value), strlen(returnArray[arrPosition]));
						arrPosition++;	
							
						memset(key, '\0', 256);
						memset(value, '\0', 256);	
						
					}
				}
		
				else if ( jsonData[i] == '"' ){
					continue;
				}
		
				else{
		
					char tmp[2];
					tmp[1] = '\0';
					tmp[0] = jsonData[i];			
					
					if(! (position % 2) ) // even num -> key
						strcat(key, tmp);
					else				// odd num -> value
						strcat(value, tmp);
				}
		}

		printf("arrPosition : %d\n", arrPosition);
		return arrPosition;		


}


