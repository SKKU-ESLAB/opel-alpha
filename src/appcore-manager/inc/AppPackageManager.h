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


#ifndef _APPPACKAGEMANAGER
#define _APPPACKAGEMANAGER

#include <vector>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "GlobalData.h"
#include "AppPackageRepo.h"
#include "JsonString.h"

using namespace std;

class AppPackageManager{

	private:
		AppPackageRepo appPkgRepo;
		AppPackageList appPkgList;
		char mUserAppsPath[1024];
		
	public:
		AppPackageManager();

		JsonString installPackage(char* pkgFilePath);
		bool deletePackage(int appID);
		
		AppPackageList* getAppList();
		AppPackageRepo getAppRepo();


		char* getAppPkgListJson();
		char* getRunningPath(char* appID);
		char* getAppDirPath(char* appID);
		
		void updatePkgList();
};

#endif
