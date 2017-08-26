/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
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

#include "AppList.h"
#include "OPELdbusLog.h"

#define QUERY_BUFFER_LENGTH 2048

AppList* AppList::initializeFromDB(std::string dbPath) {
  AppList* appList = new AppList();

  // Open DB
  bool openDBRes = appList->openDB(dbPath);
  if(!openDBRes) {
    delete appList;
    return NULL;
  }

  // Try to fetch app list from DB
  bool fetchAppListRes = appList->fetchAppList();
  if(fetchAppListRes) {
    // If fetching is successful, return it
    return appList;
  }

  // If fetching is failed, crate a new table
  appList->createDBTable();
  bool createDBTableRes = appList->createDBTable();
  if(!createDBTableRes) {
    delete appList;
    return NULL;
  }

  return appList;
}

bool AppList::openDB(std::string dbPath) {
  // Open DB
  std::string mDBPath = dbPath;
  int openRes = sqlite3_open(this->mDBPath, &db);
  if(openRes != SQLITE_OK) {
    OPEL_DBG_ERR("Fail to open DB! (%s)", dbPath);
    return false;
  }
  return true;
}

bool AppList::fetchAppList() {
  // Fetch all the app's information from DB
  char query[QUERY_BUFFER_LENGTH] = "SELECT * from APPLIST;";
  sqlite3_stmt* stmt;

  char* prepareErr;
  int prepareRes = sqlite3_prepare_v2(this->mDBPath, query, &stmt, prepareErr);
  if(prepareRes != SQLITE_OK) {
    OPEL_DBG_ERR("Failed to fetch app list from DB (%s)", prepareErr);
    sqlite3_free(prepareErr);
    return false;
  }

  while(sqlite3_step(stmt) == SQLITE_ROW) {
    // Get fields
    int appId = sqlite3_column_int(stmt, 0);
    bool isDefaultApp = (sqlite3_column_int(stmt, 1) != 0);
    std::string name(reinterpret_cast<const char*>(
          sqlite3_column_text(stmt, 2)));
    std::string mainJSFileName(reinterpret_cast<const char*>(
          sqlite3_column_text(stmt, 3)));
    std::string iconFileName(reinterpret_cast<const char*>(
          sqlite3_column_text(stmt, 4)));

    // Check this field has already loaded
    App* app = this->get(appId);
    if(app == NULL)
      continue;
    
    // Allocate a new entry and add to on-memory list
    app = new App(appId, isDefaultApp, name, mainJSFileName, iconFileName);
    this->mApps.push_back(app);
  }
  sqlite3_finalize(stmt);

  return true;
}

bool AppList::createDBTable() {
  // Create table
  char query[QUERY_BUFFER_LENGTH] = "CREATE TABLE IF NOT EXISTS APPLIST(" \
                      "ID INTEGER PRIMARY KEY," \
                      "ISDEFAULTAPP INTEGER NOT NULL," \
                      "NAME TEXT NOT NULL," \
                      "MAINJSFILENAME TEXT NOT NULL," \
                      "ICONFILENAME TEXT NOT NULL);";

  char* createErr;
  int createRes = sqlite3_exec(this->mDBPath, query, NULL, NULL, &createErr);
  if(createRes != SQLITE_OK) {
    OPEL_DBG_ERR("Fail to create DB table! (%s)", createErr);
    sqlite3_free(createErr);
    return false;
  }

  OPEL_DBG_VERB("Succeed to create DB table!");
  return true;
}

App* AppList::get(int appId) {
  std::vector<App*>::iterator alIter;
  for(alIter = this->mApps.begin();
      alIter != this->mApps.end();
      ++alIter) {
    int thisAppId = (*alIter)->getId();
    if(thisAppId == appId) {
      return (*alIter);
    }
  }
  return NULL;
}

bool AppList::add(App* app) {
  // Add to on-memory list
  this->mApps.push_back(app);
  return true;
}

bool AppList::flush(App* app) {
  // Get fields to be stored to DB
  int appId = app->getId();
  bool isDefaultApp = app->isDefaultApp();
  std::string name(app->getName());
  std::string mainJSFileName(app->getMainJSFileName());
  std::string iconFileName(app->getIconFileName());

  // Store to DB
  char query[QUERY_BUFFER_LENGTH];
  snprintf(query, QUERY_BUFFER_LENGTH,
      "INSERT INTO APPLIST" \
      "(ID, ISDEFAULTAPP, NAME, MAINJSFILENAME, ICONFILENAME)" \
      "VALUES ('%d', '%s', '%d', '%s', '%s');",
      appId, (isDefaultApp) ? 1 : 0, name, mainJSFileName, iconFileName);

  char* insertErr;
  int insertRes = sqlite3_exec(this->mDBPath, query, NULL, NULL, &insertErr);
  if(insertRes != SQLITE_OK) {
    OPEL_DBG_ERR("Fail to store a new app information to DB (%s)", insertErr);
    sqlite3_free(insertErr);
    return false;
  }
  return true;
}

bool AppList::remove(App* app) {
  boolean isSuccess = false;
  int appId = app->getId();

  // Remove from on-memory list
  std::vector<App*>::iterator alIter;
  for(alIter = this->mApps.begin();
      alIter != this->mApps.end();
      ++alIter) {
    if((*alIter)->getId() == appId) {
      delete (*alIter);
      this->mApps.erase(alIter);
      isSuccess = true;
    }
  }
  if(!isSuccess) {
    OPEL_DBG_ERR("Fail to remove app from app list (%d)", appId);
    return false;
  }

  // Remove from DB
  char query[QUERY_BUFFER_LENGTH];
  snprintf(query, QUERY_BUFFER_LENGTH, "DELETE FROM APPLIST WHERE ID=%d;" appId);
  char* insertErr;
  int insertRes = sqlite3_exec(this->mDBPath, query, NULL, NULL, &insertErr);
  if(insertRes != SQLITE_OK) {
    OPEL_DBG_ERR("Fail to remove app from DB (%d, %s)", appId, insertErr);
    sqlite3_free(insertErr);
    return false;
  }
  return true;
}
