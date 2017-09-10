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

#ifndef __MESSAGE_FACTORY_H__
#define __MESSAGE_FACTORY_H__

#include <string>

#include "BaseMessage.h"

class MessageFactory {
  public:
  // string -> JSON -> BaseMessage
  static BaseMessage* makeMessageFromJSONString(const char* rawString);

  // Make AppCoreMessage from bottom
  static BaseMessage* makeAppCoreMessage(std::string uri,
      AppCoreMessageCommandType::Value commandType) {
    AppCoreMessage* payload = new AppCoreMessage(commandType);
    BaseMessage* message = new BaseMessage(currentMessageId++, uri,
        BaseMessageType::AppCore);
    message->setPayload(payload);
    return message;
  }

  // Make AppCoreAckMessage from bottom
  static BaseMessage* makeAppCoreAckMessage(std::string uri,
      BaseMessage* originalMessage) {
    int commandMessageId = originalMessage->getMessageId();
    AppCoreMessage* originalPayload
      = (AppCoreMessage*) originalMessage->getPayload();
    AppCoreMessageCommandType::Value commandType
      = originalPayload->getCommandType();

    AppCoreAckMessage* payload = new AppCoreAckMessage(commandMessageId,
        commandType);
    BaseMessage* message = new BaseMessage(currentMessageId++, uri,
        BaseMessageType::AppCoreAck);
    message->setPayload(payload);
    return message;
  }

  // Make AppMessage from bottom
  static BaseMessage* makeAppMessage(std::string uri,
      AppMessageCommandType::Value commandType) {
    AppMessage* payload = new AppMessage(commandType);
    BaseMessage* message = new BaseMessage(currentMessageId++, uri,
        BaseMessageType::App);
    message->setPayload(payload);
    return message;
  }

  // Make CompanionMessage from bottom
  static BaseMessage* makeCompanionMessage(std::string uri,
      CompanionMessageCommandType::Value commandType) {
    CompanionMessage* payload = new CompanionMessage(commandType);
    BaseMessage* message = new BaseMessage(currentMessageId++, uri,
        BaseMessageType::Companion);
    message->setPayload(payload);
    return message;
  }

  // Make AppAckMessage from bottom
  static BaseMessage* makeAppAckMessage(std::string uri,
      BaseMessage* originalMessage) {
    int commandMessageId = originalMessage->getMessageId();
    AppMessage* originalPayload = (AppMessage*) originalMessage->getPayload();
    AppMessageCommandType::Value commandType
      = originalPayload->getCommandType();

    AppAckMessage* payload = new AppAckMessage(commandMessageId,
        commandType);
    BaseMessage* message = new BaseMessage(currentMessageId++, uri,
        BaseMessageType::AppAck);
    message->setPayload(payload);
    return message;
  }

  protected:
  // JSON -> BaseMessage / AppCoreMessage / AppMessage
  static BaseMessage* makeBaseMessageFromJSON(cJSON* messageObj);
  static AppCoreMessage* makeAppCoreMessageFromJSON(cJSON* messagePayloadObj);
  static AppMessage* makeAppMessageFromJSON(cJSON* messagePayloadObj);

  static int currentMessageId;
};

#endif // !defined(__MESSAGE_FACTORY_H__)
