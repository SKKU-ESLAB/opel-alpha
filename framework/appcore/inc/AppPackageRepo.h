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


#ifndef _APPPACKAGEREPO
#define _APPPACKAGEREPO

#include <vector>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "RepoClass.h" 

#define fileNameLength 32

using namespace std;

class AppPackage{
	private:
		int apID;
		char* apName;
		char* apFilePath;
		char* apExePath;

	public:
		AppPackage(int _apID, char* _apName, char* _apFilePath, char* _apExePath);
		AppPackage(AppPackage* arg);
		~AppPackage();

		
		int getApID();
		char* getApName();
		char* getApFilePath();
		char* getApExePath();
		
		void setApName(char* _apName);
		void setApFilePath(char* _apFilePath);
		
		void setApExePath(char* _apExePath);

		void printAp();
		AppPackage& operator=(const AppPackage &arg);
		
};


class AppPackageList{

	private:
	vector<AppPackage*> apList;
	vector<AppPackage*>::iterator apIter;

	public:
		AppPackageList();

		vector<AppPackage*>* getListVector();

			
	
		void insert(AppPackage* ap);

		AppPackage* find(int apID);

			
		void erase(AppPackage* ap);
		
		void clear();
};


class AppPackageRepo : public RepoClass{

	private:
	public:
		AppPackageRepo() ;
	
		int insertAppPackage(char* apName, char* apFilePath, char* apExePath);

 		void deleteAppPackage(int apID);

 		void updateAppPackage(AppPackage* ap);
		
 		AppPackage* selectAppPackage(int apID);
			
		void selectAllAppPackage(AppPackageList& allApList);



};


#endif

