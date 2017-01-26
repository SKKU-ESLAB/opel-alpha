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

#ifndef _REPO
#define _REPO

#include <sqlite3.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using namespace std;

class repoClass{
	private:

	protected:
		sqlite3* db;
		char* createTableQuery;
		char* dbPath;

	public:
		repoClass();
		repoClass(char* _dbPath, char* _createTableQuery);

		void openDB();

		void createTable();


		void setCreateTableQuery(char* _createTableQuery);

		void setDbPath(char* _dbPath);
		void exeQuery(char* query);

		sqlite3_stmt* prepareQuery(char* query);
	

		static int callback(void *NotUsed, int argc, char **argv, char **azColName){
			int i;
					for(i=0; i<argc; i++){
						cout << azColName[i] << argv[i] << endl;
					}
					cout << endl;
					return 0;

		}

		~repoClass();
};

#endif
