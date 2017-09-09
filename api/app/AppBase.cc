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

#include "AppBase.h"

#define APP_URI "/thing/apps"

AppBase* AppBase::singleton = null;

void AppBase::run() {
  // Initialize MessageRouter and Channels
  this->mMessageRouter = new MessageRouter();
  this->mDbusChannel = new DbusChannel(this->mMessageRouter);
  this->mLocalChannel = new LocalChannel(this->mMessageRouter,
      BaseMessageType::);

  // Run DbusChannel: run on child thread
  this->mDbusChannel->run();
  this->mMessageRouter->addRoutingEntry(APP_URI, this->mDbusChannel);

  // LocalChannel: run on child thread
  this->mLocalChannel->run();
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
    }
  } else {
    OPEL_DBG_ERR("Invalid Message Type");
    return;
  }
}

void AppBase::terminate(BaseMessage* message) {
  // TODO:
}

void AppBase::onAckCompleteLaunchingApp(BaseMessage* message) {
  // TODO:
}
