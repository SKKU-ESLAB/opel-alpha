/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
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

#include "GlobalData.h"

//------------------------ app process ---------------------------------



AppProcessInfo::AppProcessInfo(){
	
}
AppProcessInfo::AppProcessInfo(char* _appID, char* _appName, int _pid){

	appID = new char[strlen(_appID)];
	strcpy(appID, _appID);
	appName = new char[strlen(_appName)];
	strcpy(appName, _appName);
	pid=_pid;

	
}


AppProcessInfo::AppProcessInfo(char* _appID, char* _appName, int _pid, char* _appPath){

	appID = new char[strlen(_appID)];
	strcpy(appID, _appID);
	appName = new char[strlen(_appName)];
	strcpy(appName, _appName);

	pid=_pid;

	appPath = new char[strlen(_appPath)];
	strcpy(appPath, _appPath);
}

char* AppProcessInfo::getAppProcId(){
	return appID;
}

char* AppProcessInfo::getAppProcName(){
		return appName;
}
int AppProcessInfo::getAppProcPid(){
	return pid;
}

char* AppProcessInfo::getAppPath(){
	return appPath;
}





//------------------------ app process list ---------------------------------

AppProcessTable::AppProcessTable(){

	if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("[GlobalData] Constructor >> mutex init failed\n");
    }
}


AppProcessTable* AppProcessTable::getInstance(){

		if(!appProcessTableInstance) {
			appProcessTableInstance = new AppProcessTable();
		
			return appProcessTableInstance;
		}
		else {
		
			return appProcessTableInstance;
		}
}




bool AppProcessTable::insertProcess(AppProcessInfo process){
	pthread_mutex_lock(&lock);

	procList.push_back(process);
	pthread_mutex_unlock(&lock);
	return true;
}


bool AppProcessTable::deleteProcess(AppProcessInfo process){
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

bool AppProcessTable::deleteProcess(int appID){
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

AppProcessInfo AppProcessTable::findProcessByAppName(char* _appName){
	pthread_mutex_lock(&lock);

	for(appProcIter = procList.begin(); appProcIter != procList.end(); ++appProcIter ){
				if( !strcmp(_appName, appProcIter->getAppProcName() )){
					pthread_mutex_unlock(&lock);
					return *appProcIter;
				}
	}	
	pthread_mutex_unlock(&lock);

	printf("[GlobalData] findProcess_NAME >> No item in the list : app name:%s\n", _appName);	
}

bool AppProcessTable::findProcessByAppPid(int _appPid, AppProcessInfo* appProc){
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

	printf("[GlobalData] findProcess_PID >> No item in the list : app pid:%d\n", _appPid);

	return false;

}

bool AppProcessTable::findProcessByAppId(int _appId, AppProcessInfo* appProc){
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
	printf("[GlobalData] findProcess_ID >> No item in the list : app id:%d\n", _appId);
	return false;
}

bool AppProcessTable::isExistOnRunningTableByAppID(int id){

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

bool AppProcessTable::isExistOnRunningTableByPid(int id){


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
