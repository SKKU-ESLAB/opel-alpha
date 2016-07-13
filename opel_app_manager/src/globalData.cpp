#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "globalData.h"

//------------------------ app process ---------------------------------



appProcessInfo::appProcessInfo(){
	
}
appProcessInfo::appProcessInfo(char* _appID, char* _appName, int _pid){

	appID = new char[strlen(_appID)];
	strcpy(appID, _appID);
	appName = new char[strlen(_appName)];
	strcpy(appName, _appName);
	pid=_pid;

	
}


appProcessInfo::appProcessInfo(char* _appID, char* _appName, int _pid, char* _appPath){

	appID = new char[strlen(_appID)];
	strcpy(appID, _appID);
	appName = new char[strlen(_appName)];
	strcpy(appName, _appName);

	pid=_pid;

	appPath = new char[strlen(_appPath)];
	strcpy(appPath, _appPath);
}

char* appProcessInfo::getAppProcId(){
	return appID;
}

char* appProcessInfo::getAppProcName(){
		return appName;
}
int appProcessInfo::getAppProcPid(){
	return pid;
}

char* appProcessInfo::getAppPath(){
	return appPath;
}





//------------------------ app process list ---------------------------------

appProcessTable::appProcessTable(){

	if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("[globalData] Constructor >> mutex init failed\n");
    }
}


appProcessTable* appProcessTable::getInstance(){

		if(!appProcessTableInstance) {
			appProcessTableInstance = new appProcessTable();
		
			return appProcessTableInstance;
		}
		else {
		
			return appProcessTableInstance;
		}
}




bool appProcessTable::insertProcess(appProcessInfo process){
	pthread_mutex_lock(&lock);

	procList.push_back(process);
	pthread_mutex_unlock(&lock);
	return true;
}


bool appProcessTable::deleteProcess(appProcessInfo process){
	pthread_mutex_lock(&lock);

	for(appProcIter = procList.begin(); appProcIter != procList.end(); ++appProcIter ){
			if( process.getAppProcId() == appProcIter->getAppProcId()){
	
				procList.erase(appProcIter);
				pthread_mutex_unlock(&lock);
				return true;
			}		
	}	
	pthread_mutex_unlock(&lock);
	return false;

}

bool appProcessTable::deleteProcess(int appID){
	pthread_mutex_lock(&lock);

	for(appProcIter = procList.begin(); appProcIter != procList.end(); ++appProcIter ){
			int tmpID = atoi(appProcIter->getAppProcId());
			if( appID == tmpID ){
	
				procList.erase(appProcIter);
				pthread_mutex_unlock(&lock);
				return true;
			}		
	}	
	pthread_mutex_unlock(&lock);
	return false;

}

appProcessInfo appProcessTable::findProcessByAppName(char* _appName){
	pthread_mutex_lock(&lock);

	for(appProcIter = procList.begin(); appProcIter != procList.end(); ++appProcIter ){
				if( !strcmp(_appName, appProcIter->getAppProcName() )){
					pthread_mutex_unlock(&lock);
					return *appProcIter;
				}
	}	
	pthread_mutex_unlock(&lock);

	printf("[globalData] findProcess_NAME >> No item in the list : app name:%s\n", _appName);	
}

bool appProcessTable::findProcessByAppPid(int _appPid, appProcessInfo* appProc){
	pthread_mutex_lock(&lock);

	for(appProcIter = procList.begin(); appProcIter != procList.end(); ++appProcIter ){
			
			if( _appPid == appProcIter->getAppProcPid() ){
				pthread_mutex_unlock(&lock);

				*appProc = *appProcIter;
//				return *appProcIter;
				
				return true;
			}
	}	
	pthread_mutex_unlock(&lock);

	printf("[globalData] findProcess_PID >> No item in the list : app pid:%d\n", _appPid);

	return false;

}

bool appProcessTable::findProcessByAppId(int _appId, appProcessInfo* appProc){
	pthread_mutex_lock(&lock);

	for(appProcIter = procList.begin(); appProcIter != procList.end(); ++appProcIter ){
			if( _appId == atoi(appProcIter->getAppProcId()) ) {
				pthread_mutex_unlock(&lock);

				*appProc = *appProcIter;
//				return *appProcIter;
				return true;
			}
	}	
	pthread_mutex_unlock(&lock);
	printf("[globalData] findProcess_ID >> No item in the list : app id:%d\n", _appId);
	return false;
}

bool appProcessTable::isExistOnRunningTableByAppID(int id){

		pthread_mutex_lock(&lock);
		for(appProcIter = procList.begin(); appProcIter != procList.end(); ++appProcIter ){
				if( id == atoi(appProcIter->getAppProcId()) ) {

					pthread_mutex_unlock(&lock);
					return true;
				}
		}	
		
		pthread_mutex_unlock(&lock); 
		return false;
}

bool appProcessTable::isExistOnRunningTableByPid(int id){


		pthread_mutex_lock(&lock);
		for(appProcIter = procList.begin(); appProcIter != procList.end(); ++appProcIter ){
				if( id == appProcIter->getAppProcPid() ) {
					pthread_mutex_unlock(&lock);
					return true;
				}
		}	
		pthread_mutex_unlock(&lock);
		return false;
}

