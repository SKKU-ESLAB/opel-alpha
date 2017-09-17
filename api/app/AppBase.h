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

#include <node.h>

#include "MessageRouter.h"
#include "DbusChannel.h"
#include "LocalChannel.h"
#include "BaseMessage.h"
#include "OPELdbugLog.h"

using namespace v8;

class AppBase
: public LocalChannelListener {

  public:
    AppBase()
      : mAppId(-1),
      mWaitMutex(PTHREAD_MUTEX_INITIALIZER),
      mWaitCond(PTHREAD_COND_INITIALIZER) {
    }
    ~AppBase() {
      if(this->mMessageRouter != NULL)
        delete this->mMessageRouter;
      if(this->mDbusChannel != NULL)
        delete this->mDbusChannel;
      if(this->mLocalChannel != NULL)
        delete this->mLocalChannel;
    }

    // Run message framework
    void run();

    // LocalChannelListener
    virtual void onReceivedMessage(BaseMessage* message);

    // Set callbacks
    void setOnTerminate(Isolate* isolate,
        Local<Function> onTerminateCallback) {
      this->mOnTerminateCallback.Reset(isolate, onTerminateCallback);
    }
    void setOnUpdateAppConfig(Isolate* isolate,
        Local<Function> onUpdateAppConfig) {
      this->mOnUpdateAppConfigCallback.Reset(isolate, onUpdateAppConfig);
    }

    // Send appcore comamnds
    void completeLaunchingApp();

    // Send companion commands
    void sendEventPageToCompanion(const char* jsonData, bool isNoti);
    void sendConfigPageToCompanion(const char* jsonData);
    void updateSensorDataToCompanion(const char* jsonData);

  protected:
    // App ID
    int mAppId;

    // App Commands
    void terminate(BaseMessage* message);
    void updateAppConfig(BaseMessage* message);

    // Appcore Ack
    void onAckCompleteLaunchingApp(BaseMessage* message);

    // Callbacks
    Persistent<Function> mOnTerminateCallback;
    Persistent<Function> mOnUpdateAppConfigCallback;

    // Wait until appId is assigned
    pthread_mutex_t mWaitMutex;
    pthread_cond_t mWaitCond;

    // Message framework
    MessageRouter* mMessageRouter = NULL;
    DbusChannel* mDbusChannel = NULL;
    LocalChannel* mLocalChannel = NULL;
};

#endif // !defined(__MESSAGE_ADAPTER_H__)
