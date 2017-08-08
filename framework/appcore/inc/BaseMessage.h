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

#ifndef __OPEL_MESSAGE_H__
#define __OPEL_MESSAGE_H__

#include <iostream>

#include "BaseMessage.h"
#include "cJSON.h"

// Use namespace + enum for readability
namespace BaseMessageType {
  enum Value {
    NotDetermined = 0,
    AppCore = 1,
    App = 2,
    Companion = 3
  };
}

// TODO: Implementation of AckMessage, ConMessage

class BaseMessage {
  public:
    // rawString(JSON-formed) -> BaseMessage
    static BaseMessage* parse(const char* rawString);

    // File attachment
    // Attached file will be transferred to targetor be received from target
    // in each Channel.
    // Some Channel can refuse to transfer it due to its capability.
    // ex. CommChannel and LocalChnanel can handle attached file, but
    //     DbusChannel cannot.
    // Used to attach file on message to be sent
    BaseMessage* attachFile(std::string filePath);
    // Used to set local file path when attached file has came
    BaseMessage* setStoredFilePath(std::string storedFilePath) {
      this->mStoredFilePath = storedFilePath;
      return this;
    }
    std::string& getStoredFilePath() {
      return this->mStoredFilePath;
    }

    virtual cJSON* toJson();
    virtual char* toRawString() {
      cJSON* jsonObj = this->toJson();
      return cJSON_Print(jsonObj);
    }

    std::string& getUri() { return this->mUri; }
    BaseMessageType::Value getType() { return this->mType; }
    bool isFileAttached() { return this->mIsFileAttached; }
    std::string& getFileName() { return this->mFileName; }

  protected:
    BaseMessage(std::string uri, BaseMessageType::Value type,
        bool isFileAttached, std::string fileName)
      : mUri(uri), mType(type),
      mIsFileAttached(isFileAttached), mFileName(fileName),
      mStoredFilePath("") {
    }
    BaseMessage(std::string uri, BaseMessageType::Value type)
      : mUri(uri), mType(type),
      mIsFileAttached(false), mFileName(""),
      mStoredFilePath("") {
    }

    // JSON-exported values
    std::string mUri;
    BaseMessageType::Value mType;
    bool mIsFileAttached;
    std::string mFileName;

    // Internal value
    // StoreFilePath: the local storage path of attached file
    std::string mStoredFilePath;
};

// Use namespace + enum for readability
namespace AppCoreMessageCommandType {
  enum Value {
    NotDetermined = 0,
    GetAppList = 1, // params: void
    ListenAppState = 2, // params: int appId
    InitializeApp = 3, // params: std::string name
    InstallApp = 4, // params: int appId
    LaunchApp = 5, // params: int appId
    CompleteLaunchingApp = 6, // params: int appId, int pid
    TerminateApp = 7, // params: int appId
    RemoveApp = 8, // params: int appId
    GetFileList = 9, // params: std::string path
    GetFile = 10, // params: std::string path
    GetRootPath = 11 // params: void
  };
}

class AppCoreMessage: public BaseMessage {
  public:
    static AppCoreMessage* jsonToMessage(cJSON* messageObj, std::string& uri,
        BaseMessageType::Value type, bool isFileAttached, std::string fileName);

    virtual cJSON* toJson();
    virtual char* toRawString() {
      cJSON* jsonObj = this->toJson();
      return cJSON_Print(jsonObj);
    }

    AppCoreMessageCommandType::Value getCommandType() {
      return this->mCommandType;
    }
    cJSON* getAppCorePayloadObj() { return this->mAppCorePayloadObj; }

    bool getParamsListenAppState(int& appId);
    bool getParamsInitializeApp(std::string& name);
    bool getParamsInstallApp(int& appId, std::string& packgeFileName);
    bool getParamsLaunchApp(int& appId);
    bool getParamsCompleteLaunchingApp(int& appId, int& pid);
    bool getParamsTerminateApp(int& appId);
    bool getParamsRemoveApp(int& appId);
    bool getParamsGetFileList(std::string& path);
    bool getParamsGetFile(std::string& path);

  protected:
    AppCoreMessage(std::string uri, BaseMessageType::Value type,
        bool isFileAttached, std::string fileName,
        AppCoreMessageCommandType::Value commandType, cJSON* appCorePayloadObj)
      : BaseMessage(uri, type, isFileAttached, fileName),
      mCommandType(commandType), mAppCorePayloadObj(appCorePayloadObj) {
    }

    AppCoreMessage(std::string uri, BaseMessageType::Value type,
        AppCoreMessageCommandType::Value commandType, cJSON* appCorePayloadObj)
      : BaseMessage(uri, type), mCommandType(commandType),
      mAppCorePayloadObj(appCorePayloadObj) {
    }

    // JSON-exported values
    AppCoreMessageCommandType::Value mCommandType;
    cJSON* mAppCorePayloadObj;
};

// Use namespace + enum for readability
namespace AppMessageCommandType {
  enum Value {
    NotDetermined = 0,
    Terminate = 1
  };
}

class AppMessage: public BaseMessage {
  public:
    static AppMessage* jsonToMessage(cJSON* messageObj, std::string& uri,
        BaseMessageType::Value type, bool isFileAttached, std::string fileName);
    // make
    static AppMessage* make(std::string uri,
        AppMessageCommandType::Value commandType, cJSON* appPayloadObj) {
      // TODO: Making AppPayload (Layer 3/4 Implementation)
      return new AppMessage(uri, BaseMessageType::App, commandType,
          appPayloadObj);
    }

    virtual cJSON* toJson();
    virtual char* toRawString() {
      cJSON* jsonObj = this->toJson();
      return cJSON_Print(jsonObj);
    }

    AppMessageCommandType::Value getCommandType() { return this->mCommandType; }
    cJSON* getAppPayloadObj() { return this->mAppPayloadObj; }

  protected:
    AppMessage(std::string uri, BaseMessageType::Value type,
        bool isFileAttached, std::string fileName,
        AppMessageCommandType::Value commandType, cJSON* appPayloadObj)
      : BaseMessage(uri, type, isFileAttached, fileName),
      mCommandType(commandType), mAppPayloadObj(appPayloadObj) {
    }

    AppMessage(std::string uri, BaseMessageType::Value type,
        AppMessageCommandType::Value commandType, cJSON* appPayloadObj)
      : BaseMessage(uri, type), mCommandType(commandType),
      mAppPayloadObj(appPayloadObj) {
    }

    // JSON-exported values
    AppMessageCommandType::Value mCommandType;
    cJSON* mAppPayloadObj;
};

#endif // !defined(__OPEL_MESSAGE_H__)
