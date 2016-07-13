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

#include <opel_cmfw.h>
#include "globalData.h"
#include "commManager.h"
#include "appStatusManager.h"
#include "appPackageManager.h"
#include "jsonString.h"


static const char *INTF_NAME = "Opel Manager";
static sdp_session_t *session_priv;
comManager::comManager(){

	cmfw_init();
	makeConnection();
	char* opelEnvPath;
	opelEnvPath = getenv("OPEL_DIR");

	sprintf(PKG_STORAGE_PATH, "%s%s", opelEnvPath, "/bin/appManager/application/");

}

void comManager::closeConnection(){
	fprintf(stderr, "Close connection\n");
	cmfw_close( CMFW_DEFAULT_PORT );
}
bool comManager::makeConnection(){
	if (pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("[comManager] mutex init failed\n");
		return false;
	}

	int option;
	/* socket() Creation*/
	cmfw_open( CMFW_DEFAULT_PORT );

	/* accept() */
	cmfw_accept( CMFW_DEFAULT_PORT );
	
	return true;
}


comManager::~comManager(){
}

comManager* comManager::getInstance(){

	if(!instance) {
		instance = new comManager();
		return instance;
	}
		else {
			return instance;
		}
}
	
void comManager::sendMsg(char* msg){
	printf("[commManager] sendMsg >> msg:%s\n", msg);
	int res = cmfw_send_msg( CMFW_DEFAULT_PORT, msg, strlen(msg));
	if(res < 0){
		printf("[commManager] send failed\n");
	}
}
		

bool comManager::sendFile(char* srcFile, char* destFileName){


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

bool comManager::sendFile(char* fullPath){

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
	
ssize_t comManager::getMsg(char* msg){

	
	int res = cmfw_recv_msg(CMFW_DEFAULT_PORT, msg, MSGBUFSIZE);

	if (res < 0)
		return (ssize_t)res;

	printf("[commManager] getMsg >> received msg : %s\n", msg);
	return (ssize_t) strlen(msg)+1;
}
	
bool comManager::HandleInstallPkg (char *rcvFileName){ 
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
	 
		sprintf(saveFilePath, "%s", PKG_STORAGE_PATH);

		int res = cmfw_recv_file(CMFW_DEFAULT_PORT, saveFilePath);
		if(res < 0){
			printf("Receving file error\n");
			return false;
		}

		return true;
}
	
void comManager::error_handling (char *message){
	fputs (message, stderr);
	fputc ('\n', stderr);
	exit (1);
}

//sendJson(CompleteInfo) and File(Icon or APK)
void comManager::responsePkgInstallComplete(jsonString js){
	pthread_mutex_lock(&lock);

	sendMsg(js.getJsonData());

	char path[128];
	char name[128];
	strcpy( path,  js.findValue("appPath") );
	strcpy( name,  js.findValue("appIconName") );

	char fullPath[1024];
	sprintf(fullPath, "%s/%s", path, name);
	
	if( !sendFile(fullPath) ){
		printf("[commManager] Install Pkg Fail >> sendfile Fair\n");
	}

	
	pthread_mutex_unlock(&lock);
}	

void comManager::responsePkgUninstallComplete(jsonString js){
	pthread_mutex_lock(&lock);	
	sendMsg(js.getJsonData());
	pthread_mutex_unlock(&lock);
}

void comManager::responseAppRunComplete(jsonString js){
	pthread_mutex_lock(&lock);

	jsonString sendJs;								
	sendJs.addType(EXEAPP);					
	char appID[16];
	strcpy(appID, js.findValue("appID"));
	sendJs.addItem("appID", appID);	

	sendMsg(sendJs.getJsonData());
	pthread_mutex_unlock(&lock);

}

void comManager::responseAppExitComplete(char* appID){
	

	char Message[MSGBUFSIZE] = {'\0',};
	sprintf(Message, "{\"type\":\"%s\",\"appID\":\"%s\"}", KILLAPP, appID);

	pthread_mutex_lock(&lock);
	sendMsg(Message);
	pthread_mutex_unlock(&lock);
}

void comManager::responseUpdatePkgList(jsonString js){
	pthread_mutex_lock(&lock);
	sendMsg(js.getJsonData());
	pthread_mutex_unlock(&lock);
}

void comManager::responseUpdateSensorList(){

}

void comManager::responseUpdateCameraList(){

}

void comManager::response_Dbus_RegisterTermination(char* json){
	pthread_mutex_lock(&lock);
	sendMsg(json);
	pthread_mutex_unlock(&lock);
}

bool comManager::response_Dbus_SendNoti(char* json){
	pthread_mutex_lock(&lock);
	jsonString js(json);
	jsonString sendJson;
	char appID[32]={'\0',};
	
	char wholeItem[NUM_NOTI_ITEM][128];
	int length = js.getWholeKeyValue(wholeItem);
	appProcessTable* appProcList = appProcessTable::getInstance();
	int i=0;

	for(i=0; i<length; i+=2){
		printf("[comm] wholeItem : %s - %s\n", wholeItem[i], wholeItem[i+1]);
		if( ! strcmp(wholeItem[i], "img") ){

			char fullPath[128]={'\0',};

			//relative path
			if(wholeItem[i+1][0] == '.'){
				//parent path is not implemented
				if(wholeItem[i+1][1] == '.'){
					printf("[commManager] send Noti Error >> cannot open the file : %s\n", wholeItem[i+1]);

					for(int i =0; i < length; i++)
						free(wholeItem[i]);
					
					pthread_mutex_unlock(&lock);
					return false;
				}

				//current path
				else{
					appProcessInfo appProc;
					if(  ! appProcessTable::getInstance()->findProcessByAppId(atoi(appID), &appProc)  ){
						printf("[DbusManager] Cannot find the procInfo >> id : %s\n", appID);

						break;
					}
					
					strcpy(fullPath, appProc.getAppPath());
					strcat(fullPath, "/");
					strncat(fullPath, wholeItem[i+1]+2, strlen(wholeItem[i+1])-2);

					printf("[commManager >> Relative fullPath : %s |||  %s\n", appProc.getAppPath(), fullPath);
				}
			}

			//absolute path
			else if(wholeItem[i+1][0] == '/'){
				strcpy(fullPath, wholeItem[i+1]);

				fullPath[strlen(fullPath)] = '\0';
				printf("[commManager >> Absolute fullPath : %s\n", fullPath);
			}

			 
			int nResult = access( fullPath, F_OK );
			if( nResult == -1 )
			{
				printf( "[commManager] sendNoti >> file doesn't exist : %s\n", fullPath);

				break;
			}
						
			else{
				printf("file can be read\n");
			}
			

			jsonString js;
			js.addType(NOTI_PRELOAD_IMG);
			sendMsg(js.getJsonData());

			time_t now;
			struct tm t;
			struct tm *t_p;
			char dateTimeFileName[256]={'\0',};
			time(&now);
			localtime_r(&now, &t);
	
			sprintf(dateTimeFileName, "%d_%02d_%02d_%02d_%02d_%02d_%d.jpg", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, i);

			printf("[commManager] Send Noti_Preload_Img sendFile srcFullPath : %s | destFileName : %s\n", fullPath, dateTimeFileName);

			if( ! sendFile(fullPath, dateTimeFileName) ){
//			if( ! sendFile(fullPath) ){

				pthread_mutex_unlock(&lock);
				printf("[commManager] send Noti_Preload_Img sendFile Fail\n");

				break;
			}
			else{
				printf("[commManager] sendFile success\n");
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
		printf("[commManager] Send Noti page : %s\n", sendJson.getJsonData() );
		sendMsg(sendJson.getJsonData());
		
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

bool comManager::response_Dbus_SendConfig(char* json){

	char tmpJson[MSGBUFSIZE]={'\0',};
	char resultJson[MSGBUFSIZE]={'\0',};

	
	strncpy (tmpJson, json + 15, strlen(json)-15) ;

	sprintf(resultJson, "{\"type\":\"%s\",%s", CONFIG_REGISTER, tmpJson);

	pthread_mutex_lock(&lock);
	sendMsg(resultJson);
	pthread_mutex_unlock(&lock);
	
	return true;
}
void comManager::responseUpdateFileManager(jsonString js){

	pthread_mutex_lock(&lock);
	sendMsg(js.getJsonData());
	pthread_mutex_unlock(&lock);
	
}

//Need to keep handling fileName instead of the name based on current time

void comManager::responseRequestFilefromFileManager(jsonString js){
	
	pthread_mutex_lock(&lock);
	
	time_t now;
	struct tm t;
	struct tm *t_p;
	char dateTimeFileName[256]={'\0',};
	time(&now);
	localtime_r(&now, &t);
		
	sprintf(dateTimeFileName, "%d_%02d_%02d_%02d_%02d_%02d_%d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
	
	char path[1024] = {'\0',};
	strcpy(path, js.findValue("path"));

	char share[2] = {'\0',};
	strcpy(share, js.findValue("share"));

	int nResult = access( path, F_OK );

	if( nResult == -1 )
	{
			printf( "[commManager] requestFilefromFileManager >> file doesn't exist : %s\n", path);
			return;
	}
	
	jsonString js_preload;
	js_preload.addType(RemoteFileManager_requestFile_Preload);
	sendMsg(js_preload.getJsonData());


	
	if( ! sendFile(path, dateTimeFileName) ){
			pthread_mutex_unlock(&lock);
			printf("[commManager] requestFilefromFileManager >> send Preload_file Fail\n");
			return;
	}

	
	jsonString sendJp;
	sendJp.addType(RemoteFileManager_requestFile);
	sendJp.addItem("filename", dateTimeFileName);
	sendJp.addItem("originpath", path);
	sendJp.addItem("share", share);

	sendMsg(sendJp.getJsonData());
	
	pthread_mutex_unlock(&lock);

}



bool comManager::response_Dbus_FaceRecognition(char* json){

	pthread_mutex_lock(&lock);
	jsonString js(json);
	jsonString sendJson;
	char appID[32]={'\0',};
	
	char wholeItem[NUM_NOTI_ITEM][128];
	int length = js.getWholeKeyValue(wholeItem);
	appProcessTable* appProcList = appProcessTable::getInstance();
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
					printf("[commManager] send faceRecognition Error >> cannot open the file : %s\n", wholeItem[i+1]);

					for(int i =0; i < length; i++)
						free(wholeItem[i]);
					
					pthread_mutex_unlock(&lock);
					return false;
				}

				//current path
				else{
					appProcessInfo appProc;
					if(  ! appProcessTable::getInstance()->findProcessByAppId(atoi(appID), &appProc)  ){
						printf("[DbusManager] Cannot find the procInfo >> id : %s\n", appID);

						break;
					}
					
					strcpy(fullPath, appProc.getAppPath());
					strcat(fullPath, "/");
					strncat(fullPath, wholeItem[i+1]+2, strlen(wholeItem[i+1])-2);

					printf("[commManager >> Relative fullPath : %s\n", fullPath);
				}
			}

			//absolute path
			else if(wholeItem[i+1][0] == '/'){
				strcpy(fullPath, wholeItem[i+1]);

				fullPath[strlen(fullPath)] = '\0';
				printf("[commManager >> Absolute fullPath : %s\n", fullPath);
			}

			 
			int nResult = access( fullPath, F_OK );
			if( nResult == -1 )
			{
				printf( "[commManager] FaceRecognition >> file doesn't exist : %s\n", fullPath);

				//break;
			}
						
			else{
				printf("file can be read\n");
			}
			

			jsonString js;
			js.addType(CloudService_faceRecognition_preload);
			sendMsg(js.getJsonData());

			time_t now;
			struct tm t;
			struct tm *t_p;
			char dateTimeFileName[256]={'\0',};
			time(&now);
			localtime_r(&now, &t);
			

			

			sprintf(dateTimeFileName, "%05d_%04d%02d%02d%02d%02d%02d.jpg", atoi(appID), t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);

			printf("[commManager] Send FaceRecognition_Preload_Img sendFile srcFullPath : %s | destFileName : %s\n", fullPath, dateTimeFileName);

			
			if( ! sendFile(fullPath, dateTimeFileName) ){

				pthread_mutex_unlock(&lock);
				printf("[commManager] send FaceRecognition_Preload_Img sendFile Fail\n");

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
		printf("[commManager] Send FaceRecognition page : %s\n", sendJson.getJsonData() );
		sendMsg(sendJson.getJsonData());
	
	}
	else{
		printf("[CommManager] i is not equal with length why?? idk what does that mean.... i : %d lenght : %d", i, length);
	}

	pthread_mutex_unlock(&lock);

	return true;
}


int comManager::getIpAddress (const char * ifr_n, char out[]) {  
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

