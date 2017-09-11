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

#ifndef __LOCAL_CHANNEL_H__
#define __LOCAL_CHANNEL_H__

#include <vector>
#include <pthread.h>

#include "MessageRouter.h"

class LocalChannelListener {
  public:
    virtual void onReceivedMessage(BaseMessage* message) = 0;
};

class LocalChannel: public Channel {
  public:
    LocalChannel(MessageRouter* messageRouter,
        bool isCreateRoutedThread)
    : Channel(messageRouter, "LocalChannel"), mListener(NULL),
    mIsCreateRoutedThread(isCreateRoutedThread) {
    }

    // Channel function
    virtual void run();

    // LocalChannel-specific function
    void sendMessage(BaseMessage* message);
    void setListener(LocalChannelListener* listener) {
      this->mListener = listener;
    }

  protected:
    LocalChannelListener* mListener = NULL;
    bool mIsCreateRoutedThread;

    // Channel function
    virtual void onRoutedMessage(BaseMessage* message);
};

#endif
