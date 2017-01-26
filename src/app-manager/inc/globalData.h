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

#ifndef _GLOBALDATA
#define _GLOBALDATA

#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;


class appProcessInfo{
	private:
		char* appID;
		char* appName;
		char* appPath;
		int pid;
			
	public:
		appProcessInfo();
		appProcessInfo(char* _appID, char* _appName, int _pid);
		appProcessInfo(char* _appID, char* _appName, int _pid, char* _appPath);

		char* getAppProcId();
		char* getAppProcName();
		int getAppProcPid();
		char* getAppPath();
};



class appProcessTable{
	private:
		vector<appProcessInfo> procList;
		vector<appProcessInfo>::iterator appProcIter;
		pthread_mutex_t lock;
		appProcessTable();
		~appProcessTable();
		
		
	public:
		static appProcessTable* getInstance();
		bool insertProcess(appProcessInfo process);
		bool deleteProcess(appProcessInfo process);
		bool deleteProcess(int appID);
		appProcessInfo findProcessByAppName(char* _appName);
		bool findProcessByAppPid(int _appPid, appProcessInfo* appProc);
		bool findProcessByAppId(int _appId, appProcessInfo* appProc);

		bool isExistOnRunningTableByAppID(int Id); 
		bool isExistOnRunningTableByPid(int Id); 

};

static appProcessTable* appProcessTableInstance;


#endif