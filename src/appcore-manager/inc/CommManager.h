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


#include "AppStatusManager.h"
#include "AppPackageManager.h"
#include "JsonString.h"



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
//#define NIL_MSG_TO_SENSOR_VIEWER "1102"

#define FileAck      51		/* file upload acknowledge */
 	
#define FILEBUFSIZE 4096
#define MSGBUFSIZE 1024
#define NUM_NOTI_ITEM 32


class CommManager{

private:
	in_port_t servPort;
	int serv_sock;
	int clnt_sock;
	pthread_mutex_t lock;
	CommManager();
	~CommManager();

	char mUserAppsPath[1024];

public:
	static CommManager* getInstance();

	void sendMsg(const char* msg);
	bool sendFile(char* filePath, char* fileName);
	bool sendFile(char* fullPath);
	ssize_t getMsg(char* msg);
	void error_handling (char *message);
	int getIpAddress (const char * ifr, char out[]);	
	
	//pid_t HandleExecuteApp(int clnt_sock);
	bool HandleInstallPkg (char *);
//	void HandleDeleteApp();
//	void HandleKillApp();

  void initialize();
	void closeConnection();
	bool makeConnection();


	// Send to Android	
	void responsePkgInstallComplete(JsonString js);
	void responsePkgUninstallComplete(JsonString js);
	void responseAppRunComplete(JsonString js);
	void responseAppExitComplete(char* appID);
	void responseUpdatePkgList(JsonString js);
	void responseUpdateSensorList();
	void responseUpdateCameraList();

	void response_Dbus_RegisterTermination(char* json);
	bool response_Dbus_SendNoti(char* json);
	bool response_Dbus_SendConfig(char* json);
	void responseUpdateFileManager(JsonString js);
	void responseRequestFilefromFileManager(JsonString js);
  bool response_Dbus_MsgToSensorViewer(char* json);
	bool response_Dbus_FaceRecognition(char* json);

	bool wfdOn(void);

};
#endif
