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

#include "App.h"

bool App::changeState(AppState::Value newState) {
  // Check new state
  bool isChangeApproved = false;
  switch(this->mState) {
    case AppState::Initializing:
      if(newState == AppState::Initialized)
        isChangeApproved = true;
      break;
    case AppState::Initialized:
      if(newState == AppState::Installing
          || newState == AppState::Removed)
        isChangeApproved = true;
      break;
    case AppState::Installing:
      if(newState == AppState::Ready
          || newState == AppState::Removed)
        isChangeApproved = true;
      break;
    case AppState::Ready:
      if(newState == AppState::Launching
          || newState == AppState::Removing)
        isChangeApproved = true;
      break;
    case AppState::Launching:
      if(newState == AppState::Running
          || newState == AppState::Ready)
        isChangeApproved = true;
      break;
    case AppState::Running:
      if(newState == AppState::Terminating
          || newState == AppState::Ready)
        isChangeApproved = true;
      break;
    case AppState::Terminating:
      if(newState == AppState::Ready)
        isChangeApproved = true;
      break;
    case AppState::Removing:
      if(newState == AppState::Removed)
        isChangeApproved = true;
      break;
    case AppState::Removed:
      break;
  }

  // Apply new state
  if(isChangeApproved) {
    this->mState = newState;
    if(this->mStateListener != NULL) {
      this->mStateListener->onChangedState(this->mState);
    }
  }
  return isChangeApproved;
}

bool App::setFromManifest(std::string manifestFilePath) {
  // TODO: 
	xmlDocPtr doc;
	xmlNodePtr cur;

	char manifestPath[512];
	sprintf(manifestPath, "%s/manifest.xml", appPackageDirPath);

	doc = xmlParseFile( manifestPath );
	if (doc == NULL ) {
		fprintf(stderr,"Document not parsed successfully. \n");
		return NULL;
	}

	cur = xmlDocGetRootElement(doc);
	
	if (cur == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return NULL;
	}

	
	if (xmlStrcmp(cur->name, (const xmlChar *) "application")) {
		fprintf(stderr,"document of the wrong type, root node != story");
		xmlFreeDoc(doc);
		return NULL;
	}

	cur = cur->xmlChildrenNode;

	char appIconFileName[PATH_BUFFER_SIZE]={'/0',};
	char appLabel[PATH_BUFFER_SIZE]={'/0',};
	char appMainFile[PATH_BUFFER_SIZE]={'/0',};
	
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"icon"))){
			//parseStory (doc, cur);
			xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			sprintf(appIconFileName, "%s", key);
			printf("[AppPackageManager] Parse XML >> keyword: %s %s\n", cur->name, appIconFileName);
			xmlFree(key);
		
		
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"label"))){
			//parseStory (doc, cur);
			xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			sprintf(appLabel, "%s", key);
			printf("[AppPackageManager] Parse XML >> keyword: %s %s\n", cur->name, appLabel);
			xmlFree(key);
		
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"mainFile"))){
			//parseStory (doc, cur);
			xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			sprintf(appMainFile, "%s", key);
			printf("[AppPackageManager] Parse XML >> keyword: %s %s\n", cur->name, appMainFile);			
			xmlFree(key);
		}
		
	cur = cur->next;
	}
	xmlFreeDoc(doc);


	// insert app info to repository
	char exeFilePath[128];
	sprintf(exeFilePath, "%s/%s", appPackageDirPath,appMainFile);

	int appID = appPkgRepo.insertAppPackage( appLabel, appPackageDirPath, exeFilePath);

	char appIDStr[128];
	sprintf(appIDStr, "%d", appID);


	//send IconFile with appId, appName, 


	JsonString jp;
	jp.addType(INSTALLPKG);
	jp.addItem("appID",appIDStr);
	jp.addItem("appName",appLabel);
	jp.addItem("appPath", appPackageDirPath);
	jp.addItem("appIconName", appIconFileName);

}

void App::finishInitializing(int appId) { // Initializing -> Initialized
  this->mAppId = appId;
  this->changeState(AppState::Initialized);
}

void App::startInstalling(std::string packageFilePath) { // Initialized -> Installing
  this->mPackageFilePath = packageFilePath;
  this->changeState(AppState::Installing);
}

void App::successInstalling(bool isDefaultApp,
    std::string name, std::string mainJSFileName) { // Installing -> Ready
  this->mIsDefaultApp = isDefaultApp;
  this->mName = name;
  this->mMainJSFileName = mainJSFileName;

  this->changeState(AppState::Ready);
}

void App::failInstalling() { // Installing -> Removed
  this->changeState(AppState::Removed);
}

void App::startLaunching(int pid) { // Ready -> Launching
  this->mPid = pid;
  this->changeState(AppState::Initialized);
}

void App::successLaunching() { // Launching -> Running
  this->changeState(AppState::Running);
}

void App::failLaunching() { // Launching -> Ready
  this->changeState(AppState::Ready);
}

void App::startTerminating() { // Running -> Terminating
  this->changeState(AppState::Terminating);
}

void App::finishTerminating() { // Terminating -> Ready
  this->changeState(AppState::Ready);
}

void App::startRemoving() { // Ready -> Removing
  this->changeState(AppState::Removing);
}

void App::finishRemoving() { // Removing -> Removed
  this->changeState(AppState::Removed);
}
