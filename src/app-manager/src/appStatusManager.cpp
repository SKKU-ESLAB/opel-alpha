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

#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>


#include "appStatusManager.h"
#include "globalData.h"
#include "jsonString.h"



using namespace std;

appStatusManager::appStatusManager(){
		appProcList = appProcessTable::getInstance();
}


//1 : CameraStream, 2 : SensorViewer
int appStatusManager::runNativeJSApp(int type){

	pid_t pid;
	
	char* appPath;
	appPath = getenv("OPEL_APPS_DIR");
	
	if (type == 1){
		pid = fork();
		if(pid == 0){	// Child for executing the application
		
			char _filePath[1024] = {'0', };
			sprintf(_filePath, "%s%s", appPath,  "/system/CameraViewer/index.js");

			char* fullPath[] = {"node", _filePath, NULL};	
			printf("[appStatusManager] runNewApp full run Path : %s\n", _filePath);
			execvp("node", fullPath);
		}
	
		else if (pid > 0){ 	// parent for managing child's PID & mainstream
			printf("[appStatusManager] Fork & Run NativeApp:CameraViewer \n" );		
			return (int)pid;
		}
	}



	else if (type == 2){
		pid = fork();
	
		if(pid == 0){	// Child for executing the application

			char _filePath[1024] = {'0', };
			sprintf(_filePath, "%s%s", appPath,  "/system/SensorViewer/index.js");

			char* fullPath[] = {"node", _filePath, NULL};	
			printf("[appStatusManager] runNewApp full run Path : %s\n", _filePath);
			execvp("node", fullPath);
		}
	
		else if (pid > 0){ 	// parent for managing child's PID & mainstream
			printf("[appStatusManager] Fork & Run NativeApp:SensorViewer \n" );		
			return (int)pid;
		}	
	}
}

bool appStatusManager::runNewApplication(jsonString js, char* _filePath ){

    pid_t pid;
	pid = fork();
	
	if(pid < 0){
		printf("[appStatusManager] Fail to fork\n");

		return false;
	}
	
	else if(pid == 0){	// Child for executing the application
//		char* filePath = strcat("./opelApp/", _filePath);
		
		char* fullPath[] = {"node", _filePath, NULL};	
		printf("[appStatusManager] runNewApp full run Path : %s\n", _filePath);
		execvp("node", fullPath);
	}
	
	else if (pid > 0){ 	// parent for managing child's PID & mainstream

		char appID[128] = {'\0',};
		char appName[128] = {'\0',};
		char appPath[128] = {'\0',};
		char dirLastPath[128] = {'\0',};

		strncpy(appID, js.findValue("appID").c_str(), 128);
		strncpy(appName, js.findValue("appName").c_str(), 128);

		strncpy(dirLastPath, js.findValue("appPath").c_str(), 128);
		getcwd(appPath, 128);

		strncat(appPath, dirLastPath+1, strlen(dirLastPath)-1);

		printf("[appStatusManager] Fork & Run App id %s name %s pid %d path %s\n",
        appID, appName, pid, appPath);		
		appProcessInfo newProcess(appID, appName, pid, appPath);
		appProcList->insertProcess(newProcess);
		
		return true;
	}
}

void appStatusManager::exitApplication(int _appid){
	printf("[appStatusManager] exitApplication >> appID : %d \n", _appid);
	appProcList->deleteProcess(_appid);

}
