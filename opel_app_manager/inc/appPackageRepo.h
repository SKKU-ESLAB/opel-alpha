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

