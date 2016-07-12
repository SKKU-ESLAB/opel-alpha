#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>


#include "deviceRepo.h"

#include "_repoClass.h" 


using namespace std;



device::device(int _devID, char* _devName, char* _devStatus){
	devID = _devID;
		
	devName = new char[strlen(_devName)];
	strcpy(devName, _devName);
			
	devStatus = new char[strlen(_devStatus)];
	strcpy(devStatus, _devStatus);
}
		
int device::getDevID() { 	return devID;		}
char* device::getDevName()  {	return devName;		}
char* device::getDevStatus(){	return devStatus;	}


void device::setDevName(char* _devName){ 	
	delete[] devName;
	devName = new char[strlen(_devName)];
	strcpy(devName, _devName); 		
}

void device::setDevStatus(char* _devStatus){ 	
	delete[] devStatus;
	devStatus = new char[strlen(_devStatus)];
	strcpy(devStatus, _devStatus); 		
}

void device::printDev(){
	cout << "ID > " << devID << "  Name > " << devName << "  Status > " << devStatus << endl;
}




deviceList::deviceList(){}
deviceList::deviceList(vector<device> _devList){
	devList = _devList;
}

void deviceList::printAllDevList(){
	for(devIter = devList.begin(); devIter != devList.end(); ++devIter ){
		devIter->printDev();
	}	
}

void deviceList::insert(device dev){
	devList.push_back(dev);
}

device deviceList::find(int devID){
	for(devIter = devList.begin(); devIter != devList.end(); ++devIter ){
		if( devID == devIter->getDevID()){
			return *devIter;
		}
	}	
}

void deviceList::erase(device dev){

	for(devIter = devList.begin(); devIter != devList.end(); ++devIter ){
		if( dev.getDevID() == devIter->getDevID()){
			dev.printDev();
			devList.erase(devIter);
		}		
	}	
}

		



deviceRepo::deviceRepo() {
	char createTableQuery[] = "CREATE TABLE IF NOT EXISTS DEVICE("  \
						   "DEVID INTEGER PRIMARY KEY AUTOINCREMENT," \
						   "DEVNAME           TEXT    NOT NULL," \
						   "DEVSTATUS         TEXT ); ";

	char dbPath[] = "deviceRepository.db";

	setCreateTableQuery(createTableQuery);
	setDbPath(dbPath);
	openDB();
}
	
void deviceRepo::insertDevice(char* devName, char* devStatus){
	char* insertQuery;
	insertQuery = new char[strlen(devName) + strlen(devStatus) + 128];
	
	sprintf(insertQuery, "INSERT INTO DEVICE (DEVNAME,DEVSTATUS) VALUES ('%s','%s ');", devName, devStatus);
	cout << insertQuery << endl;
	exeQuery(insertQuery);

	delete[] insertQuery;
}

void deviceRepo::deleteDevice(int devID){
	char* deleteQuery;
	deleteQuery = new char[256];

	sprintf(deleteQuery, "DELETE from DEVICE where DEVID=%d;", devID);
	exeQuery(deleteQuery);
	delete[] deleteQuery;
}
		
void deviceRepo::updateDevice(device* dev){
	char* updateQuery;
	updateQuery = new char[strlen(dev->getDevName())+ strlen(dev->getDevStatus()) + 256];

	sprintf(updateQuery, "UPDATE DEVICE set DEVNAME = '%s', DEVSTATUS='%s' where DEVID=%d;", dev->getDevName(), dev->getDevStatus(), dev->getDevID());
	exeQuery(updateQuery);

	delete[] updateQuery;
}
		
device* deviceRepo::selectDevice(int devID){
	char* selectQuery = new char[256];
	sqlite3_stmt * stmt;
	device* dev;
			
	sprintf(selectQuery, "SELECT * from DEVICE where DEVID=%d", devID);
	stmt = prepareQuery(selectQuery);
	if(sqlite3_step(stmt) == SQLITE_ROW){
		dev = new device(sqlite3_column_int(stmt, 0), (char*)sqlite3_column_text (stmt, 1), (char*)sqlite3_column_text(stmt, 2));				
	}

	delete[] selectQuery;
	sqlite3_finalize(stmt);

	return dev;
}
		
void deviceRepo::selectAllDevice(deviceList& allDevList){
	device* tmpDev;
	sqlite3_stmt * stmt;

	stmt = prepareQuery("SELECT * from DEVICE");
	while(sqlite3_step(stmt) == SQLITE_ROW){
		tmpDev = new device(sqlite3_column_int(stmt, 0), (char*)sqlite3_column_text (stmt, 1), (char*)sqlite3_column_text(stmt, 2));
		allDevList.insert(*tmpDev);
	}

	sqlite3_finalize(stmt);
			
}




