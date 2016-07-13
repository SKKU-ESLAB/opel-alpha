#ifndef _SERVICEDAEMONREPO
#define _SERVICEDAEMONREPO


#include <vector>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "_repoClass.h" 

using namespace std;

class serviceDaemon{
	private:
		int smID;
		char* smName;
		char* smFilePath;
		char* smIpcPath;

	public:
		serviceDaemon(int _smID, char* _smName, char* _smFilePath, char* _smIpcPath);
		
		int getSmID();
		char* getSmName();
		char* getSmFilePath();
		char* getSmIpcPath();
		
		void setSmName(char* _smName);
		void setSmFilePath(char* _smFilePath);
		
		void setSmIpcPath(char* _smIpcPath);

		void printSm();
};


class serviceDaemonList{

	private:
	vector<serviceDaemon> smList;
	vector<serviceDaemon>::iterator smIter;

	public:
		serviceDaemonList();
		serviceDaemonList(vector<serviceDaemon> _smList);

		void printAllSmList();
		void insert(serviceDaemon sm);

		serviceDaemon find(int smID);

		void erase(serviceDaemon sm);
};


class serviceDaemonRepo : public repoClass{

	private:
	public:
		serviceDaemonRepo() ;
	
		void insertServiceManager(char* smName, char* smPath, char* smIpcPath);

 		void deleteServiceManager(int smID);

 		void updateServiceManager(serviceDaemon* sm);
		
 		serviceDaemon* selectServiceManager(int smID);
			
		void selectAllServiceManager(serviceDaemonList& allSmList);

};

#endif


