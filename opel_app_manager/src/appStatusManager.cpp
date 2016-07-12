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
	
	char* dirPath;
	dirPath = getenv("OPEL_DIR");
	
	if (type == 1){
		pid = fork();
		if(pid == 0){	// Child for executing the application
		
			char _filePath[1024] = {'0', };
			sprintf(_filePath, "%s%s", dirPath,  "/bin/appManager/nativeJSApp/CameraViewer/index.js");

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
			sprintf(_filePath, "%s%s", dirPath,  "/bin/appManager/nativeJSApp/SensorViewer/index.js");

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
		char dirPath[128] = {'\0',};
		char dirLastPath[128] = {'\0',};

		strcpy(appID, js.findValue("appID"));
		strcpy(appName, js.findValue("appName"));

		strcpy(dirLastPath, js.findValue("dirPath"));
		getcwd(dirPath, 128);

		strncat(dirPath, dirLastPath+1, strlen(dirLastPath)-1);

		printf("[appStatusManager] Fork & Run App appId %s appName %s pid %d dirPath %s\n", appID, appName, pid, dirPath);		
		appProcessInfo newProcess(appID, appName, pid, dirPath);
		appProcList->insertProcess(newProcess);
		
		return true;
	}
}

void appStatusManager::exitApplication(int _appid){
	printf("[appStatusManager] exitApplication >> appID : %d \n", _appid);
	appProcList->deleteProcess(_appid);

}



