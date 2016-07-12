#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "appPackageRepo.h"
#include "_repoClass.h"

using namespace std;



appPackage::appPackage(int _apID, char* _apName, char* _apFilePath, char* _apExePath){
			apID = _apID;
		
			apName = new char[strlen(_apName)+1];
			strcpy(apName, _apName);
			
			apFilePath = new char[strlen(_apFilePath)+1];
			strcpy(apFilePath, _apFilePath);

			apExePath = new char[strlen(_apExePath)+1];
			strcpy(apExePath, _apExePath);
}

appPackage::appPackage(appPackage* arg){
		apID =arg->apID;
		
		apName = new char[strlen(arg->apName)+1];
		strcpy(apName, arg->apName);
			
		apFilePath = new char[strlen(arg->apFilePath)+1];
		strcpy(apFilePath, arg->apFilePath);

		apExePath = new char[strlen(arg->apExePath)+1];
		strcpy(apExePath, arg->apExePath);
}


appPackage::~appPackage(){
			delete[] apName;
			delete[] apFilePath;
			delete[] apExePath;
}

appPackage& appPackage::operator=(const appPackage &arg){
	apID =arg.apID;
	apName = new char[strlen(arg.apName)+1];
	strcpy(apName, arg.apName);
			
	apFilePath = new char[strlen(arg.apFilePath)+1];
	strcpy(apFilePath, arg.apFilePath);

	apExePath = new char[strlen(arg.apExePath)+1];
	strcpy(apExePath, arg.apExePath);
	
}

		
int appPackage::getApID() { 	return apID;		}
char* appPackage::getApName()  {	return apName;		}
char* appPackage::getApFilePath(){	return apFilePath;	}
char* appPackage::getApExePath(){	return apExePath;	}

		
void appPackage::setApName(char* _apName){ 	
			delete[] apName;
			apName = new char[strlen(_apName)];
			strcpy(apName, _apName); 		
}

void appPackage::setApFilePath(char* _apFilePath){ 	
		delete[] apFilePath;
		apFilePath = new char[strlen(_apFilePath)];
		strcpy(apFilePath, _apFilePath); 		
}
		
void appPackage::setApExePath(char* _apExePath){	
		delete[] apExePath;
		apExePath = new char[strlen(_apExePath)];
		strcpy(apExePath, _apExePath);		
}

void appPackage::printAp(){
		cout << "ID > " << apID << "  Name > " << apName << "  Path > " << apFilePath << "  EXEPath > " << apExePath << endl;
}






appPackageList::appPackageList(){}

vector<appPackage*>* appPackageList::getListVector(){

	return &apList;
}

void appPackageList::insert(appPackage* ap){		
		appPackage *tmp = new appPackage(ap);
		apList.push_back(tmp);
}

appPackage* appPackageList::find(int apID){
		for(apIter = apList.begin(); apIter != apList.end(); ++apIter ){
			if( apID == (*apIter)->getApID()){
				return *apIter;
			}
		}	
}
void appPackageList::erase(appPackage* ap){
		for(apIter = apList.begin(); apIter != apList.end(); ++apIter ){
			if( ap->getApID() == (*apIter)->getApID()){
				delete *apIter;
				apList.erase(apIter);				
			}		
		}	
}


void appPackageList::clear(){
	apList.clear();
}








appPackageRepo::appPackageRepo() {
		char createTableQuery[] = "CREATE TABLE IF NOT EXISTS APPPACKAGE("  \
					   "APID INTEGER PRIMARY KEY AUTOINCREMENT," \
					   "APNAME           TEXT    NOT NULL," \
					   "APFILEPATH           TEXT    NOT NULL," \
					   "APEXEPATH         TEXT NOT NULL); ";
		char dbPath[] = "appPackageRepository.db";
	setCreateTableQuery(createTableQuery);
		setDbPath(dbPath);
		openDB();
}

int appPackageRepo::insertAppPackage(char* apName, char* apFilePath, char* apExePath){
		char* insertQuery;
		insertQuery = new char[strlen(apName) + strlen(apFilePath) + strlen(apExePath) + 128];
		printf("[appPkgRepo] insertAppPackage , apName : %s, apFilePath : %s, apExePath : %s\n", apName, apFilePath, apExePath);
		sprintf(insertQuery, "INSERT INTO APPPACKAGE (APNAME,APFILEPATH,APEXEPATH) VALUES ('%s','%s','%s');", apName, apFilePath, apExePath );
		cout << insertQuery << endl;
		exeQuery(insertQuery);
		delete[] insertQuery;

		sqlite3_int64 id = sqlite3_last_insert_rowid(db);
		int _id = (int) id;

		return _id;
		
}

void appPackageRepo::deleteAppPackage(int apID){
		char* deleteQuery;
		deleteQuery = new char[256];
		sprintf(deleteQuery, "DELETE from APPPACKAGE where APID=%d;", apID);
		exeQuery(deleteQuery);
		delete[] deleteQuery;
}

		
void appPackageRepo::updateAppPackage(appPackage* ap){
	char* updateQuery;
	updateQuery = new char[strlen(ap->getApName())+ strlen(ap->getApFilePath()) + strlen(ap->getApExePath()) + 256];

	sprintf(updateQuery, "UPDATE APPPACKAGE set APNAME = '%s', APFILEPATH='%s', APEXEPATH='%s' where APID=%d;", 
													ap->getApName(), ap->getApFilePath(),ap->getApExePath(), ap->getApID());
			
	exeQuery(updateQuery);

	delete[] updateQuery;
}
		
appPackage* appPackageRepo::selectAppPackage(int apID){
	char* selectQuery = new char[256];
	sqlite3_stmt * stmt;
	appPackage* ap = NULL;
	
	sprintf(selectQuery, "SELECT * from APPPACKAGE where APID=%d", apID);
	stmt = prepareQuery(selectQuery);
	if(sqlite3_step(stmt) == SQLITE_ROW){
		ap = new appPackage(sqlite3_column_int(stmt, 0), (char*)sqlite3_column_text (stmt, 1), 
								(char*)sqlite3_column_text(stmt, 2), (char*)sqlite3_column_text(stmt, 3));				
	}

	delete[] selectQuery;
	sqlite3_finalize(stmt);

	return ap;
}
		
void appPackageRepo::selectAllAppPackage(appPackageList& allApList){

	sqlite3_stmt * stmt;
	allApList.clear();
	
	stmt = prepareQuery("SELECT * from APPPACKAGE");
	while(sqlite3_step(stmt) == SQLITE_ROW){
		printf("1.5\n");
		appPackage* tmpAp = new appPackage(sqlite3_column_int(stmt, 0), (char*)sqlite3_column_text (stmt, 1)
										, (char*)sqlite3_column_text(stmt, 2), (char*)sqlite3_column_text(stmt, 3));
				printf("1.6\n");
		allApList.insert(tmpAp);
		delete tmpAp;
				printf("1.7\n");
	}

	sqlite3_finalize(stmt);
			
}


