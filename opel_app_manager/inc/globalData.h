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
