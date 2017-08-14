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
#include <string>
#include <vector>

#include "cJSON.h"

class MessageFactory;

// Use namespace + enum for readability
namespace BaseMessageType {
  enum Value {
    NotDetermined = 0,
    AppCore = 10,
    AppCoreAck = 11,
    App = 20,
    Companion = 30
  };
}

#define OPEL_MESSAGE_KEY_MESSAGE_NUM "messageId"
#define OPEL_MESSAGE_KEY_URI "uri"
#define OPEL_MESSAGE_KEY_TYPE "type"
#define OPEL_MESSAGE_KEY_IS_FILE_ATTACHED "isFileAttached"
#define OPEL_MESSAGE_KEY_FILE_NAME "fileName"
#define OPEL_MESSAGE_KEY_PAYLOAD "payload"

// BaseMessage: the root base message
// - Decoding(makeFromJSON): C++, Java
// - Encoding(make, toJSON): C++, Java
class BaseMessage {
  public:
    friend class MessageFactory;

    // File attachment
    // Attached file will be transferred to target or be received from target
    // in each Channel.
    // Some Channel can refuse to transfer it due to its capability.
    // ex. CommChannel and LocalChnanel can handle attached file, but
    //     DbusChannel cannot.
    
    // Attach file on message to be sent
    BaseMessage* attachFile(std::string filePath);

    // Set local file path when attached file has came
    BaseMessage* setStoredFilePath(std::string storedFilePath) {
      this->mStoredFilePath = storedFilePath;
      return this;
    }
    std::string& getStoredFilePath() {
      return this->mStoredFilePath;
    }

    // encoding to JSON
    virtual cJSON* toJSON();
    char* toJSONString() {
      cJSON* jsonObj = this->toJSON();
      return cJSON_Print(jsonObj);
    }

    // Get parameters
    int getMessageId() { return this->mMessageId; }
    std::string& getUri() { return this->mUri; }
    BaseMessageType::Value getType() { return this->mType; }
    bool isFileAttached() { return this->mIsFileAttached; }
    std::string& getFileName() { return this->mFileName; }

  protected:
    BaseMessage(int messageId, std::string uri, BaseMessageType::Value type,
        bool isFileAttached, std::string fileName)
      : mMessageId(messageId), mUri(uri), mType(type),
      mIsFileAttached(isFileAttached), mFileName(fileName),
      mStoredFilePath("") {
    }
    BaseMessage(int messageId, std::string uri, BaseMessageType::Value type)
      : mMessageId(messageId), mUri(uri), mType(type),
      mIsFileAttached(false), mFileName(""),
      mStoredFilePath("") {
    }

    // JSON-exported values
    int mMessageId;
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
    GetAppList = 1, // params: void (ACK params= ParamAppList)
    ListenAppState = 2, // params: int appId (ACK params: int appState)
    InitializeApp = 3, // params: std::string name
    InstallApp = 4, // params: int appId
    LaunchApp = 5, // params: int appId
    CompleteLaunchingApp = 6, // params: int appId, int pid
    TerminateApp = 7, // params: int appId
    RemoveApp = 8, // params: int appId
    GetFileList = 9, // params: std::string path (ACK params: AckParamFileList)
    GetFile = 10, // params: std::string path (ACK params: void)
    GetRootPath = 11 // params: void (ACK params: std::string rootPath)
  };
}

#define APPCORE_MESSAGE_KEY_COMMAND_TYPE "commandType"
#define APPCORE_MESSAGE_KEY_PAYLOAD "payload"

// AppCoreMessage: message sent to AppCore Framework
// - Decoding(makeFromJSON): C++
// - Encoding(make, toJSON): Java
class AppCoreMessage: public BaseMessage {
  public:
    friend class MessageFactory;

    ~AppCoreMessage() {
      if(this->mAppCorePayloadObj != NULL) {
        cJSON_Delete(this->mAppCorePayloadObj);
      }
    }

    // encoding to JSON: not implemented for C++

    // Get parameters
    AppCoreMessageCommandType::Value getCommandType() {
      return this->mCommandType;
    }
    cJSON* getAppCorePayloadObj() { return this->mAppCorePayloadObj; }

    // Set command-specific parameters
    cJSON* setAppCorePayloadObj(cJSON* appCorePayloadObj) {
      this->mAppCorePayloadObj = appCorePayloadObj;
    }

    // Get command-specific parameters
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
    // Initializer without file
    AppCoreMessage(int messageId, std::string uri,
        AppCoreMessageCommandType::Value commandType)
      : BaseMessage(messageId, uri, BaseMessageType::AppCore),
      mCommandType(commandType), mAppCorePayloadObj(NULL) {
    }

    // Initializer with file
    AppCoreMessage(int messageId, std::string uri,
        bool isFileAttached, std::string fileName,
        AppCoreMessageCommandType::Value commandType)
      : BaseMessage(messageId, uri, BaseMessageType::AppCore,
          isFileAttached, fileName),
      mCommandType(commandType), mAppCorePayloadObj(NULL) {
    }

    // JSON-exported values
    AppCoreMessageCommandType::Value mCommandType;
    cJSON* mAppCorePayloadObj;
};

class ParamAppListEntry {
  public:
    ParamAppListEntry(int appId, std::string appName, bool isDefaultApp)
      : mAppId(appId), mAppName(appName), mIsDefaultApp(isDefaultApp) { }

    int getAppId() { return this->mAppId; }
    std::string getAppName() { return this->mAppName; }
    bool isDefaultApp() { return this->mIsDefaultApp; }
  private:
    int mAppId;
    std::string mAppName;
    bool mIsDefaultApp;
};

class ParamAppList {
  public:
    // Make
    static ParamAppList* make() {
      return new ParamAppList();
    }

    void addEntry(int appId, std::string appName, bool isDefaultApp) {
      ParamAppListEntry newEntry(appId, appName, isDefaultApp);
      this->mAppList.push_back(newEntry);
    }
    
    // encoding to JSON
    cJSON* toJSON();

    std::vector<ParamAppListEntry>& getList() { return this->mAppList; }

  private:
    ParamAppList() { }
    std::vector<ParamAppListEntry> mAppList;
};

class ParamFileList {
  public:
    // Make
    static ParamFileList* make() {
      return new ParamFileList();
    }

    void addEntry(std::string fileName) {
      this->mFileList.push_back(fileName);
    }
    
    // encoding to JSON
    cJSON* toJSON();

    std::vector<std::string>& getList() { return this->mFileList; }

  private:
    ParamFileList() { }
    std::vector<std::string> mFileList;
};

#define APPCORE_ACK_MESSAGE_KEY_COMMAND_MESSAGE_NUM "commandMessageId"
#define APPCORE_ACK_MESSAGE_KEY_COMMAND_TYPE "commandType"
#define APPCORE_ACK_MESSAGE_KEY_PAYLOAD "payload"

// AppCoreAckMessage: ack message sent from AppCore Framework
// - Decoding(makeFromJSON): Java
// - Encoding(make, toJSON): C++
class AppCoreAckMessage : public BaseMessage {
  public:
    friend class MessageFactory;

    ~AppCoreAckMessage() {
      if(this->mAppCoreAckPayloadObj != NULL) {
        cJSON_Delete(this->mAppCoreAckPayloadObj);
      }
    }

    // encoding to JSON
    virtual cJSON* toJSON();

    // Get parameters
    AppCoreMessageCommandType::Value getCommandType() {
      return this->mCommandType;
    }

    // Set command-specific parameters
    void setParamsGetAppList(ParamAppList& appList);
    void setParamsListenAppState(int appState);
    void setParamsGetFileList(std::string path, ParamFileList& fileList);
    void setParamsGetRootPath(std::string rootPath);

  protected:
    // Initializer without file
    AppCoreAckMessage(int messageId, std::string uri,
        int commandMessageId,
        AppCoreMessageCommandType::Value commandType)
      : BaseMessage(messageId, uri, BaseMessageType::AppCoreAck),
      mCommandMessageId(commandMessageId),
      mCommandType(commandType), mAppCoreAckPayloadObj(NULL) { }

    // JSON-exported values
    int mCommandMessageId;
    AppCoreMessageCommandType::Value mCommandType;
    cJSON* mAppCoreAckPayloadObj;
};

// Use namespace + enum for readability
namespace AppMessageCommandType {
  enum Value {
    NotDetermined = 0,
    Terminate = 1
  };
}

#define APP_MESSAGE_KEY_COMMAND_TYPE "commandType"
#define APP_MESSAGE_KEY_PAYLOAD "payload"

// AppMessage: message sent to App
// - Decoding(makeFromJSON): C++
// - Encoding(make, toJSON): C++
class AppMessage: public BaseMessage {
  public:
    friend class MessageFactory;

    ~AppMessage() {
      if(this->mAppPayloadObj != NULL) {
        cJSON_Delete(this->mAppPayloadObj);
      }
    }

    // encoding to JSON
    virtual cJSON* toJSON();

    // Get parameters
    AppMessageCommandType::Value getCommandType() { return this->mCommandType; }
    cJSON* getAppPayloadObj() { return this->mAppPayloadObj; }

    // Set command-specific parameters
    void setAppPayloadObj(cJSON* appPayloadObj) {
      this->mAppPayloadObj = appPayloadObj;
    }

  protected:
    // Initializer without file
    AppMessage(int messageId, std::string uri,
        AppMessageCommandType::Value commandType)
      : BaseMessage(messageId, uri, BaseMessageType::App),
      mCommandType(commandType), mAppPayloadObj(NULL) {
    }

    // Initializer with file
    AppMessage(int messageId, std::string uri,
        bool isFileAttached, std::string fileName,
        AppMessageCommandType::Value commandType)
      : BaseMessage(messageId, uri, BaseMessageType::App,
          isFileAttached, fileName),
      mCommandType(commandType), mAppPayloadObj(NULL) {
    }

    // JSON-exported values
    AppMessageCommandType::Value mCommandType;
    cJSON* mAppPayloadObj;
};

namespace CompanionMessageCommandType {
  enum Value {
    NotDetermined = 0,
    SendEventPage = 1, // params: string legacyData
    SendConfigPage = 2 // params: string legacyData
  };
}

#define COMPANION_MESSAGE_KEY_COMMAND_TYPE "commandType"
#define COMPANION_MESSAGE_KEY_PAYLOAD "payload"

// CompanionMessage: message sent to companion device
// - Decoding(makeFromJSON): Java
// - Encoding(make, toJSON): JavaScript
class CompanionMessage: public BaseMessage {
  public:
    friend class MessageFactory;

    ~CompanionMessage() {
      if(this->mCompanionPayloadObj != NULL) {
        cJSON_Delete(this->mCompanionPayloadObj);
      }
    }

    // encoding to JSON
    virtual cJSON* toJSON();

    // Get parameters
    CompanionMessageCommandType::Value getCommandType() {
      return this->mCommandType;
    }
    cJSON* getCompanionPayloadObj() { return this->mCompanionPayloadObj; }

    // Set command-specific parameters
    void setParamsSendEventPage(std::string legacyData);
    void setParamsSendConfigPage(std::string legacyData);

  protected:
    // Initializer without file
    CompanionMessage(int messageId, std::string uri,
        CompanionMessageCommandType::Value commandType)
      : BaseMessage(messageId, uri, BaseMessageType::Companion),
      mCommandType(commandType), mCompanionPayloadObj(NULL) {
    }

    // JSON-exported values
    CompanionMessageCommandType::Value mCommandType;
    cJSON* mCompanionPayloadObj;
};

#endif // !defined(__OPEL_MESSAGE_H__)
