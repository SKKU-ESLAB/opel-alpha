#ifndef _COMMMANAGER
#define _COMMMANAGER


#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>


#include "appStatusManager.h"
#include "appPackageManager.h"
#include "jsonString.h"



#define FILENAMESIZE 256
#define INSTALLPKG				"1000"
#define EXEAPP					"1001"
#define KILLAPP	    			"1002"		
#define DELETEAPP				"1003" 		
#define UPDATEAPPINFO			"1004" 		
#define NOTI					"1005"	
#define UPDATE_SENSOR_INFO		"1006"
#define UPDATE_CAMERA_INFO		"1007"
#define NOTI_PRELOAD_IMG		"1008"	

#define CONFIG_REGISTER			"1009"	
#define CONFIG_EVENT			"1010"	

#define RUN_NATIVE_CAMERAVIEWER "1011"
#define RUN_NATIVE_SENSORVIEWER "1012"
#define TERM_NATIVE_CAMERAVIEWER "2011"
#define TERM_NATIVE_SENSORVIEWER "2012"
#define ANDROID_TERMINATE "1013"


#define RemoteFileManager_getListOfCurPath "1014"
#define RemoteFileManager_requestFile "1015"
#define RemoteFileManager_requestFile_Preload "1016"

#define CloudService_faceRecognition "1017"
#define CloudService_faceRecognition_preload "1018"


#define NIL_TERMINATION			"1100"
#define NIL_CONFIGURATION		"1101"

#define FileAck      51		/* file upload acknowledge */
 	
#define FILEBUFSIZE 4096
#define MSGBUFSIZE 1024
#define NUM_NOTI_ITEM 32


class comManager{

private:
	in_port_t servPort;
	int serv_sock;
	int clnt_sock;
	pthread_mutex_t lock;
	comManager();
	~comManager();

	char PKG_STORAGE_PATH[1024];

public:
	static comManager* getInstance();

	void sendMsg(char* msg);
	bool sendFile(char* filePath, char* fileName);
	bool sendFile(char* fullPath);
	ssize_t getMsg(char* msg);
	void error_handling (char *message);
	int getIpAddress (const char * ifr, char out[]);	
	
	//pid_t HandleExecuteApp(int clnt_sock);
	bool HandleInstallPkg (char *);
//	void HandleDeleteApp();
//	void HandleKillApp();

	void closeConnection();
	bool makeConnection();


	// Send to Android	
	void responsePkgInstallComplete(jsonString js);
	void responsePkgUninstallComplete(jsonString js);
	void responseAppRunComplete(jsonString js);
	void responseAppExitComplete(char* appID);
	void responseUpdatePkgList(jsonString js);
	void responseUpdateSensorList();
	void responseUpdateCameraList();

	void response_Dbus_RegisterTermination(char* json);
	bool response_Dbus_SendNoti(char* json);
	bool response_Dbus_SendConfig(char* json);
	void responseUpdateFileManager(jsonString js);
	void responseRequestFilefromFileManager(jsonString js);
	bool response_Dbus_FaceRecognition(char* json);


};


static comManager* instance;

#endif

