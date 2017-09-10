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

#ifndef __MESSAGE_ROUTER_H__
#define __MESSAGE_ROUTER_H__

#include <pthread.h>
#include <iostream>
#include <map>

#include "BaseMessage.h"
#include "Channel.h"

class Channel;
class MessageRouter;

class MessageRouter {
  public:
    MessageRouter() : mMasterRoutingTableMutex(PTHREAD_MUTEX_INITIALIZER) {
    }

    // Routing functions
    void addRoutingEntry(const char* uriString, Channel* channel);
    void removeRoutingEntry(const char* uriString);
    void routeMessage(BaseMessage* message);

  protected:
    Channel* findBestChannelLocked(const char* uriString);
    Channel* findExactChannelLocked(const char* uriString);

    // Master Routing Table
    //   entry = std::pair<const char* uriString, Channel* channel>
    std::map<const char*, Channel*> mMasterRoutingTable;
    pthread_mutex_t mMasterRoutingTableMutex;
};

#endif // !defined(__MESSAGE_ROUTER_H__)