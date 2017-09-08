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

#include "MessageFWGlue.h"

#define APP_URI "/thing/apps"

MessageFWGlue* MessageFWGlue::singleton = null;

// TODO: call it in nil.cc
MessageFWGlue* MessageFWGlue::get() {
  if(MessageFWGlue::singleton == null) {
    MessageFWGlue::singleton = new MessageFWGlue();
  }
  return MessageFWGlue::singleton;
}

void MessageFWGlue::run() {
  // Initialize MessageRouter and Channels
  this->mMessageRouter = new MessageRouter();
  this->mDbusChannel = new DbusChannel(this->mMessageRouter);
  this->mLocalChannel = new LocalChannel(this->mMessageRouter);

  // Run DbusChannel: run on child thread
  this->mDbusChannel->run();
  this->mMessageRouter->addRoutingEntry(APP_URI, this->mDbusChannel);
  
  // LocalChannel: run on child thread
  this->mLocalChannel->run();
}

void MessageFWGlue::onReceivedMessage(BaseMessage* message) {
  // TODO: 
}
