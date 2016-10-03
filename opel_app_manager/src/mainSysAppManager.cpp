#include <vector>


#include "mainSysAppManager.h"
#include "globalData.h"
#include "deviceManager.h"
#include "serviceDaemonManager.h"
#include "appStatusManager.h"
#include "appPackageManager.h"
#include "DbusManager.h"
#include "memoryManager.h"
#include "commManager.h"
#include "remoteFileManager.h"
#include "jsonString.h"


deviceManager devManager;
serviceDaemonManager smManager; //keep tracking pid of running process (service)
memoryManager memManager;
appPackageManager apManager;
appStatusManager asManager; //keep tracking pid of running process (user app)
DbusManager dbusManager;
comManager* cm;
remoteFileManager rfm;

int pidOfCameraViewer;
int pidOfSensorViewer;

void sigchld_handler(int signum){

	int status;
	int pid;

	pid = wait(&status);

	if(pid > 0 && pid != pidOfCameraViewer && pid != pidOfSensorViewer){
		printf("[Main] SIGCHLD Handler >> Child was killed [User app pid : %d]\n", pid);

		//[MORE] send D-bus signal to sensor/camera manager
		//[MORE] update killed app info to android

		char appID[4] = {'\0',};
		appProcessInfo appProc;
		if(  ! appProcessTable::getInstance()->findProcessByAppPid(pid, &appProc)  ){
			printf("[sigchld handler] Cannot find the procInfo >> pid : %d\n", pid);
			return ;
		}
		
		strcpy(appID, appProc.getAppProcId());

		asManager.exitApplication(atoi(appID));
		cm->responseAppExitComplete(appID);
		//		dbusManager.Sensor_all_request_unregister(pid);
	}

	else{
		if(pid == pidOfCameraViewer){
			printf("[Main] SIGCHLD Handler >> Native Camera Child was killed\n");
			pidOfCameraViewer = 0;
		}
		else if(pid == pidOfSensorViewer){
			printf("[Main] SIGCHLD Handler >> Native Sensor Child was killed\n");
			pidOfSensorViewer = 0;
		}
	}

}



int main(){

	signal(SIGCHLD, sigchld_handler);
	char rsBuf[512] = {'/0',};
	
	cm = comManager::getInstance();
	appProcessTable* appProcList = appProcessTable::getInstance();

	while(1){
		char rcvMsg[MSGBUFSIZE] = {'\0',};

		ssize_t numBytesRcvd = cm->getMsg(rcvMsg);

		if (numBytesRcvd == 0 || numBytesRcvd < 0){
			printf("peer connection closed, accept again\n");
			cm->closeConnection();
			cm->makeConnection();
			continue;
		}

		printf("Received Msg : %s\n", rcvMsg);
		jsonString js(rcvMsg);		

    char msgType[1024];
    strncpy(msgType, js.findValue("type").c_str(), 1024);
//		char* msgType = js.findValue("type");	

		if (!strcmp(msgType,INSTALLPKG)){
			char pkgFileName[MSGBUFSIZE] = {0,};

			if(!(cm->HandleInstallPkg(pkgFileName)))
				continue;

			jsonString ret_js = apManager.installPackage(pkgFileName);	
			
			if ( &ret_js != NULL){		
				
				char pkgFileName[1024] = {'\0',};
				strncpy(pkgFileName, js.findValue("pkgFileName").c_str(), 1024);				
				ret_js.addItem("pkgFileName", pkgFileName);
				
				cm->responsePkgInstallComplete(ret_js);	
			}
		}

		else if(!strcmp(msgType, EXEAPP)){
			printf("[MAIN] Request >> EXE App\n");

			char appID[16]={'\0',};
			strncpy(appID, js.findValue("appID").c_str(), 16);

			if( !appProcList->isExistOnRunningTableByAppID( atoi(appID) ) ){
 
				char* runPath = apManager.getRunningPath(appID);
				char* dirPath = apManager.getAppDirPath(appID);	

				js.addItem("dirPath",dirPath);

				printf("[MAIN] run app runpath : %s, dir path : %s\n", runPath, dirPath);

				if ( asManager.runNewApplication(js, runPath) ){

					cm->responseAppRunComplete(js);
				}

				//pid_t pid = HandleExecuteApp(clnt_sock); 

				delete runPath;
				delete dirPath;
				
			}
			else{
				printf("[MAIN] appID : %s is already running\n", appID );
			}
		}

		else if(!strcmp(msgType,KILLAPP)){
			printf("[MAIN] Request >> KILL App\n");

			if( appProcList->isExistOnRunningTableByAppID( atoi(js.findValue("appID").c_str()) )){

				if ( dbusManager.makeTerminationEvent(js) ){

				}
			}
			else{
				printf("[MAIN] appID : %s is already dead\n", js.findValue("appID").c_str() );
			}

		}

		else if(!strcmp(msgType,UPDATEAPPINFO)){
			printf("[MAIN] Request >> Update App Infomation\n");

			vector<appPackage*> *apList = apManager.getAppList()->getListVector();
			vector<appPackage*>::iterator apIter;

			jsonString js;
			js.addType(UPDATEAPPINFO);

			for(apIter = apList->begin(); apIter != apList->end(); ++apIter ){

				int appID_ = (*apIter)->getApID();

				char appID[16]= {'\0',};
				sprintf(appID, "%d", appID_);

				char appName[fileNameLength]= {'\0',};
				strcpy( appName, (*apIter)->getApName() );

				if(appProcList->isExistOnRunningTableByAppID(appID_)){
					strcat(appID, "/1");

				}

				else{
					strcat(appID, "/0");

				}

				js.addItem(appID, appName);

			}	

			char addr[64] = {0,};  
  
			if (cm->getIpAddress("wlan0", addr) > 0) {  
			        printf("[CommManager] get IP(wlan0) : %s\n", addr);  
  			}  			

			else if (cm->getIpAddress("eth0", addr) > 0) {  
			        printf("[CommManager] get IP(eth0) : %s\n", addr);
  			}  
			js.addItem("IP_ADDR__a", addr);

			cm->responseUpdatePkgList(js.getJsonData().c_str());
		}


		else if(!strcmp(msgType,CONFIG_EVENT)){
			printf("[MAIN] Request >> Config Setting Event\n");

			if( appProcList->isExistOnRunningTableByAppID( atoi(js.findValue("appID").c_str()) )){

				if ( dbusManager.makeConfigEvent(js) ){
					
				}
			}
			else{
				printf("[MAIN] appID : %s is already dead\n", js.findValue("appID").c_str() );
			}
		}

		else if(!strcmp(msgType,RUN_NATIVE_CAMERAVIEWER)){
			if(pidOfCameraViewer == 0){
				while(false == cm->wfdOn()){
					sleep(1);
				}
				pidOfCameraViewer = asManager.runNativeJSApp(1); 

			}
		}

		else if(!strcmp(msgType,RUN_NATIVE_SENSORVIEWER)){
			pidOfSensorViewer = asManager.runNativeJSApp(2);	
		}
		
		else if(!strcmp(msgType, TERM_NATIVE_CAMERAVIEWER)){
			if(pidOfCameraViewer != 0){
				kill(pidOfCameraViewer, SIGKILL);
				dbusManager.sendTerminationToCameraManager();
			}
		}

		else if(!strcmp(msgType, TERM_NATIVE_SENSORVIEWER)){
			if(pidOfSensorViewer != 0){
				kill(pidOfSensorViewer, SIGKILL);
		
			}
		}
		
		else if(!strcmp(msgType,ANDROID_TERMINATE)){
			printf("Android activity backed or pause\n");
			cm->closeConnection();
			cm->makeConnection();
			continue;
		}

		else if(!strcmp(msgType,DELETEAPP)){
			printf("[MAIN] Request >> DELETE App\n");

			char appID[16]={'\0',};
			strcpy(appID, js.findValue("appID").c_str());

			
			if( !appProcList->isExistOnRunningTableByAppID( atoi(appID) ) ){
				 //Delete whole of the file and update DB
				 if(apManager.deletePackage(atoi(appID))){	
					//jsonString ret_js;
					//ret_js.addType(DELETEAPP);
					//ret_js.addItem("appID", appID);
					
					cm->responsePkgUninstallComplete(js);
				 }
				 else{
					printf("[MAIN] appID : %s fail to delete\n", appID );				 	
				 }
			}
			else{
				printf("[MAIN] appID : %s is running, Cannot remove this app\n", appID );
			}		
		}


		else if(!strcmp(msgType,RemoteFileManager_getListOfCurPath)){

			char path[1024] = {'\0',};
			strcpy(path, js.findValue("path").c_str());

			jsonString sendJp;
			sendJp.addType(RemoteFileManager_getListOfCurPath);
			rfm.seekDir(path,&sendJp);

			cm->responseUpdateFileManager(sendJp);	


		}
		
		else if(!strcmp(msgType,RemoteFileManager_requestFile)){

			cm->responseRequestFilefromFileManager(js);
		}

		else{
			printf("[MAIN] error_not define msg : %s\n", msgType);
		}
	}	
}

