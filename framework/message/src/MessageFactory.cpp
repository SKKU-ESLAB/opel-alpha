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

#include "MessageFactory.h"
#include "BaseMessage.h"
#include "cJSON.h"
#include "OPELdbugLog.h"

#define RETURN_IF_INVALID_CJSON_OBJ(a, ret) \
  if((a) == NULL) { \
    OPEL_DBG_ERR("JSON handling error: %s", cJSON_GetErrorPtr()); \
    return ret; \
  }

BaseMessage* MessageFactory::makeMessageFromJSONString(const char* rawString) {
  // Parse rawString in JSON
  cJSON* thisObj = cJSON_Parse(rawString);
  RETURN_IF_INVALID_CJSON_OBJ(thisObj, NULL);
  return makeBaseMessageFromJSON(thisObj);
}

BaseMessage* MessageFactory::makeBaseMessageFromJSON(cJSON* messageObj) {
  cJSON* thisObj = messageObj;

  // Get properties
  // messageId
  cJSON* messageIdObj = cJSON_GetObjectItem(thisObj,
      BASE_MESSAGE_KEY_MESSAGE_NUM);
  RETURN_IF_INVALID_CJSON_OBJ(messageIdObj, NULL);
  int messageId = atoi(messageIdObj->valuestring);
  
  // URI
  cJSON* uriObj = cJSON_GetObjectItem(thisObj, BASE_MESSAGE_KEY_URI);
  RETURN_IF_INVALID_CJSON_OBJ(uriObj, NULL);
  std::string uri(uriObj->valuestring);

  // Type
  cJSON* typeObj = cJSON_GetObjectItem(thisObj, BASE_MESSAGE_KEY_TYPE);
  RETURN_IF_INVALID_CJSON_OBJ(typeObj, NULL);
  // BE AWARE: Since cJSON internally consider integer value as Number(double),
  // I decided to use string-shaped integer in JSON.
  // Once integer value is carried by Number variable, its value can be changed
  // because there is a limit in mantissa's representation.
  BaseMessageType::Value type
    = static_cast<BaseMessageType::Value>(atoi(typeObj->valuestring));

  // isFileAttached
  cJSON* isFileAttachedObj = cJSON_GetObjectItem(thisObj,
      BASE_MESSAGE_KEY_IS_FILE_ATTACHED);
  RETURN_IF_INVALID_CJSON_OBJ(isFileAttachedObj, NULL);
  int isFileAttachedNum = atoi(isFileAttachedObj->valuestring);
  bool isFileAttached = (isFileAttachedNum == 0) ? false : true;

  // FileName
  std::string fileName("");
  if(isFileAttachedNum) {
    cJSON* fileNameObj = cJSON_GetObjectItem(thisObj,
        BASE_MESSAGE_KEY_FILE_NAME);
    RETURN_IF_INVALID_CJSON_OBJ(fileNameObj, NULL);
    fileName = fileNameObj->valuestring;
  }

  cJSON* payloadObj = cJSON_GetObjectItem(thisObj, BASE_MESSAGE_KEY_PAYLOAD);
  RETURN_IF_INVALID_CJSON_OBJ(payloadObj, NULL);

  // Allocate and initialize a new BaseMessage
  BaseMessage* newMessage = new BaseMessage(messageId, uri, type,
      isFileAttached, fileName);
  switch(type) {
    case BaseMessageType::AppCore:
      {
        AppCoreMessage* messagePayload = makeAppCoreMessageFromJSON(payloadObj);
        newMessage->setPayload(messagePayload);
      }
      break;
    case BaseMessageType::App:
      {
        AppMessage* messagePayload = makeAppMessageFromJSON(payloadObj);
        newMessage->setPayload(messagePayload);
      }
      break;
    case BaseMessageType::AppCoreAck:
    case BaseMessageType::Companion:
    default:
      // These types cannot be handled.
      break;
  }
  return newMessage;
}

AppCoreMessage* MessageFactory::makeAppCoreMessageFromJSON(cJSON* messagePayloadObj) {
  cJSON* thisObj = messagePayloadObj;

  // Allocate and initialize a new BaseMessage and return it
  cJSON* commandTypeObj = cJSON_GetObjectItem(thisObj,
      APPCORE_MESSAGE_KEY_COMMAND_TYPE);
  RETURN_IF_INVALID_CJSON_OBJ(commandTypeObj, NULL);
  AppCoreMessageCommandType::Value commandType
    = static_cast<AppCoreMessageCommandType::Value>(atoi(
          commandTypeObj->valuestring));

  cJSON* appCorePayloadObj = cJSON_GetObjectItem(thisObj,
      APPCORE_MESSAGE_KEY_PAYLOAD);
  RETURN_IF_INVALID_CJSON_OBJ(appCorePayloadObj, NULL);
  
  // Allocate and initialize a new AppCoreMessage
  AppCoreMessage* newMessage = new AppCoreMessage(commandType);
  newMessage->setAppCorePayloadObj(appCorePayloadObj);
  return newMessage;
}

AppMessage* MessageFactory::makeAppMessageFromJSON(cJSON* messagePayloadObj) {
  cJSON* thisObj = messagePayloadObj;

  // Allocate and initialize a new BaseMessage and return it
  cJSON* commandTypeObj = cJSON_GetObjectItem(thisObj, "commandType");
  RETURN_IF_INVALID_CJSON_OBJ(commandTypeObj, NULL);
  AppMessageCommandType::Value commandType
    = static_cast<AppMessageCommandType::Value>(atoi(
          commandTypeObj->valuestring));

  cJSON* appPayloadObj = cJSON_GetObjectItem(thisObj, "payload");
  RETURN_IF_INVALID_CJSON_OBJ(appPayloadObj, NULL);
  
  // Allocate and initialize a new AppMessage
  AppMessage* newMessage = new AppMessage(commandType);
  newMessage->setAppPayloadObj(appPayloadObj);
  return newMessage;
}
