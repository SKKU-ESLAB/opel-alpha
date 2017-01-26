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


#ifndef _APPPACKAGEREPO
#define _APPPACKAGEREPO

#include <vector>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "_repoClass.h" 

#define fileNameLength 32

using namespace std;

class appPackage{
	private:
		int apID;
		char* apName;
		char* apFilePath;
		char* apExePath;

	public:
		appPackage(int _apID, char* _apName, char* _apFilePath, char* _apExePath);
		appPackage(appPackage* arg);
		~appPackage();

		
		int getApID();
		char* getApName();
		char* getApFilePath();
		char* getApExePath();
		
		void setApName(char* _apName);
		void setApFilePath(char* _apFilePath);
		
		void setApExePath(char* _apExePath);

		void printAp();
		appPackage& operator=(const appPackage &arg);
		
};


class appPackageList{

	private:
	vector<appPackage*> apList;
	vector<appPackage*>::iterator apIter;

	public:
		appPackageList();

		vector<appPackage*>* getListVector();

			
	
		void insert(appPackage* ap);

		appPackage* find(int apID);

			
		void erase(appPackage* ap);
		
		void clear();
};


class appPackageRepo : public repoClass{

	private:
	public:
		appPackageRepo() ;
	
		int insertAppPackage(char* apName, char* apFilePath, char* apExePath);

 		void deleteAppPackage(int apID);

 		void updateAppPackage(appPackage* ap);
		
 		appPackage* selectAppPackage(int apID);
			
		void selectAllAppPackage(appPackageList& allApList);



};


#endif

