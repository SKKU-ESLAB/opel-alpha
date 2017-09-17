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

#include <cstring>
#include <sys/types.h>
#include <unistd.h>

#include "MessageRouter.h"
#include "OPELdbugLog.h"

void MessageRouter::addRoutingEntry(const char* uriString, Channel* channel) {
  pthread_mutex_lock(&this->mMasterRoutingTableMutex);
  std::pair<const char*, Channel*> newEntry(uriString, channel);
  this->mMasterRoutingTable.insert(newEntry);
  pthread_mutex_unlock(&this->mMasterRoutingTableMutex);

  OPEL_DBG_VERB("Entry (%s -> %s) is added to MessageRouterTable",
      uriString, channel->getName().c_str());
}

void MessageRouter::removeRoutingEntry(const char* uriString) {
  pthread_mutex_lock(&this->mMasterRoutingTableMutex);

  bool found = false;
  Channel* targetChannel = NULL;
  std::map<const char*, Channel*>::iterator rtIter;
  for(rtIter = this->mMasterRoutingTable.begin();
      rtIter != this->mMasterRoutingTable.end();
      ++rtIter) {
    std::string entryUri(rtIter->first);
    if(entryUri.compare(uriString) == 0) {
      // Remove from routing table
      this->mMasterRoutingTable.erase(rtIter);

      targetChannel = rtIter->second;
      found = true;
      break;
    }
  }

  if(found == false) {
    OPEL_DBG_WARN("Cannot find entry for URI %s", uriString);
  }

  OPEL_DBG_VERB("Entry (%s -> %s) is removed from MessageRouterTable",
      uriString, targetChannel->getName().c_str());

  pthread_mutex_unlock(&this->mMasterRoutingTableMutex);
}

void MessageRouter::printRoutingTable() {
  pthread_mutex_lock(&this->mMasterRoutingTableMutex);

  std::map<const char*, Channel*>::iterator rtIter;
  for(rtIter = this->mMasterRoutingTable.begin();
      rtIter != this->mMasterRoutingTable.end();
      ++rtIter) {
    std::string entryUri(rtIter->first);
    Channel* entryChannel = rtIter->second;
    OPEL_DBG_VERB("Entry %s: %s",
        entryUri.c_str(), entryChannel->getName().c_str());
  }

  pthread_mutex_unlock(&this->mMasterRoutingTableMutex);
}

void MessageRouter::routeMessage(BaseMessage* message) {
  const char* uriString = message->getUri().c_str();

  // Find all the target entry of given URI 
  Channel* targetChannel = this->findBestChannelLocked(uriString);

  // If the message did not routed at all, make a warning message
  if(targetChannel != NULL) {
    OPEL_DBG_VERB("(pid=%d) Route to %s(%s), %s",
        (int)getpid(), uriString,
        targetChannel->getName().c_str(), message->toJSONString());
    targetChannel->routeMessage(message);
  } else {
    OPEL_DBG_WARN("(pid=%d) Message did not routed!: %s",
        (int)getpid(), message->toJSONString());
    this->printRoutingTable();
  }
}

Channel* MessageRouter::findBestChannelLocked(const char* uriString) {
  // Find all the target entry of given URI 
  pthread_mutex_lock(&this->mMasterRoutingTableMutex);
  Channel* targetChannel = NULL;
  const char* targetUriString = NULL;
  std::map<const char*, Channel*>::iterator rtIter;
  for(rtIter = this->mMasterRoutingTable.begin();
      rtIter != this->mMasterRoutingTable.end();
      ++rtIter) {
    std::string entryUri(rtIter->first);
    Channel* entryChannel = rtIter->second;

    size_t foundPos = entryUri.find(uriString);
    // Select the best matching target
    if(foundPos == 0) {
      if((targetUriString == NULL)
          || (entryUri.size() < strlen(targetUriString))) {
        targetChannel = entryChannel;
        targetUriString = entryUri.c_str();
      }
    }
  }
  pthread_mutex_unlock(&this->mMasterRoutingTableMutex);
  return targetChannel;
}

Channel* MessageRouter::findExactChannelLocked(const char* uriString) {
  // Find all the target entry of given URI 
  pthread_mutex_lock(&this->mMasterRoutingTableMutex);
  Channel* targetChannel = NULL;
  std::map<const char*, Channel*>::iterator rtIter;
  for(rtIter = this->mMasterRoutingTable.begin();
      rtIter != this->mMasterRoutingTable.end();
      ++rtIter) {
    std::string entryUri(rtIter->first);
    Channel* entryChannel = rtIter->second;

    if(entryUri.compare(uriString) == 0) {
      pthread_mutex_unlock(&this->mMasterRoutingTableMutex);
      return entryChannel;
    }
  }
  pthread_mutex_unlock(&this->mMasterRoutingTableMutex);
  return NULL;
}
