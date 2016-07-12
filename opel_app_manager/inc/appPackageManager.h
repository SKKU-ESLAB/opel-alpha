#ifndef _APPPACKAGEMANAGER
#define _APPPACKAGEMANAGER

#include <vector>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "globalData.h"
#include "appPackageRepo.h"
#include "jsonString.h"

//#define PKG_STORAGE_PATH "/application/"

using namespace std;

class appPackageManager{

	private:
		appPackageRepo appPkgRepo;
		appPackageList appPkgList;
		char PKG_STORAGE_PATH[1024];
		
	public:
		appPackageManager();

		jsonString installPackage(char* pkgFilePath);
		bool deletePackage(int appID);
		
		appPackageList* getAppList();
		appPackageRepo getAppRepo();


		char* getAppPkgListJson();
		char* getRunningPath(char* appID);
		char* getAppDirPath(char* appID);
		
		void updatePkgList();
};

#endif
