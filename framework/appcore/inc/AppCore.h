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

#ifndef __APP_CORE_H__
#define __APP_CORE_H__

#include "LocalChannel.h"
#include "App.h"
#include "MessageRouter.h"
#include "DbusChannel.h"
#include "CommChannel.h"
#include "BaseMessage.h"

#include <iostream>

class LocalChannel;

class AppCore: public CommChannelStateListener, public LocalChannelListener {
  public:
    AppCore() {
    }

    ~AppCore() {
      delete this->mMessageRouter;
      delete this->mDbusChannel;
      delete this->mCommChannel;
      delete this->mLocalChannel;
    }

    // Main loop
    void run();

    // Signal handler
    void onSignalSIGCHLD();

    // Appcore Manager's global data
    const char* getUserAppsDir() { return this->mUserAppsDir; }
    const char* getSystemAppsDir() { return this->mSystemAppsDir; }
    const char* getDataDir() { return this->mDataDir; }
    const char* getTempDir() { return this->mTempDir; }

    // LocalChannelListener
    virtual void onReceivedMessage(BaseMessage* message);

    // CommStateListener
    virtual void onCommChannelStateChanged(CommChannelState::Value state);

  protected:
    std::vector<App> mApps;

    bool initializeDirs();

    // Appcore Commands
    void getAppList(AppCoreMessage* message);
    void listenAppState(AppCoreMessage* message);
    void initializeApp(AppCoreMessage* message);
    void installApp(AppCoreMessage* message);
    void launchApp(AppCoreMessage* message);
    void completeLaunchingApp(AppCoreMessage* message);
    void terminateApp(AppCoreMessage* message);
    void removeApp(AppCoreMessage* message);
    void getFileList(AppCoreMessage* message);
    void getFile(AppCoreMessage* message);
    void getRootPath(AppCoreMessage* message);

    MessageRouter* mMessageRouter;
    DbusChannel* mDbusChannel;
    CommChannel* mCommChannel;
    LocalChannel* mLocalChannel;

    char mUserAppsDir[1024]; // User Apps Dir: ${OPEL_APPS_DIR}/user
    char mSystemAppsDir[1024]; // System Apps Dir: ${OPEL_APPS_DIR}/system
    char mDataDir[1024]; // Data Dir: ${OPEL_DATA_DIR}
    char mTempDir[1024]; // Temp Dir: ${OPEL_TEMP_DIR}
};

#endif // !defined(__APP_CORE_H__)
