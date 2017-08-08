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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include "BaseMessage.h"
#include "cJSON.h"
#include "OPELdbugLog.h"

#define RETURN_IF_INVALID_CJSON_OBJ(a, ret) \
  if((a) == NULL) { \
    OPEL_DBG_ERR("JSON parsing error before: %s", cJSON_GetErrorPtr()); \
    return ret; \
  }

#define OPEL_MESSAGE_KEY_URI "uri"
#define OPEL_MESSAGE_KEY_TYPE "type"
#define OPEL_MESSAGE_KEY_PAYLOAD "payload"
#define OPEL_MESSAGE_KEY_IS_FILE_ATTACHED "isFileAttached"
#define OPEL_MESSAGE_KEY_FILE_NAME "fileName"

#define EXT4_FILE_PATH_LENGTH 4097

BaseMessage* BaseMessage::parse(const char* rawString) {
  // Parse rawString in JSON
  cJSON* rootObj = cJSON_Parse(rawString);
  RETURN_IF_INVALID_CJSON_OBJ(rootObj, NULL);

  // Get properties
  // URI
  cJSON* uriObj = cJSON_GetObjectItem(rootObj, OPEL_MESSAGE_KEY_URI);
  RETURN_IF_INVALID_CJSON_OBJ(uriObj, NULL);
  std::string uri(uriObj->valuestring);

  // Type
  cJSON* typeObj = cJSON_GetObjectItem(rootObj, OPEL_MESSAGE_KEY_TYPE);
  RETURN_IF_INVALID_CJSON_OBJ(typeObj, NULL);
  // BE AWARE: Since cJSON internally consider integer value as Number(double),
  // I decided to use string-shaped integer in JSON.
  // Once integer value is carried by Number variable, its value can be changed
  // because there is a limit in mantissa's representation.
  BaseMessageType::Value type
    = static_cast<BaseMessageType::Value>(atoi(typeObj->valuestring));

  // isFileAttached
  cJSON* isFileAttachedObj = cJSON_GetObjectItem(rootObj,
      OPEL_MESSAGE_KEY_IS_FILE_ATTACHED);
  RETURN_IF_INVALID_CJSON_OBJ(isFileAttachedObj, NULL);
  int isFileAttachedNum = atoi(isFileAttachedObj->valuestring);
  bool isFileAttached = (isFileAttachedNum == 0) ? false : true;

  // FileName
  std::string fileName("");
  if(isFileAttachedNum) {
    cJSON* fileNameObj = cJSON_GetObjectItem(rootObj,
        OPEL_MESSAGE_KEY_FILE_NAME);
    RETURN_IF_INVALID_CJSON_OBJ(fileNameObj, NULL);
    fileName = fileNameObj->valuestring;
  }

  cJSON* payloadObj = cJSON_GetObjectItem(rootObj, OPEL_MESSAGE_KEY_PAYLOAD);
  RETURN_IF_INVALID_CJSON_OBJ(payloadObj, NULL);

  // Allocate and initialize a new BaseMessage
  BaseMessage* newMessage = NULL;
  switch(type) {
    case BaseMessageType::AppCore:
      newMessage = AppCoreMessage::jsonToMessage(payloadObj, uri, type,
          isFileAttached, fileName);
      break;
    case BaseMessageType::App:
      newMessage = AppMessage::jsonToMessage(payloadObj, uri, type,
          isFileAttached, fileName);
      break;
    default:
      // These types cannot be handled.
      newMessage = new BaseMessage(uri, type, isFileAttached, fileName);
      break;
  }
  return newMessage;
}

// Used to attach file on message to be sent
BaseMessage* BaseMessage::attachFile(std::string filePath) {
  char filePathBuffer[EXT4_FILE_PATH_LENGTH];
  snprintf(filePathBuffer, EXT4_FILE_PATH_LENGTH, "%s", filePath.c_str());
  std::string fileName(basename(filePathBuffer));

  this->mIsFileAttached = true;
  this->mFileName = fileName;
  return setStoredFilePath(filePath);
}

cJSON* BaseMessage::toJson() {
  cJSON* rootObj = cJSON_CreateObject();
  char tempStr[20];

  // URI
  cJSON_AddStringToObject(rootObj, OPEL_MESSAGE_KEY_URI, this->mUri.c_str());

  // Type
  sprintf(tempStr, "%d", this->mType);
  cJSON_AddStringToObject(rootObj, OPEL_MESSAGE_KEY_TYPE, tempStr);

  // isFileAttached
  sprintf(tempStr, "%d", this->mIsFileAttached);
  cJSON_AddStringToObject(rootObj, OPEL_MESSAGE_KEY_IS_FILE_ATTACHED, tempStr);

  if(this->mIsFileAttached) {
    // FileName
    cJSON_AddStringToObject(rootObj, OPEL_MESSAGE_KEY_IS_FILE_ATTACHED,
        this->mFileName.c_str());
  }
  return rootObj;
}

#define APPCORE_MESSAGE_KEY_COMMAND_TYPE "commandType"
#define APPCORE_MESSAGE_KEY_PAYLOAD "payload"

AppCoreMessage* AppCoreMessage::jsonToMessage(cJSON* messageObj,
    std::string& uri, BaseMessageType::Value type, bool isFileAttached,
    std::string fileName) {
  // Allocate and initialize a new BaseMessage and return it
  cJSON* commandTypeObj = cJSON_GetObjectItem(messageObj,
      APPCORE_MESSAGE_KEY_COMMAND_TYPE);
  RETURN_IF_INVALID_CJSON_OBJ(commandTypeObj, NULL);
  AppCoreMessageCommandType::Value commandType
    = static_cast<AppCoreMessageCommandType::Value>(atoi(
          commandTypeObj->valuestring));

  cJSON* appCorePayloadObj = cJSON_GetObjectItem(messageObj,
      APPCORE_MESSAGE_KEY_PAYLOAD);
  RETURN_IF_INVALID_CJSON_OBJ(appCorePayloadObj, NULL);
  
  // Allocate and initialize a new AppCoreMessage
  AppCoreMessage* newMessage = new AppCoreMessage(uri, type, isFileAttached,
      fileName, commandType, appCorePayloadObj);
  return newMessage;
}

cJSON* AppCoreMessage::toJson() {
  cJSON* rootPayloadObj = cJSON_CreateObject();
  char commandTypeStr[20];
  sprintf(commandTypeStr, "%d", this->mCommandType);
  cJSON_AddStringToObject(rootPayloadObj, APPCORE_MESSAGE_KEY_COMMAND_TYPE,
      commandTypeStr);
  cJSON_AddItemToObject(rootPayloadObj, APPCORE_MESSAGE_KEY_PAYLOAD,
      this->mAppCorePayloadObj);

  cJSON* rootObj = BaseMessage::toJson();
  cJSON_AddItemToObject(rootObj, OPEL_MESSAGE_KEY_PAYLOAD, rootPayloadObj);
  return rootObj;
}

#define APP_MESSAGE_KEY_COMMAND_TYPE "commandType"
#define APP_MESSAGE_KEY_PAYLOAD "payload"

AppMessage* AppMessage::jsonToMessage(cJSON* messageObj, std::string& uri,
    BaseMessageType::Value type, bool isFileAttached, std::string fileName) {
  // Allocate and initialize a new BaseMessage and return it
  cJSON* commandTypeObj = cJSON_GetObjectItem(messageObj, "commandType");
  RETURN_IF_INVALID_CJSON_OBJ(commandTypeObj, NULL);
  AppMessageCommandType::Value commandType
    = static_cast<AppMessageCommandType::Value>(atoi(
          commandTypeObj->valuestring));

  cJSON* appPayloadObj = cJSON_GetObjectItem(messageObj, "payload");
  RETURN_IF_INVALID_CJSON_OBJ(appPayloadObj, NULL);
  
  // Allocate and initialize a new AppMessage
  AppMessage* newMessage = new AppMessage(uri, type, isFileAttached, fileName, 
      commandType, appPayloadObj);
  return newMessage;
}

cJSON* AppMessage::toJson() {
  cJSON* rootPayloadObj = cJSON_CreateObject();
  char commandTypeStr[20];
  sprintf(commandTypeStr, "%d", this->mCommandType);
  cJSON_AddStringToObject(rootPayloadObj, APP_MESSAGE_KEY_COMMAND_TYPE,
      commandTypeStr);
  cJSON_AddItemToObject(rootPayloadObj, APP_MESSAGE_KEY_PAYLOAD,
      this->mAppPayloadObj);

  cJSON* rootObj = BaseMessage::toJson();
  cJSON_AddItemToObject(rootObj, OPEL_MESSAGE_KEY_PAYLOAD, rootPayloadObj);
  return rootObj;
}

bool AppCoreMessage::getParamsListenAppState(int& appId) { 
  cJSON* appIdObj = cJSON_GetObjectItem(this->mAppCorePayloadObj, "appId");
  RETURN_IF_INVALID_CJSON_OBJ(appIdObj, false);
  appId = atoi(appIdObj->valuestring);

  return true;
}

bool AppCoreMessage::getParamsInitializeApp(std::string& name) { 
  cJSON* nameObj = cJSON_GetObjectItem(this->mAppCorePayloadObj, "name");
  RETURN_IF_INVALID_CJSON_OBJ(nameObj, false);
  name.assign(nameObj->valuestring);

  return true;
}

bool AppCoreMessage::getParamsInstallApp(int& appId,
    std::string& packageFileName) { 
  cJSON* appIdObj = cJSON_GetObjectItem(this->mAppCorePayloadObj, "appId");
  RETURN_IF_INVALID_CJSON_OBJ(appIdObj, false);
  appId = atoi(appIdObj->valuestring);

  cJSON* packageFileNameObj = cJSON_GetObjectItem(this->mAppCorePayloadObj,
      "packgeFileName");
  RETURN_IF_INVALID_CJSON_OBJ(packageFileNameObj, false);
  packageFileName.assign(packageFileNameObj->valuestring);

  return true;
}

bool AppCoreMessage::getParamsLaunchApp(int& appId) { 
  cJSON* appIdObj = cJSON_GetObjectItem(this->mAppCorePayloadObj, "appId");
  RETURN_IF_INVALID_CJSON_OBJ(appIdObj, false);
  appId = atoi(appIdObj->valuestring);

  return true;
}

bool AppCoreMessage::getParamsCompleteLaunchingApp(int& appId, int& pid) { 
  cJSON* appIdObj = cJSON_GetObjectItem(this->mAppCorePayloadObj, "appId");
  RETURN_IF_INVALID_CJSON_OBJ(appIdObj, false);
  appId = atoi(appIdObj->valuestring);

  cJSON* pidObj = cJSON_GetObjectItem(this->mAppCorePayloadObj, "pid");
  RETURN_IF_INVALID_CJSON_OBJ(pidObj, false);
  pid = atoi(pidObj->valuestring);

  return true;
}

bool AppCoreMessage::getParamsTerminateApp(int& appId) { 
  cJSON* appIdObj = cJSON_GetObjectItem(this->mAppCorePayloadObj, "appId");
  RETURN_IF_INVALID_CJSON_OBJ(appIdObj, false);
  appId = atoi(appIdObj->valuestring);

  return true;
}

bool AppCoreMessage::getParamsRemoveApp(int& appId) { 
  cJSON* appIdObj = cJSON_GetObjectItem(this->mAppCorePayloadObj, "appId");
  RETURN_IF_INVALID_CJSON_OBJ(appIdObj, false);
  appId = atoi(appIdObj->valuestring);

  return true;
}

bool AppCoreMessage::getParamsGetFileList(std::string& path) { 
  cJSON* pathObj = cJSON_GetObjectItem(this->mAppCorePayloadObj, "path");
  RETURN_IF_INVALID_CJSON_OBJ(pathObj, false);
  path.assign(pathObj->valuestring);

  return true;
}

bool AppCoreMessage::getParamsGetFile(std::string& path) { 
  cJSON* pathObj = cJSON_GetObjectItem(this->mAppCorePayloadObj, "path");
  RETURN_IF_INVALID_CJSON_OBJ(pathObj, false);
  path.assign(pathObj->valuestring);

  return true;
}
