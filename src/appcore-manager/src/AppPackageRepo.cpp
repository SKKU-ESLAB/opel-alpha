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

#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "AppPackageRepo.h"
#include "RepoClass.h"

using namespace std;



AppPackage::AppPackage(int _apID, char* _apName, char* _apFilePath, char* _apExePath){
			apID = _apID;
		
			apName = new char[strlen(_apName)+1];
			strcpy(apName, _apName);
			
			apFilePath = new char[strlen(_apFilePath)+1];
			strcpy(apFilePath, _apFilePath);

			apExePath = new char[strlen(_apExePath)+1];
			strcpy(apExePath, _apExePath);
}

AppPackage::AppPackage(AppPackage* arg){
		apID =arg->apID;
		
		apName = new char[strlen(arg->apName)+1];
		strcpy(apName, arg->apName);
			
		apFilePath = new char[strlen(arg->apFilePath)+1];
		strcpy(apFilePath, arg->apFilePath);

		apExePath = new char[strlen(arg->apExePath)+1];
		strcpy(apExePath, arg->apExePath);
}


AppPackage::~AppPackage(){
			delete[] apName;
			delete[] apFilePath;
			delete[] apExePath;
}

AppPackage& AppPackage::operator=(const AppPackage &arg){
	apID =arg.apID;
	apName = new char[strlen(arg.apName)+1];
	strcpy(apName, arg.apName);
			
	apFilePath = new char[strlen(arg.apFilePath)+1];
	strcpy(apFilePath, arg.apFilePath);

	apExePath = new char[strlen(arg.apExePath)+1];
	strcpy(apExePath, arg.apExePath);
	
}

		
int AppPackage::getApID() { 	return apID;		}
char* AppPackage::getApName()  {	return apName;		}
char* AppPackage::getApFilePath(){	return apFilePath;	}
char* AppPackage::getApExePath(){	return apExePath;	}

		
void AppPackage::setApName(char* _apName){ 	
			delete[] apName;
			apName = new char[strlen(_apName)];
			strcpy(apName, _apName); 		
}

void AppPackage::setApFilePath(char* _apFilePath){ 	
		delete[] apFilePath;
		apFilePath = new char[strlen(_apFilePath)];
		strcpy(apFilePath, _apFilePath); 		
}
		
void AppPackage::setApExePath(char* _apExePath){	
		delete[] apExePath;
		apExePath = new char[strlen(_apExePath)];
		strcpy(apExePath, _apExePath);		
}

void AppPackage::printAp(){
		cout << "ID > " << apID << "  Name > " << apName << "  Path > " << apFilePath << "  EXEPath > " << apExePath << endl;
}






AppPackageList::AppPackageList(){}

vector<AppPackage*>* AppPackageList::getListVector(){

	return &apList;
}

void AppPackageList::insert(AppPackage* ap){		
		AppPackage *tmp = new AppPackage(ap);
		apList.push_back(tmp);
}

AppPackage* AppPackageList::find(int apID){
		for(apIter = apList.begin(); apIter != apList.end(); ++apIter ){
			if( apID == (*apIter)->getApID()){
				return *apIter;
			}
		}	
}
void AppPackageList::erase(AppPackage* ap){
		for(apIter = apList.begin(); apIter != apList.end(); ++apIter ){
			if( ap->getApID() == (*apIter)->getApID()){
				delete *apIter;
				apList.erase(apIter);				
			}		
		}	
}


void AppPackageList::clear(){
	apList.clear();
}








AppPackageRepo::AppPackageRepo() {
		char createTableQuery[] = "CREATE TABLE IF NOT EXISTS APPPACKAGE("  \
					   "APID INTEGER PRIMARY KEY AUTOINCREMENT," \
					   "APNAME           TEXT    NOT NULL," \
					   "APFILEPATH           TEXT    NOT NULL," \
					   "APEXEPATH         TEXT NOT NULL); ";
		char dbPath[] = "AppPackageRepository.db";
	setCreateTableQuery(createTableQuery);
		setDbPath(dbPath);
		openDB();
}

int AppPackageRepo::insertAppPackage(char* apName, char* apFilePath, char* apExePath){
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

void AppPackageRepo::deleteAppPackage(int apID){
		char* deleteQuery;
		deleteQuery = new char[256];
		sprintf(deleteQuery, "DELETE from APPPACKAGE where APID=%d;", apID);
		exeQuery(deleteQuery);
		delete[] deleteQuery;
}

		
void AppPackageRepo::updateAppPackage(AppPackage* ap){
	char* updateQuery;
	updateQuery = new char[strlen(ap->getApName())+ strlen(ap->getApFilePath()) + strlen(ap->getApExePath()) + 256];

	sprintf(updateQuery, "UPDATE APPPACKAGE set APNAME = '%s', APFILEPATH='%s', APEXEPATH='%s' where APID=%d;", 
													ap->getApName(), ap->getApFilePath(),ap->getApExePath(), ap->getApID());
			
	exeQuery(updateQuery);

	delete[] updateQuery;
}
		
AppPackage* AppPackageRepo::selectAppPackage(int apID){
	char* selectQuery = new char[256];
	sqlite3_stmt * stmt;
	AppPackage* ap = NULL;
	
	sprintf(selectQuery, "SELECT * from APPPACKAGE where APID=%d", apID);
	stmt = prepareQuery(selectQuery);
	if(sqlite3_step(stmt) == SQLITE_ROW){
		ap = new AppPackage(sqlite3_column_int(stmt, 0), (char*)sqlite3_column_text (stmt, 1), 
								(char*)sqlite3_column_text(stmt, 2), (char*)sqlite3_column_text(stmt, 3));				
	}

	delete[] selectQuery;
	sqlite3_finalize(stmt);

	return ap;
}
		
void AppPackageRepo::selectAllAppPackage(AppPackageList& allApList){

	sqlite3_stmt * stmt;
	allApList.clear();
	
	stmt = prepareQuery("SELECT * from APPPACKAGE");
	while(sqlite3_step(stmt) == SQLITE_ROW){
		AppPackage* tmpAp = new AppPackage(sqlite3_column_int(stmt, 0), (char*)sqlite3_column_text (stmt, 1)
										, (char*)sqlite3_column_text(stmt, 2), (char*)sqlite3_column_text(stmt, 3));
		allApList.insert(tmpAp);
		delete tmpAp;
	}

	sqlite3_finalize(stmt);
			
}


