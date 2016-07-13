#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "serviceDaemonRepo.h"

#include "_repoClass.h"


using namespace std;



	serviceDaemon::serviceDaemon(int _smID, char* _smName, char* _smFilePath, char* _smIpcPath){
			smID = _smID;
		
			smName = new char[strlen(_smName)];
			strcpy(smName, _smName);
			
			smFilePath = new char[strlen(_smFilePath)];
			strcpy(smFilePath, _smFilePath);

			smIpcPath = new char[strlen(_smIpcPath)];
			strcpy(smIpcPath, _smIpcPath);
		}
		
	int serviceDaemon::getSmID() { 	return smID;		}
	char* serviceDaemon::getSmName()  {	return smName;		}
	char* serviceDaemon::getSmFilePath(){	return smFilePath;	}
	char* serviceDaemon::getSmIpcPath(){	return smIpcPath;	}

		
	void serviceDaemon::setSmName(char* _smName){ 	
			delete[] smName;
			smName = new char[strlen(_smName)];
			strcpy(smName, _smName); 		
	}

	void serviceDaemon::setSmFilePath(char* _smFilePath){ 	
			delete[] smFilePath;
			smFilePath = new char[strlen(_smFilePath)];
			strcpy(smFilePath, _smFilePath); 		
	}
		
	void serviceDaemon::setSmIpcPath(char* _smIpcPath){	
			delete[] smIpcPath;
			smIpcPath = new char[strlen(_smIpcPath)];
			strcpy(smIpcPath, _smIpcPath);		
	}

	void serviceDaemon::printSm(){
			cout << "ID > " << smID << "  Name > " << smName << "  Path > " << smFilePath << "  IPCPath > " << smIpcPath << endl;
	}



	serviceDaemonList::serviceDaemonList(){}
	serviceDaemonList::serviceDaemonList(vector<serviceDaemon> _smList){
			smList = _smList;
	}

	void serviceDaemonList::printAllSmList(){

			for(smIter = smList.begin(); smIter != smList.end(); ++smIter ){
				smIter->printSm();
			}	
	}

	void serviceDaemonList::insert(serviceDaemon sm){
			smList.push_back(sm);
	}

	serviceDaemon serviceDaemonList::find(int smID){

			for(smIter = smList.begin(); smIter != smList.end(); ++smIter ){
				if( smID == smIter->getSmID()){
					return *smIter;
				}
			}	
	}

	void serviceDaemonList::erase(serviceDaemon sm){

			for(smIter = smList.begin(); smIter != smList.end(); ++smIter ){
				if( sm.getSmID() == smIter->getSmID()){

					smList.erase(smIter);
				}		
			}	
	}




	serviceDaemonRepo::serviceDaemonRepo() {
			char createTableQuery[] = "CREATE TABLE IF NOT EXISTS SERVICEMANAGER("  \
						   "SMID INTEGER PRIMARY KEY AUTOINCREMENT," \
						   "SMNAME           TEXT    NOT NULL," \
   						   "SMPATH           TEXT    NOT NULL," \
						   "SMIPCPATH         TEXT NOT NULL); ";

			char dbPath[] = "serviceDaemonRepository.db";
			setCreateTableQuery(createTableQuery);
			setDbPath(dbPath);
			openDB();
	}
	
	void serviceDaemonRepo::insertServiceManager(char* smName, char* smPath, char* smIpcPath){
			char* insertQuery;
			insertQuery = new char[strlen(smName) + strlen(smPath) + strlen(smIpcPath) + 128];
			
			sprintf(insertQuery, "INSERT INTO SERVICEMANAGER (SMNAME,SMPATH,SMIPCPATH) VALUES ('%s','%s','%s');", smName, smPath,smIpcPath );
			cout << insertQuery << endl;
			exeQuery(insertQuery);

			delete[] insertQuery;
	}

 	void serviceDaemonRepo::deleteServiceManager(int smID){
			char* deleteQuery;
			deleteQuery = new char[256];

			sprintf(deleteQuery, "DELETE from SERVICEMANAGER where SMID=%d;", smID);
			exeQuery(deleteQuery);

			delete[] deleteQuery;
	}

		
 	void serviceDaemonRepo::updateServiceManager(serviceDaemon* sm){
			char* updateQuery;
			updateQuery = new char[strlen(sm->getSmName())+ strlen(sm->getSmFilePath()) + strlen(sm->getSmIpcPath()) + 256];

			sprintf(updateQuery, "UPDATE SERVICEMANAGER set SMNAME = '%s', SMPATH='%s', SMIPCPATH='%s' where SMID=%d;", 
															sm->getSmName(), sm->getSmFilePath(),sm->getSmIpcPath(), sm->getSmID());
			
			exeQuery(updateQuery);

			delete[] updateQuery;
	}
		
 	serviceDaemon* serviceDaemonRepo::selectServiceManager(int smID){
			char* selectQuery = new char[256];
			sqlite3_stmt * stmt;
			serviceDaemon* sm;
			
			sprintf(selectQuery, "SELECT * from SERVICEMANAGER where SMID=%d", smID);
			stmt = prepareQuery(selectQuery);
			if(sqlite3_step(stmt) == SQLITE_ROW){
				sm = new serviceDaemon(sqlite3_column_int(stmt, 0), (char*)sqlite3_column_text (stmt, 1), 
										(char*)sqlite3_column_text(stmt, 2), (char*)sqlite3_column_text(stmt, 3));				
			}

			delete[] selectQuery;
			sqlite3_finalize(stmt);

			return sm;
	}
		
	void serviceDaemonRepo::selectAllServiceManager(serviceDaemonList& allSmList){
			serviceDaemon* tmpSm;
			sqlite3_stmt * stmt;

			stmt = prepareQuery("SELECT * from SERVICEMANAGER");
			while(sqlite3_step(stmt) == SQLITE_ROW){
				tmpSm = new serviceDaemon(sqlite3_column_int(stmt, 0), (char*)sqlite3_column_text (stmt, 1)
										, (char*)sqlite3_column_text(stmt, 2), (char*)sqlite3_column_text(stmt, 3));
				allSmList.insert(*tmpSm);
			}

			sqlite3_finalize(stmt);
			
	}








