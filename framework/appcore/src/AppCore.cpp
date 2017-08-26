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
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>

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
    sprintf(this->mUserAppsDir, "%s%s", dirString, "/user/");
    if(stat(this->mUserAppsDir, &st) == -1) {
      mkdir(this->mUserAppsDir, 0755);
    }

    // System Apps Dir
    sprintf(this->mSystemAppsDir, "%s%s", dirString, "/system/");
    if(stat(this->mSystemAppsDir, &st) == -1) {
      mkdir(this->mSystemAppsDir, 0755);
    }
  } else {
    OPEL_DBG_ERR("Cannot read OPEL_APPS_DIR");
    return false;
  }

  // Data Dir
  dirString = getenv("OPEL_DATA_DIR");
  if(dirString != NULL) {
    sprintf(this->mDataDir, "%s", dirString);
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
    sprintf(this->mTempDir, "%s", dirString);
    if(stat(this->mTempDir, &st) == -1) {
      mkdir(this->mTempDir, 0755);
    }
  } else {
    // TODO: For now, we use ${OPEL_DATA_DIR}/temp as ${OPEL_TEMP_DIR} as now.
    // In the future, we should make ${OPEL_TEMP_DIR} a compulsory value.
    dirString = getenv("OPEL_DATA_DIR");
    if(dirString != NULL) {
      sprintf(this->mTempDir, "%s/temp", dirString);
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

// Main loop
void AppCore::run() {
  // Initialize directories
  if(this->initializeDirs() == false) {
    OPEL_DBG_ERR("Cannot find at least one OPEL environment variable.");
    return;
  }

  // Initialize MessageRouter and Channels
  this->mMessageRouter = new MessageRouter();
  this->mDbusChannel = new DbusChannel(this->mMessageRouter);
  this->mCommChannel = new CommChannel(this->mMessageRouter,
      this->getTempDir());
  this->mLocalChannel = new LocalChannel(this->mMessageRouter);

  // Run DbusChannel and add it to MessageRouter's routing table
  this->mDbusChannel->run();
  this->mMessageRouter->addRoutingEntry("/thing/apps", this->mDbusChannel);

  // Run CommChannel
  // CommChannel will be added to routing table when a remote device is
  // connected
  this->mCommChannel->setStateListener(this);
  this->mCommChannel->run();

  // LocalChannel: run on main thread
  // Main loop starts to run in LocalChannel::run()
  this->mLocalChannel->setListener(this);
  this->mLocalChannel->run();
  this->mMessageRouter->addRoutingEntry("/thing/appcore", this->mLocalChannel);
}

// Signal handler
void AppCore::onSignalSIGCHLD() {
  int status;
  int pid;

  pid = wait(&status);

  if (pid > 0) {
    // Handle 3rd-party App Termination Event on the main thread
    OPEL_DBG_VERB("Child was killed [User app pid : %d]", pid);
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
#define COMPANION_DEVICE_URI "/comp0"
void AppCore::onCommChannelStateChanged(CommChannelState::Value state) {
  switch(state) {
    case CommChannelState::IDLE:
      // Remove routing entry for CommChannel
      this->mMessageRouter->removeRoutingEntry(COMPANION_DEVICE_URI);
      break;
    case CommChannelState::LISTENING_DEFAULT:
      // ignore
      this->mMessageRouter->addRoutingEntry(COMPANION_DEVICE_URI, this->mCommChannel);
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
    AppCoreMessage* originalPayload = (AppCoreMessage*)originalMessage->getPayload();
    int thisAppId = -1;
    requestPayload->getParamsListenAppState(thisAppId);
    if(thisAppId == appId) {
      // Make ACK message
      BaseMessage* ackMessage
        = MessageFactory::makeAppCoreAckMessage(COMPANION_DEVICE_URI, originalMessage); 
      AppCoreAckMessage* ackPayload = ackMessage->getPayload();
      ackPayload->setParamsListenAppState(appId, newState);

      // Send ACK message
      this->mLocalChannel->sendMessage(message);
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
  this->mLocalChannel->sendMessage(message);
  delete paramAppList;
}

void AppCore::listenAppState(BaseMessage* message) {
  // Get arguments
  BaseMessage* originalMessage = (*iter);
  AppCoreMessage* originalPayload = (AppCoreMessage*)originalMessage->getPayload();
  int appId = -1;
  requestPayload->getParamsListenAppState(appId);

  // Check if there has already been listener of the app
  std::vector<BaseMessage*>::iterator iter;
  for(iter = this->mListenAppStateMessageList.begin();
      iter != this->mListenAppStateMessageList.end();
      iter++) {
    BaseMessage* originalMessage = (*iter);
    AppCoreMessage* originalPayload = (AppCoreMessage*)originalMessage->getPayload();
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
  //
  // TODO: not yet implemented
}

void AppCore::installApp(BaseMessage* message) {
  // Get arguments
  int appId;
  std::string packageFileName;
  if(message->getParamsInstallApp(appId, packageFileName) == false) {
    OPEL_DBG_ERR("Invalid AppCoreMessage! (commandType: %d)",
        message->getCommandType());
    return;
  }
  // TODO: not yet implemented
}

void AppCore::launchApp(BaseMessage* message) {
  // Get arguments
  int appId;
  if(message->getParamsLaunchApp(appId) == false) {
    OPEL_DBG_ERR("Invalid AppCoreMessage! (commandType: %d)",
        message->getCommandType());
    return;
  }
  // TODO: not yet implemented
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
  // TODO: not yet implemented
}

void AppCore::terminateApp(BaseMessage* message) {
  // Get arguments
  int appId;
  if(message->getParamsTerminateApp(appId) == false) {
    OPEL_DBG_ERR("Invalid AppCoreMessage! (commandType: %d)",
        message->getCommandType());
    return;
  }
  // TODO: not yet implemented
}

void AppCore::removeApp(BaseMessage* message) {
  // Get arguments
  int appId;
  if(message->getParamsTerminateApp(appId) == false) {
    OPEL_DBG_ERR("Invalid AppCoreMessage! (commandType: %d)",
        message->getCommandType());
    return;
  }
  // TODO: not yet implemented
}

void AppCore::getFileList(BaseMessage* message) {
  // Get arguments
  std::string path;
  if(message->getParamsGetFileList(path) == false) {
    OPEL_DBG_ERR("Invalid AppCoreMessage! (commandType: %d)",
        message->getCommandType());
    return;
  }
  // TODO: not yet implemented
}

void AppCore::getFile(BaseMessage* message) {
  // Get arguments
  std::string path;
  if(message->getParamsGetFile(path) == false) {
    OPEL_DBG_ERR("Invalid AppCoreMessage! (commandType: %d)",
        message->getCommandType());
    return;
  }
  // TODO: not yet implemented
}

void AppCore::getRootPath(BaseMessage* message) {
  // No arguments
  // TODO: update arguments
  // TODO: not yet implemented
}
