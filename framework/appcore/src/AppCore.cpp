/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
 *              Dongig Sin<dongig@skku.edu>
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

#include <iostream>
#include <vector>
#include <dirent.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "AppCore.h"
#include "OPELdbugLog.h"
#include "BaseMessage.h"
#include "AppList.h"

using namespace std;

bool AppCore::initializeDirs() {
  // Initialize user app's, system app's directory path
  struct stat st = {0};
  char* dirString;

  // Apps Dir
  dirString = getenv("OPEL_APPS_DIR");
  if(dirString != NULL) {
    // User Apps Dir
    snprintf(this->mUserAppsDir, PATH_BUFFER_SIZE, "%s/%s",
        dirString, "user");
    if(stat(this->mUserAppsDir, &st) == -1) {
      mkdir(this->mUserAppsDir, 0755);
    }

    // System Apps Dir
    snprintf(this->mSystemAppsDir, PATH_BUFFER_SIZE, "%s/%s",
        dirString, "system");
    if(stat(this->mSystemAppsDir, &st) == -1) {
      mkdir(this->mSystemAppsDir, 0755);
    }

    // App List DB Dir
    snprintf(this->mAppListDBDir, PATH_BUFFER_SIZE, "%s/%s",
        dirString, "AppListDB.sqlite");
  } else {
    OPEL_DBG_ERR("Cannot read OPEL_APPS_DIR");
    return false;
  }

  // Data Dir
  dirString = getenv("OPEL_DATA_DIR");
  if(dirString != NULL) {
    snprintf(this->mDataDir, PATH_BUFFER_SIZE, "%s", dirString);
    if(stat(this->mDataDir, &st) == -1) {
      mkdir(this->mDataDir, 0755);
    }
  } else {
    OPEL_DBG_ERR("Cannot read OPEL_DATA_DIR");
    return false;
  }

  // Temp Dir
  dirString = getenv("OPEL_TEMP_DIR");
  if(dirString != NULL) {
    snprintf(this->mTempDir, PATH_BUFFER_SIZE, "%s", dirString);
    if(stat(this->mTempDir, &st) == -1) {
      mkdir(this->mTempDir, 0755);
    }
  } else {
    // TODO: For now, we use ${OPEL_DATA_DIR}/temp as ${OPEL_TEMP_DIR} as now.
    // In the future, we should make ${OPEL_TEMP_DIR} a compulsory value.
    dirString = getenv("OPEL_DATA_DIR");
    if(dirString != NULL) {
      snprintf(this->mTempDir, PATH_BUFFER_SIZE, "%s/temp", dirString);
      OPEL_DBG_WARN("Cannot read OPEL_TEMP_DIR, so OPEL_TEMP_DIR is set as %s",
          this->mTempDir);
      if(stat(this->mTempDir, &st) == -1) {
        mkdir(this->mTempDir, 0755);
      }
    } else {
      OPEL_DBG_ERR("Cannot read OPEL_DATA_DIR, %s",
          "so we cannot determine OPEL_TEMP_DIR");
      return false;
    }
  }
  return true;
}

#define COMPANION_DEVICE_URI "/comp0"
#define APPS_URI "/thing/apps"
#define APPCORE_URI "/thing/appcore"

// Main loop
void AppCore::run() {
  // Initialize directories
  if(this->initializeDirs() == false) {
    OPEL_DBG_ERR("Cannot find at least one OPEL environment variable.");
    return;
  }

  // Initialize AppList
  this->initializeFromDB(this->mAppListDBPath);

  // Initialize MessageRouter and Channels
  this->mMessageRouter = new MessageRouter();
  this->mDbusChannel = new DbusChannel(this->mMessageRouter);
  this->mCommChannel = new CommChannel(this->mMessageRouter,
      this->getTempDir());
  this->mLocalChannel = new LocalChannel(this->mMessageRouter);

  // Run DbusChannel and add it to MessageRouter's routing table
  this->mDbusChannel->run();
  this->mMessageRouter->addRoutingEntry(APPS_URI, this->mDbusChannel);

  // Run CommChannel
  // CommChannel will be added to routing table when a remote device is
  // connected
  this->mCommChannel->setStateListener(this);
  this->mCommChannel->run();

  // LocalChannel: run on main thread
  // Main loop starts to run in LocalChannel::run()
  this->mLocalChannel->setListener(this);
  this->mLocalChannel->run();
  this->mMessageRouter->addRoutingEntry(APPCORE_URI, this->mLocalChannel);
}

// Signal handler
void AppCore::onSignalSIGCHLD() {
  int status;
  int pid;

  pid = wait(&status);
  if (pid > 0) {
    // Handle 3rd-party App Termination Event on the main thread
    this->completeTerminatingApp(pid);
  }
}

// Appcore Commands
void AppCore::onReceivedMessage(BaseMessage* message) {
  if(message == NULL) {
    OPEL_DBG_ERR("Invalid AppCore Message");
    return;
  }
  if(message->getType() == BaseMessageType::AppCore) {
    OPEL_DBG_ERR("Not AppCore Message");
    return;
  }
  AppCoreMessage* payload = (AppCoreMessage*)message->getPayload();
  if(payload == NULL) {
    OPEL_DBG_ERR("AppCoreMessage payload does not exist");
    return;
  }

  switch(payload->getCommandType()) {
    case AppCoreMessageCommandType::GetAppList:
      this->getAppList(baseMessage);
      break;
    case AppCoreMessageCommandType::ListenAppState:
      this->listenAppState(baseMessage);
      break;
    case AppCoreMessageCommandType::InitializeApp:
      this->initializeApp(baseMessage);
      break;
    case AppCoreMessageCommandType::InstallApp:
      this->installApp(baseMessage);
      break;
    case AppCoreMessageCommandType::LaunchApp:
      this->launchApp(baseMessage);
      break;
    case AppCoreMessageCommandType::CompleteLaunchingApp:
      this->completeLaunchingApp(baseMessage);
      break;
    case AppCoreMessageCommandType::TerminateApp:
      this->terminateApp(baseMessage);
      break;
    case AppCoreMessageCommandType::RemoveApp:
      this->removeApp(baseMessage);
      break;
    case AppCoreMessageCommandType::GetFileList:
      this->getFileList(baseMessage);
      break;
    case AppCoreMessageCommandType::GetFile:
      this->getFile(baseMessage);
      break;
    case AppCoreMessageCommandType::GetRootPath:
      this->getRootPath(baseMessage);
      break;
  }
}

// It supports only single companion device
void AppCore::onCommChannelStateChanged(CommChannelState::Value state) {
  switch(state) {
    case CommChannelState::IDLE:
      // Remove routing entry for CommChannel
      this->mMessageRouter->removeRoutingEntry(COMPANION_DEVICE_URI);
      break;
    case CommChannelState::LISTENING_DEFAULT:
      // ignore
      this->mMessageRouter->addRoutingEntry(COMPANION_DEVICE_URI,
          this->mCommChannel);
      break;
    case CommChannelState::CONNECTED_DEFAULT:
      // Add routing entry for CommChannel
      break;
    case CommChannelState::LISTENING_LARGEDATA:
    case CommChannelState::CONNECTED_LARGEDATA:
      // ignore
      break;
  }
}

void AppCore::onChangedState(int appId, AppState::Value newState) {
  // Check if the app is one of which state is changed
  std::vector<BaseMessage*>::iterator iter;
  for(iter = this->mListenAppStateMessageList.begin();
      iter != this->mListenAppStateMessageList.end();
      iter++) {
    BaseMessage* originalMessage = (*iter);
    AppCoreMessage* originalPayload
      = (AppCoreMessage*)originalMessage->getPayload();
    int thisAppId = -1;
    requestPayload->getParamsListenAppState(thisAppId);
    if(thisAppId == appId) {
      // Make ACK message
      BaseMessage* ackMessage
        = MessageFactory::makeAppCoreAckMessage(COMPANION_DEVICE_URI,
            originalMessage); 
      AppCoreAckMessage* ackPayload = ackMessage->getPayload();
      ackPayload->setParamsListenAppState(appId, newState);

      // Send ACK message
      this->mLocalChannel->sendMessage(ackMessage);
      return;
    }
  }
}

void AppCore::getAppList(BaseMessage* message) {
  // No arguments
  int messageId = message->getMessageId();

  // Make AppList parameter
  ParamAppList* paramAppList = ParamAppList::make();
  std::vector<App*>& appList
  std::vector<App*>::iterator iter;
  for(iter = appList.begin();
      iter != appList.end();
      iter++) {
    int appId = (*iter)->getAppId();
    std::string appName = (*iter)->getAppName();
    bool isDefaultApp = (*iter)->isDefaultApp();
    paramAppList->addEntry(appId, appName, isDefaultApp);
  }

  // Make ACK message
  BaseMessage* ackMessage
    = MessageFactory::makeAppCoreAckMessage(COMPANION_DEVICE_URI, message); 
  AppCoreAckMessage* ackPayload = ackMessage->getPayload();
  ackPayload->setParamsGetAppList(paramAppList);

  // Send ACK message
  this->mLocalChannel->sendMessage(ackMessage);
  delete paramAppList;
}

void AppCore::listenAppState(BaseMessage* message) {
  // Get arguments
  BaseMessage* originalMessage = (*iter);
  AppCoreMessage* originalPayload
    = (AppCoreMessage*)originalMessage->getPayload();
  int appId = -1;
  requestPayload->getParamsListenAppState(appId);

  // Check if there has already been listener of the app
  std::vector<BaseMessage*>::iterator iter;
  for(iter = this->mListenAppStateMessageList.begin();
      iter != this->mListenAppStateMessageList.end();
      iter++) {
    BaseMessage* originalMessage = (*iter);
    AppCoreMessage* originalPayload =
      (AppCoreMessage*)originalMessage->getPayload();
    int thisAppId = -1;
    requestPayload->getParamsListenAppState(thisAppId);
    if(thisAppId == appId) {
      return;
    }
  }
  
  // If there is no listener, add it to the list
  this->mListenAppStateMessageList.push_back(message);
}

void AppCore::initializeApp(BaseMessage* message) {
  // No arguments
  
  // Add to on-memory app list
  App* app = new App();
  this->mAppList->add(app);

  // Update state
  app->finishInitializing(this->mNextAppId++);

  // Make ACK message
  BaseMessage* ackMessage
    = MessageFactory::makeAppCoreAckMessage(COMPANION_DEVICE_URI, message); 
  AppCoreAckMessage* ackPayload = ackMessage->getPayload();
  ackPayload->setParamsInitializeApp(app->getId());

  // Send ACK message
  this->mLocalChannel->sendMessage(ackMessage);
  delete paramAppList;
}

void AppCore::installApp(BaseMessage* message) {
  // Get arguments
  std::string packageFilePath(message->getStoredFilePath());

  int appId;
  std::string packageFileName;
  message->getParamsInstallApp(appId, packageFileName);

  // Find app for the appId
  App* app = this->mAppList->getByAppId(appId);
  if(app == NULL) {
    OPEL_DBG_ERR("App does not exist in the app list!");
    return;
  }

  // Update state
  app->startInstalling();

  // Make app package directory
  // truncate opk extension
	char appPackageDirName[PATH_BUFFER_SIZE];
	strncpy(appPackageDirName, packageFileName, strlen(packageFileName)-4); 

  // ${OPEL_APPS_DIR}/user/${APP_NAME}
	char appPackageDirPath[PATH_BUFFER_SIZE];
	snprintf(appPackageDirPath, PATH_BUFFER_SIZE, "%s/%s",
      this->mUserAppsDir, appPackageDirName); 

	struct stat st = {0};
	if(stat(appPackageDirPath, &st) == -1) {
    mkdir(appPackageDirPath, 0755);
	} else {
    OPEL_DBG_ERR("There has already been app package directory!");
    app->failInstalling();
  }

  // Archive app package
	snprintf(packageFilePath, PATH_BUFFER_SIZE, "%s/%s",
      this->mUserAppsDir, packageFileName.c_str());
	char* commandUnzip[4] ={0,};
	commandUnzip[0] = "";
	commandUnzip[1] = packageFilePath;
	commandUnzip[2] = "-d";
	commandUnzip[3] = appPackageDirPath;

	do_unzip(4, commandUnzip);
	sync();

  // Remove app package file
	if (remove(packageFilePath) == -1){
		OPEL_DBG_WARN("Cannot remove app package file: %s\n", packageFilePath);
    app->failInstalling();
	}
	
  // Parse app manifest file
  char manifestFilePath[PATH_BUFFER_SIZE];
  snprintf(manifestFilePath, "%s/%s", appPackageDirPath, "manifest.xml");
  bool settingSuccess = app->setFromManifest(manifestFilePath);
  if(!settingSuccess) {
    app->failInstalling();
    return;
  }

  // Flush the app information to database
  this->mAppList->flush(app);

  // Update state
  app->successInstalling(appPackageDirPath);
  return; 
}

void AppCore::removeApp(BaseMessage* message) {
  // Get arguments
  int appId;
  if(message->getParamsTerminateApp(appId) == false) {
    OPEL_DBG_ERR("Invalid AppCoreMessage! (commandType: %d)",
        message->getCommandType());
    return;
  }

  // Find app for the appId
  App* app = this->mAppList->getAppId(appId);
  if(app == NULL) {
    OPEL_DBG_ERR("App does not exist in the app list!");
    return;
  }

  // Update state
  app->startRemoving();

	char commandRm[PATH_BUFFER_SIZE];
	snprintf(commandRm, PATH_BUFFER_SIZE,
      "rm -rf %s", app->getPackagePath());

  // Update state
  app->finishRemoving();

  // Remove from app list
  this->mAppList->remove(app);
	return;
}

void AppCore::launchApp(BaseMessage* message) {
  // Get arguments
  int appId;
  if(message->getParamsLaunchApp(appId) == false) {
    OPEL_DBG_ERR("Invalid AppCoreMessage! (commandType: %d)",
        message->getCommandType());
    return;
  }

  // Find app for the appId
  App* app = this->mAppList->getAppId(appId);
  if(app == NULL) {
    OPEL_DBG_ERR("App does not exist in the app list!");
    return;
  }
  
  pid_t pid;
  pid = fork();

  if (pid > 0) {
    // parent for managing child's PID & mainstream
    // Update state
    app->startLaunching(pid);
  } else if(pid == 0) {
    // Child for executing the application
    char mainJSFilePath[PATH_BUFFER_SIZE];
    snprintf(mainJSFilePath, PATH_BUFFER_SIZE, "%s/%s",
        app->getPackagePath(), app->getMainJSFileName());
    char* fullPath[] = {"node", mainJSFilePath, NULL};	
    OPEL_DBG_VERB("Launch app: %s", mainJSFilePath);
    execvp("node", fullPath);
  } else {
    OPEL_DBG_ERR("Fail to fork app process\n");
  }
}

void AppCore::completeLaunchingApp(BaseMessage* message) {
  // Get arguments
  int appId;
  int pid;
  if(message->getParamsCompleteLaunchingApp(appId, pid) == false) {
    OPEL_DBG_ERR("Invalid AppCoreMessage! (commandType: %d)",
        message->getCommandType());
    return;
  }

  // Find app for the appId
  App* app = this->mAppList->getByAppId(appId);
  if(app == NULL) {
    OPEL_DBG_ERR("App does not exist in the app list!");
    return;
  }

  // Update state
  app->successLaunching();
}

void AppCore::terminateApp(BaseMessage* message) {
  // Get arguments
  int appId;
  if(message->getParamsTerminateApp(appId) == false) {
    OPEL_DBG_ERR("Invalid AppCoreMessage! (commandType: %d)",
        message->getCommandType());
    return;
  }

  // Update state
  app->startTerminating();

  // Make terminate message
  char uri[PATH_BUFFER_SIZE];
  snprintf(uri, PATH_BUFFER_SIZE, "%s/%s", APPS_URI, app->getAppId());
  BaseMessage* appMessage
    = MessageFactory::makeAppMessage(APPS_URI, AppMessageType::Terminate);

  // Send the terminate message
  this->mLocalChannel->sendMessage(appMessage);
}

void AppCore::completeTerminatingApp(int pid) {
  // Find app for the pid
  App* app = this->mAppList->getByPid(pid);
  if(app == NULL) {
    OPEL_DBG_ERR("App does not exist in the app list!");
    return;
  }

  OPEL_DBG_VERB("Child was killed [User app pid : %d]", pid);

  // Update state
  app->finishTerminating();
}

void AppCore::getFileList(BaseMessage* message) {
  // Get arguments
  std::string path;
  if(message->getParamsGetFileList(path) == false) {
    OPEL_DBG_ERR("Invalid AppCoreMessage! (commandType: %d)",
        message->getCommandType());
    return;
  }

  // Check if the directory is available
	DIR *dir;
	if((dir = opendir(path)) == NULL) {
		OPEL_LOG_ERR("cannot open the directory: %s\n", path.c_str());
		return 0;
	}

  // Add file entries to the file list to be returned
  ParamFileList* paramFileList = ParamFileList::make();
	struct dirent *dirEntry;
	struct stat st;
	
	while((dirEntry = readdir(dir)) != NULL){
    // Get the file entry's stat
		lstat(dirEntry->d_name, &st);

    std::string fileName(dirEntry->d_name);
    int fileType = ((S_ISDIR(st.st_mode)) ? 1 :
        ((S_ISREG(st.st_mode)) ? 2 : 0));
    int fileSizeBytes = st.st_size;

    char fileTimeBuffer[30];
    strftime(fileTimeBuffer, "%Y-%m-%d %H:%M:%S", st.st_atime);
    std::string fileTime(fileTimeBuffer);

    paramFileList->addEntry(fileName, fileType, fileSizeBytes, fileTime);
	}

  // Send ACK message
  this->mLocalChannel->sendMessage(ackMessage);
  delete paramFileList;
}

void AppCore::getFile(BaseMessage* message) {
  // Get arguments
  std::string path;
  if(message->getParamsGetFile(path) == false) {
    OPEL_DBG_ERR("Invalid AppCoreMessage! (commandType: %d)",
        message->getCommandType());
    return;
  }

  // Make ACK message
  BaseMessage* ackMessage
    = MessageFactory::makeAppCoreAckMessage(COMPANION_DEVICE_URI, message); 
  ackMessage->attachFile(path);

  // Send ACK message
  this->mLocalChannel->sendMessage(ackMessage);
}

void AppCore::getRootPath(BaseMessage* message) {
  // No arguments

  // Make ACK message
  BaseMessage* ackMessage
    = MessageFactory::makeAppCoreAckMessage(COMPANION_DEVICE_URI, message); 
  AppCoreAckMessage* ackPayload = ackMessage->getPayload();
  ackPayload->setParamsGetRootPath(this->mDataDir);

  // Send ACK message
  this->mLocalChannel->sendMessage(ackMessage);
}
