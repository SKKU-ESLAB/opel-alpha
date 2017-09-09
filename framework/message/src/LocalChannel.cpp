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

#include <pthread.h>

#include "LocalChannel.h"
#include "BaseMessage.h"
#include "OPELdbugLog.h"

void LocalChannel::run() {
  // Run RoutedLoop on this thread(main thread)
  this->runRoutedLoop(false);
}

void LocalChannel::onRoutedMessage(BaseMessage* message) {
  if(mListener != NULL) {
    this->mListener->onReceivedMessage(message);
  } else {
    OPEL_DBG_WARN("LocalChannel's listener is not set!");
    OPEL_DBG_WARN("Therefore, a message is ignored: %s",
        message->toJSONString());
  }
}

void LocalChannel::sendMessage(BaseMessage* message) {
  // Pass the given message to MessageRouter
  this->mMessageRouter->routeMessage(message);
}
