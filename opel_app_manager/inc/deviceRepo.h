#ifndef _DEVREPO
#define _DEVREPO


#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>


#include "_repoClass.h" 


using namespace std;


class device{
	private:
		int devID;
		char* devName;
		char* devStatus;

	public:
		device(int _devID, char* _devName, char* _devStatus);
		
		int getDevID();
		char* getDevName();
		char* getDevStatus();

		
		void setDevName(char* _devName);

		void setDevStatus(char* _devStatus);

		void printDev();

};


class deviceList{

	private:
	vector<device> devList;
	vector<device>::iterator devIter;

	public:
		deviceList();
		deviceList(vector<device> _devList);

		void printAllDevList();

		void insert(device dev);

		device find(int devID);

		void erase(device dev);

};

class deviceRepo : public repoClass{

	private:
	public:
		deviceRepo();
	
		void insertDevice(char* devName, char* devStatus);

 		void deleteDevice(int devID);

		
 		void updateDevice(device* dev);
		
 		device* selectDevice(int devID);
		
		void selectAllDevice(deviceList& allDevList);

};

#endif

