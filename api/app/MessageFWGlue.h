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

#ifndef __MESSAGE_ADAPTER_H__
#define __MESSAGE_ADAPTER_H__

#include "MessageRouter.h"
#include "DbusChannel.h"
#include "LocalChannel.h"
#include "BaseMessage.h"

class MessageFWGlue
: public LocalChannelListener {

  public:
    MessageFWGlue() {
    }
    ~MessageFWGlue() {
      delete this->mMessageRouter;
      delete this->mDbusChannel;
      delete this->mLocalChannel;
    }

    // Run message framework
    void run();

    // LocalChannelListener
    virtual void onReceivedMessage(BaseMessage* message);

  private:
    MessageRouter* mMessageRouter;
    DbusChannel* mDbusChannel;
    // TODO: Make MessageFWGlue's own LocalChannel
    LocalChannel* mLocalChannel;
};

#endif // !defined(__MESSAGE_ADAPTER_H__)
