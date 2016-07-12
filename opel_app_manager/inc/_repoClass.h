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