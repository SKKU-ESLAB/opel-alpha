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
#include <sys/types.h>
#include <unistd.h>

#include "AppBase.h"
#include "BaseMessage.h"
#include "MessageFactory.h"

#define COMPANION_DEVICE_URI "/comp0"
#define APPCORE_URI "/thing/appcore"
#define APP_URI "/thing/apps"

void AppBase::run() {
  // Initialize MessageRouter and Channels
  this->mMessageRouter = new MessageRouter();
  this->mDbusChannel = new DbusChannel(this->mMessageRouter);
  this->mLocalChannel = new LocalChannel(this->mMessageRouter, true);

  // Run DbusChannel: run on child thread
  this->mDbusChannel->run();
  this->mMessageRouter->addRoutingEntry(APPCORE_URI, this->mDbusChannel);
  this->mMessageRouter->addRoutingEntry(COMPANION_DEVICE_URI,
      this->mDbusChannel);

  // LocalChannel: run on child thread
  this->mLocalChannel->run();
  this->mMessageRouter->addRoutingEntry(APP_URI, this->mLocalChannel);
}

// Send appcore commands
void AppBase::completeLaunchingApp() {
  // Make appcore message
  BaseMessage* appcoreMessage 
    = MessageFactory::makeAppCoreMessage(APPCORE_URI,
        AppCoreMessageCommandType::CompleteLaunchingApp); 
  AppCoreMessage* appCorePayload
    = (AppCoreMessage*)appcoreMessage->getPayload();
  appCorePayload->setParamsCompleteLaunchingApp(getpid());

  // Send appcore message
  this->mLocalChannel->sendMessage(appcoreMessage);
}

// Send companion commands
void AppBase::sendEventPageToCompanion(const char* jsonData, bool isNoti) {
  if(this->mAppId == -1) {
    OPEL_DBG_ERR("App ID is not initialized!");
    return;
  }

  // Make companion message
  BaseMessage* companionMessage
    = MessageFactory::makeCompanionMessage(COMPANION_DEVICE_URI,
        CompanionMessageCommandType::SendEventPage); 
  CompanionMessage* companionPayload
    = (CompanionMessage*)companionMessage->getPayload();
  companionPayload->setParamsSendEventPage(
      this->mAppId, jsonData, isNoti);

  // Send companion message
  this->mLocalChannel->sendMessage(companionMessage);
}

void AppBase::sendConfigPageToCompanion(const char* jsonData) {
  if(this->mAppId == -1) {
    OPEL_DBG_ERR("App ID is not initialized!");
    return;
  }

  // Make companion message
  BaseMessage* companionMessage
    = MessageFactory::makeCompanionMessage(COMPANION_DEVICE_URI,
        CompanionMessageCommandType::SendConfigPage); 
  CompanionMessage* companionPayload
    = (CompanionMessage*)companionMessage->getPayload();
  companionPayload->setParamsSendConfigPage(
      this->mAppId, jsonData);

  // Send companion message
  this->mLocalChannel->sendMessage(companionMessage);
}

void AppBase::updateSensorDataToCompanion(const char* jsonData) {
  // Make companion message
  BaseMessage* companionMessage
    = MessageFactory::makeCompanionMessage(COMPANION_DEVICE_URI,
        CompanionMessageCommandType::UpdateSensorData); 
  CompanionMessage* companionPayload
    = (CompanionMessage*)companionMessage->getPayload();
  companionPayload->setParamsUpdateSensorData(jsonData);

  // Send companion message
  this->mLocalChannel->sendMessage(companionMessage);
}

void AppBase::onReceivedMessage(BaseMessage* message) {
  if(message == NULL) {
    OPEL_DBG_ERR("Invalid App Message");
    return;
  }

  if(message->getType() != BaseMessageType::App) {
    // App Message
    AppMessage* payload = (AppMessage*)message->getPayload();
    if(payload == NULL) {
      OPEL_DBG_ERR("AppMessage payload does not exist");
      return;
    }

    switch(payload->getCommandType()) {
      case AppMessageCommandType::Terminate:
        this->terminate(message);
        break;
      case AppMessageCommandType::UpdateAppConfig:
        this->updateAppConfig(message);
        break;
      default:
        // Do not handle it
        break;
    }
  } else if(message->getType() != BaseMessageType::AppCoreAck) {
    // AppCore Ack Message
    AppCoreAckMessage* payload = (AppCoreAckMessage*)message->getPayload();
    if(payload == NULL) {
      OPEL_DBG_ERR("AppCoreAckMessage payload does not exist");
      return;
    }

    switch(payload->getCommandType()) {
      case AppCoreMessageCommandType::CompleteLaunchingApp:
        this->onAckCompleteLaunchingApp(message);
        break;
      default:
        // Do not handle it
        break;
    }
  } else {
    OPEL_DBG_ERR("Invalid Message Type");
    return;
  }
}

// TODO: thread issue
void AppBase::terminate(BaseMessage* message) {
  Isolate* isolate = Isolate::GetCurrent();
  TryCatch try_catch;

  printf("[NIL] termination Event (app id: %d)\n", this->mAppId);

  // Call onTerminate callback
  Handle<Value> argv[] = { };
  Local<Function> onTerminateCallback
    = Local<Function>::New(isolate, this->mOnTerminateCallback);
  onTerminateCallback->Call(isolate->GetCurrentContext()->Global(), 0, argv);

  if (try_catch.HasCaught()) {
    Local<Value> exception = try_catch.Exception();
    String::Utf8Value exception_str(exception);
    printf("Exception: %s\n", *exception_str);
  }

  // Terminate this app
  exit(1);
}

// TODO: thread issue
void AppBase::updateAppConfig(BaseMessage* message) {
  Isolate* isolate = Isolate::GetCurrent();
  TryCatch try_catch;

  // Arguments
  std::string legacyData;

  // Get arguments
  AppMessage* payload = (AppMessage*)message->getPayload();
  payload->getParamsUpdateAppConfig(legacyData);

  printf("[NIL] Receive Config value :%s\n", legacyData.c_str());

  // Call onUpdateAppConfig callback
  const uint8_t* jsonData;
  jsonData = (uint8_t*)legacyData.c_str();
  Handle<Value> argv[] = { String::NewFromOneByte(isolate, jsonData) };
  Local<Function> onUpdateAppConfigCallback
    = Local<Function>::New(isolate, mOnUpdateAppConfigCallback);
  onUpdateAppConfigCallback->Call(
      isolate->GetCurrentContext()->Global(), 1, argv);

  if (try_catch.HasCaught()) {
    Local<Value> exception = try_catch.Exception();
    String::Utf8Value exception_str(exception);
    printf("Exception: %s\n", *exception_str);

    // Make ACK message
    BaseMessage* ackMessage
      = MessageFactory::makeAppAckMessage(COMPANION_DEVICE_URI, message); 
    AppAckMessage* ackPayload = (AppAckMessage*)ackMessage->getPayload();
    ackPayload->setParamsUpdateAppConfig(false);

    // Send ACK message
    this->mLocalChannel->sendMessage(ackMessage);
    return;
  }

  // Make ACK message
  BaseMessage* ackMessage
    = MessageFactory::makeAppAckMessage(COMPANION_DEVICE_URI, message); 
  AppAckMessage* ackPayload = (AppAckMessage*)ackMessage->getPayload();
  ackPayload->setParamsUpdateAppConfig(true);

  // Send ACK message
  this->mLocalChannel->sendMessage(ackMessage);
  return;
}

void AppBase::onAckCompleteLaunchingApp(BaseMessage* message) {
  // Arguments
  int appId = -1;

  // Get arguments
  AppCoreAckMessage* payload = (AppCoreAckMessage*)message->getPayload();
  payload->getParamsCompleteLaunchingApp(appId);

  // Set app id
  this->mAppId = appId;
}
