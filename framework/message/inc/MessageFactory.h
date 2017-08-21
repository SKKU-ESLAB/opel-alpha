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

  // Make AppCoreAckMessage from bottom
  static AppCoreAckMessage* makeAppCoreAckMessage(
      int commandMessageId, AppCoreMessageCommandType::Value commandType) {
    return new AppCoreAckMessage(commandMessageId, commandType);
  }

  // Make AppMessage from bottom
  static AppMessage* makeAppMessage(AppMessageCommandType::Value commandType) {
    return new AppMessage(commandType);
  }

  // Make CompanionMesage from bottom
  static CompanionMessage* makeCompanionMessage(
      CompanionMessageCommandType::Value commandType) {
    return new CompanionMessage(commandType);
  }

  protected:
  // JSON -> BaseMessage / AppCoreMessage / AppMessage
  static BaseMessage* makeBaseMessageFromJSON(cJSON* messageObj);
  static AppCoreMessage* makeAppCoreMessageFromJSON(cJSON* messagePayloadObj);
  static AppMessage* makeAppMessageFromJSON(cJSON* messagePayloadObj);
};

#endif // !defined(__MESSAGE_FACTORY_H__)
