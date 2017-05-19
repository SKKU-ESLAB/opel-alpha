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


#include "RepoClass.h"


using namespace std;


RepoClass::RepoClass(){  }
RepoClass::RepoClass(char* _dbPath, char* _createTableQuery){
		strcpy(dbPath, _dbPath);
		strcpy(createTableQuery, _createTableQuery);
}

void RepoClass::openDB(){

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


void RepoClass::createTable(){
		char* zErrMsg;
		int res = sqlite3_exec(db, createTableQuery, callback, 0, &zErrMsg);
		if( res != SQLITE_OK ){
			cout << "Fail to create table::" << createTableQuery << endl;
		}else{
			cout << "Table created successfully" << endl;
		}
}


void RepoClass::setCreateTableQuery(char* _createTableQuery){
		createTableQuery = new char [strlen(_createTableQuery)];
		strcpy(createTableQuery, _createTableQuery);
}

void RepoClass::setDbPath(char* _dbPath){
		dbPath = new char[strlen(_dbPath)];
		strcpy(dbPath, _dbPath);
}




void RepoClass::exeQuery(char* query){
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

sqlite3_stmt* RepoClass::prepareQuery(char* query){

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



RepoClass::~RepoClass(){
		sqlite3_close(db);
}





