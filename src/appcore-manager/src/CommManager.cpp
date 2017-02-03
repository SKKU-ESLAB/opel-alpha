/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: Eunsoo Park<esevan.park@gmail.com>
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

#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>  
#include <string.h>  

#include <sys/socket.h>  
#include <sys/ioctl.h>  
#include <sys/stat.h>  
#include <netinet/in.h>  
#include <net/if.h>  
#include <arpa/inet.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#include "opel_cmfw.h"
#include "GlobalData.h"
#include "CommManager.h"
#include "AppStatusManager.h"
#include "AppPackageManager.h"
#include "JsonString.h"


static const char *INTF_NAME = "Opel Manager";
static sdp_session_t *session_priv;
static CommManager* instance;
CommManager::CommManager(){

	cmfw_init();
	makeConnection();
	char* opelAppsDir;
	opelAppsDir = getenv("OPEL_APPS_DIR");

	sprintf(mUserAppsPath, "%s%s", opelAppsDir, "/user/");

}

void CommManager::closeConnection(){
	fprintf(stderr, "Close connection\n");
	cmfw_close( CMFW_DEFAULT_PORT );
}
bool CommManager::makeConnection(){
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("[CommManager] mutex init failed\n");
		return false;
	}

	int option;
	/* socket() Creation*/
	cmfw_open( CMFW_DEFAULT_PORT );

	/* accept() */
	cmfw_accept( CMFW_DEFAULT_PORT );
	
	return true;
}


CommManager::~CommManager(){
}

CommManager* CommManager::getInstance(){

	if(!instance) {
		instance = new CommManager();
		return instance;
	}
		else {
			return instance;
		}
}
	
void CommManager::sendMsg(const char* msg){
	printf("[CommManager] sendMsg >> msg:%s\n", msg);
	int res = cmfw_send_msg( CMFW_DEFAULT_PORT, msg, strlen(msg));
	if(res < 0){
		printf("[CommManager] send failed\n");
	}
}
		

bool CommManager::sendFile(char* srcFile, char* destFileName){


		size_t fsize = 0, nsize = 0, fpsize = 0;

		char buf[MSGBUFSIZE]={'\0',};


		FILE* sendFile = fopen(srcFile, "r");
//		FILE* sendFile = fopen("/home/pi/testImg.jpg", "r+");	

		if (sendFile == NULL) {
			return false;
		}

		struct stat file_info;
		stat( srcFile, &file_info);
		char str[MSGBUFSIZE]={'\0',};
		fsize = file_info.st_size;
		sprintf(str, "%d", file_info.st_size);

		sendMsg(destFileName);
		sendMsg(str);

		int res = cmfw_send_file( CMFW_DEFAULT_PORT, srcFile );
		printf("[sendFile] %s done %d", srcFile, res);

    	fclose(sendFile);
		return (res == 0);
		

}

bool CommManager::sendFile(char* fullPath){

		size_t fsize = 0, nsize = 0, fpsize = 0;
	
		char buf[MSGBUFSIZE];
    	memset(buf, 0x00, 256);

		FILE* sendFile = fopen(fullPath, "r");

		if (sendFile == NULL) {
			return false;
		}
		
		struct stat file_info;
		stat( fullPath, &file_info);
		char str[MSGBUFSIZE];
		fsize = file_info.st_size;
		sprintf(str, "%d", file_info.st_size);
		
		char fileName[MSGBUFSIZE];
		char* tmp = strrchr(fullPath, '/') + 1;
		strcpy(fileName, tmp);

		sendMsg(fileName);
		sendMsg(str);

		int res = cmfw_send_file( CMFW_DEFAULT_PORT, fullPath );
		printf("[sendFile] %s done %d", fullPath, res);

    	fclose(sendFile);
		return (res == 0);
	
}	
	
ssize_t CommManager::getMsg(char* msg){

	
	int res = cmfw_recv_msg(CMFW_DEFAULT_PORT, msg, MSGBUFSIZE);

	if (res < 0)
		return (ssize_t)res;

	printf("[CommManager] getMsg >> received msg : %s\n", msg);
	return (ssize_t) strlen(msg)+1;
}
	
bool CommManager::HandleInstallPkg (char *rcvFileName){ 
		char rcvFileSize[MSGBUFSIZE] =	{'\0',}; 
		char saveFilePath[MSGBUFSIZE] = {'\0',};
		uint32_t netFileSize = 0;
		uint32_t rcvdFileSize = 0;
	
		//FILE NAME
		ssize_t numBytesRcvd = getMsg(rcvFileName);

		numBytesRcvd = getMsg(rcvFileSize);	
		if(numBytesRcvd == 0 || numBytesRcvd < 0){
			error_handling("Peer connection closed");
			return false;
		}
		netFileSize = atoi(rcvFileSize);
	 
		sprintf(saveFilePath, "%s", mUserAppsPath);

		int res = cmfw_recv_file(CMFW_DEFAULT_PORT, saveFilePath);
		if(res < 0){
			printf("Receving file error\n");
			return false;
		}

		return true;
}
	
void CommManager::error_handling (char *message){
	fputs (message, stderr);
	fputc ('\n', stderr);
	exit (1);
}

//sendJson(CompleteInfo) and File(Icon or APK)
void CommManager::responsePkgInstallComplete(JsonString js){
	pthread_mutex_lock(&lock);

	sendMsg(js.getJsonData().c_str());

	char path[128];
	char name[128];
	strncpy( path,  js.findValue("appPath").c_str(), 128 );
	strncpy( name,  js.findValue("appIconName").c_str(), 128 );

	char fullPath[1024];
	sprintf(fullPath, "%s/%s", path, name);
	
	if( !sendFile(fullPath) ){
		printf("[CommManager] Install Pkg Fail >> sendfile Fair\n");
	}

	
	pthread_mutex_unlock(&lock);
}	

void CommManager::responsePkgUninstallComplete(JsonString js){
	pthread_mutex_lock(&lock);	
	sendMsg(js.getJsonData().c_str());
	pthread_mutex_unlock(&lock);
}

void CommManager::responseAppRunComplete(JsonString js){
	pthread_mutex_lock(&lock);

	JsonString sendJs;								
	sendJs.addType(EXEAPP);					
	char appID[16];
	strncpy(appID, js.findValue("appID").c_str(), 16);
	sendJs.addItem("appID", appID);	

	sendMsg(sendJs.getJsonData().c_str());
	pthread_mutex_unlock(&lock);

}

void CommManager::responseAppExitComplete(char* appID){
	

	char Message[MSGBUFSIZE] = {'\0',};
	sprintf(Message, "{\"type\":\"%s\",\"appID\":\"%s\"}", KILLAPP, appID);

	pthread_mutex_lock(&lock);
	sendMsg(Message);
	pthread_mutex_unlock(&lock);
}

void CommManager::responseUpdatePkgList(JsonString js){
	pthread_mutex_lock(&lock);
	sendMsg(js.getJsonData().c_str());
	pthread_mutex_unlock(&lock);
}

void CommManager::responseUpdateSensorList(){

}

void CommManager::responseUpdateCameraList(){

}

void CommManager::response_Dbus_RegisterTermination(char* json){
	pthread_mutex_lock(&lock);
	sendMsg(json);
	pthread_mutex_unlock(&lock);
}

bool CommManager::response_Dbus_SendNoti(char* json){
	pthread_mutex_lock(&lock);
	JsonString js(json);
	JsonString sendJson;
	char appID[32]={'\0',};
	
	char wholeItem[NUM_NOTI_ITEM][128];
	int length = js.getWholeKeyValue(wholeItem);
	AppProcessTable* appProcList = AppProcessTable::getInstance();
	int i=0;

	for(i=0; i<length; i+=2){
		printf("[comm] wholeItem : %s - %s\n", wholeItem[i], wholeItem[i+1]);
		if( ! strcmp(wholeItem[i], "img") ){

			char fullPath[128]={'\0',};

			//relative path
			if(wholeItem[i+1][0] == '.'){
				//parent path is not implemented
				if(wholeItem[i+1][1] == '.'){
					printf("[CommManager] send Noti Error >> cannot open the file : %s\n", wholeItem[i+1]);

					for(int i =0; i < length; i++)
						free(wholeItem[i]);
					
					pthread_mutex_unlock(&lock);
					return false;
				}

				//current path
				else{
					AppProcessInfo appProc;
					if(  ! AppProcessTable::getInstance()->findProcessByAppId(atoi(appID), &appProc)  ){
						printf("[DbusManager] Cannot find the procInfo >> id : %s\n", appID);

						break;
					}
					
					strcpy(fullPath, appProc.getAppPath());
					strcat(fullPath, "/");
					strncat(fullPath, wholeItem[i+1]+2, strlen(wholeItem[i+1])-2);

					printf("[CommManager >> Relative fullPath : %s |||  %s\n", appProc.getAppPath(), fullPath);
				}
			}

			//absolute path
			else if(wholeItem[i+1][0] == '/'){
				strcpy(fullPath, wholeItem[i+1]);

				fullPath[strlen(fullPath)] = '\0';
				printf("[CommManager >> Absolute fullPath : %s\n", fullPath);
			}

			 
			int nResult = access( fullPath, F_OK );
			if( nResult == -1 )
			{
				printf( "[CommManager] sendNoti >> file doesn't exist : %s\n", fullPath);

				break;
			}
						
			else{
				printf("file can be read\n");
			}
			

			JsonString js;
			js.addType(NOTI_PRELOAD_IMG);
			sendMsg(js.getJsonData().c_str());

			time_t now;
			struct tm t;
			struct tm *t_p;
			char dateTimeFileName[256]={'\0',};
			time(&now);
			localtime_r(&now, &t);
	
			sprintf(dateTimeFileName, "%d_%02d_%02d_%02d_%02d_%02d_%d.jpg", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, i);

			printf("[CommManager] Send Noti_Preload_Img sendFile srcFullPath : %s | destFileName : %s\n", fullPath, dateTimeFileName);

			if( ! sendFile(fullPath, dateTimeFileName) ){
//			if( ! sendFile(fullPath) ){

				pthread_mutex_unlock(&lock);
				printf("[CommManager] send Noti_Preload_Img sendFile Fail\n");

				break;
			}
			else{
				printf("[CommManager] sendFile success\n");
			}

			
			sendJson.addItem("img", dateTimeFileName);
		}
		
		else if( ! strcmp(wholeItem[i], "type") ){
			sendJson.addType(NOTI);
		}	
		else if( ! strcmp(wholeItem[i], "appID") ){
			strcpy(appID, wholeItem[i+1]);
			sendJson.addItem(wholeItem[i], wholeItem[i+1]);
		}	
		
		else {
			sendJson.addItem(wholeItem[i], wholeItem[i+1]);
		}
	}
	
	if(i == length){
		printf("[CommManager] Send Noti page : %s\n", sendJson.getJsonData().c_str() );
		sendMsg(sendJson.getJsonData().c_str());
		
	}
	else{
		printf("[CommManager] i is not equal with length why?? idk what does that mean...\n" );
	}
	
	/*for(int i =0; i < length; i++){
		printf("free wholeItem[%d]\n", i);
		free(wholeItem[i]);
	}*/
	
	pthread_mutex_unlock(&lock);

	return true;
}

bool CommManager::response_Dbus_SendConfig(char* json){

	char tmpJson[MSGBUFSIZE]={'\0',};
	char resultJson[MSGBUFSIZE]={'\0',};

	
	strncpy (tmpJson, json + 15, strlen(json)-15) ;

	sprintf(resultJson, "{\"type\":\"%s\",%s", CONFIG_REGISTER, tmpJson);

	pthread_mutex_lock(&lock);
	sendMsg(resultJson);
	pthread_mutex_unlock(&lock);
	
	return true;
}
void CommManager::responseUpdateFileManager(JsonString js){

	pthread_mutex_lock(&lock);
	sendMsg(js.getJsonData().c_str());
	pthread_mutex_unlock(&lock);
	
}

//Need to keep handling fileName instead of the name based on current time

void CommManager::responseRequestFilefromFileManager(JsonString js){
	
	pthread_mutex_lock(&lock);
	
	time_t now;
	struct tm t;
	struct tm *t_p;
	char dateTimeFileName[256]={'\0',};
	time(&now);
	localtime_r(&now, &t);
		
	sprintf(dateTimeFileName, "%d_%02d_%02d_%02d_%02d_%02d_%d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	
	char path[1024] = {'\0',};
	strncpy(path, js.findValue("path").c_str(), 1024);

	char share[2] = {'\0',};
	strncpy(share, js.findValue("share").c_str(), 2);

	int nResult = access( path, F_OK );

	if( nResult == -1 )
	{
			printf( "[CommManager] requestFilefromFileManager >> file doesn't exist : %s\n", path);
			return;
	}
	
	JsonString js_preload;
	js_preload.addType(RemoteFileManager_requestFile_Preload);
	sendMsg(js_preload.getJsonData().c_str());


	
	if( ! sendFile(path, dateTimeFileName) ){
			pthread_mutex_unlock(&lock);
			printf("[CommManager] requestFilefromFileManager >> send Preload_file Fail\n");
			return;
	}

	
	JsonString sendJp;
	sendJp.addType(RemoteFileManager_requestFile);
	sendJp.addItem("filename", dateTimeFileName);
	sendJp.addItem("originpath", path);
	sendJp.addItem("share", share);

	sendMsg(sendJp.getJsonData().c_str());
	
	pthread_mutex_unlock(&lock);

}



bool CommManager::response_Dbus_FaceRecognition(char* json){

	pthread_mutex_lock(&lock);
	JsonString js(json);
	JsonString sendJson;
	char appID[32]={'\0',};
	
	char wholeItem[NUM_NOTI_ITEM][128];
	int length = js.getWholeKeyValue(wholeItem);
	AppProcessTable* appProcList = AppProcessTable::getInstance();
	int i=0;

	


//	appID\":\"%s\",\"appTitle\":\"%s\",\"imgPath\":
	sendJson.addType(CloudService_faceRecognition);

	for(i=0; i<length; i+=2){
		printf("[comm] wholeItem : %s - %s\n", wholeItem[i], wholeItem[i+1]);
		if( ! strcmp(wholeItem[i], "imgPath") ){

			char fullPath[128]={'\0',};

			//relative path
			if(wholeItem[i+1][0] == '.'){
				//parent path is not implemented
				if(wholeItem[i+1][1] == '.'){
					printf("[CommManager] send faceRecognition Error >> cannot open the file : %s\n", wholeItem[i+1]);

					for(int i =0; i < length; i++)
						free(wholeItem[i]);
					
					pthread_mutex_unlock(&lock);
					return false;
				}

				//current path
				else{
					AppProcessInfo appProc;
					if(  ! AppProcessTable::getInstance()->findProcessByAppId(atoi(appID), &appProc)  ){
						printf("[DbusManager] Cannot find the procInfo >> id : %s\n", appID);

						break;
					}
					
					strcpy(fullPath, appProc.getAppPath());
					strcat(fullPath, "/");
					strncat(fullPath, wholeItem[i+1]+2, strlen(wholeItem[i+1])-2);

					printf("[CommManager >> Relative fullPath : %s\n", fullPath);
				}
			}

			//absolute path
			else if(wholeItem[i+1][0] == '/'){
				strcpy(fullPath, wholeItem[i+1]);

				fullPath[strlen(fullPath)] = '\0';
				printf("[CommManager >> Absolute fullPath : %s\n", fullPath);
			}

			 
			int nResult = access( fullPath, F_OK );
			if( nResult == -1 )
			{
				printf( "[CommManager] FaceRecognition >> file doesn't exist : %s\n", fullPath);

				//break;
			}
						
			else{
				printf("file can be read\n");
			}
			

			JsonString js;
			js.addType(CloudService_faceRecognition_preload);
			sendMsg(js.getJsonData().c_str());

			time_t now;
			struct tm t;
			struct tm *t_p;
			char dateTimeFileName[256]={'\0',};
			time(&now);
			localtime_r(&now, &t);
			

			

			sprintf(dateTimeFileName, "%05d_%04d%02d%02d%02d%02d%02d.jpg", atoi(appID), t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

			printf("[CommManager] Send FaceRecognition_Preload_Img sendFile srcFullPath : %s | destFileName : %s\n", fullPath, dateTimeFileName);

			
			if( ! sendFile(fullPath, dateTimeFileName) ){

				pthread_mutex_unlock(&lock);
				printf("[CommManager] send FaceRecognition_Preload_Img sendFile Fail\n");

				break;
			}
			
			sendJson.addItem("img", dateTimeFileName);
		}
				
		else if( ! strcmp(wholeItem[i], "appID") ){
			strcpy(appID, wholeItem[i+1]);
			sendJson.addItem(wholeItem[i], wholeItem[i+1]);
		}	
		else {
			sendJson.addItem(wholeItem[i], wholeItem[i+1]);
		}
	}
	
	if(i == length){
		printf("[CommManager] Send FaceRecognition page : %s\n", sendJson.getJsonData().c_str() );
		sendMsg(sendJson.getJsonData().c_str());
	
	}
	else{
		printf("[CommManager] i is not equal with length why?? idk what does that mean.... i : %d lenght : %d", i, length);
	}

	pthread_mutex_unlock(&lock);

	return true;
}


int CommManager::getIpAddress (const char * ifr_n, char out[]) {  
   int sockfd; 
   char ipstr[40]; 
   struct ifreq ifr; 


   strncpy(ifr.ifr_name,ifr_n,IFNAMSIZ); 
   sockfd =socket(AF_INET,SOCK_STREAM,0); 

    if (ioctl(sockfd,SIOCGIFADDR,&ifr)< 0 ) 
    { 
        perror("ioctl"); 
        return -1; 
    } 

    inet_ntop(AF_INET,ifr.ifr_addr.sa_data+2,ipstr,sizeof(struct sockaddr)); 

    strcpy(out, ipstr);
    return 0; 

}	

bool CommManager::wfdOn(void){
	return cmfw_wfd_on(CMFW_DEFAULT_PORT);
}
