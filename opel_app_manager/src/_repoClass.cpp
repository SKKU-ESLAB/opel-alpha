#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>


#include "_repoClass.h"


using namespace std;


repoClass::repoClass(){  }
repoClass::repoClass(char* _dbPath, char* _createTableQuery){
		strcpy(dbPath, _dbPath);
		strcpy(createTableQuery, _createTableQuery);
}

void repoClass::openDB(){

		int res;

		//if( access(dbPath, R_OK & W_OK) )
		
		res = sqlite3_open(dbPath, &db);
		if(res){
			cout << "Can't open DB" << endl;
		}
		else{
			cout << "Open DB::>>" << dbPath << endl;
			createTable();
		}
	
}


void repoClass::createTable(){
		char* zErrMsg;
		int res = sqlite3_exec(db, createTableQuery, callback, 0, &zErrMsg);
		if( res != SQLITE_OK ){
			cout << "Fail to create table::" << createTableQuery << endl;
		}else{
			cout << "Table created successfully" << endl;
		}
}


void repoClass::setCreateTableQuery(char* _createTableQuery){
		createTableQuery = new char [strlen(_createTableQuery)];
		strcpy(createTableQuery, _createTableQuery);
}

void repoClass::setDbPath(char* _dbPath){
		dbPath = new char[strlen(_dbPath)];
		strcpy(dbPath, _dbPath);
}




void repoClass::exeQuery(char* query){
		char *zErrMsg = 0;
		int res = sqlite3_exec(db, query, callback, 0, &zErrMsg);
		if( res != SQLITE_OK ){
			cout << "SQL error"<< zErrMsg <<  endl; 			
			sqlite3_free(zErrMsg);

		}

		else{
			cout << "["<< query <<"]" << "success" << endl;
		}
	}

sqlite3_stmt* repoClass::prepareQuery(char* query){

		sqlite3_stmt *stmt;
		
		int res = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
		if( res != SQLITE_OK ){
			cout << "SQL error"<<  endl;			

		}

		else{

			cout << "["<< query <<"]" << "success" << endl;

			return stmt;
		}

		return 0;
}



repoClass::~repoClass(){
		sqlite3_close(db);
}





