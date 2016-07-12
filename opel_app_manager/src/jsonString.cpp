
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
jsonString::jsonString(char* inputJson){
	//jsonData = new char[MSGBUFSIZE];
	memset(jsonData, '\0', (sizeof(char) * MSGBUFSIZE));	
	strcpy(jsonData, inputJson);
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

char* jsonString::getJsonData(){
	return jsonData;
}

char* jsonString::findValue(char* desKey){
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

					return value;
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

						return value;
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


