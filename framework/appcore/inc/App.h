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

#ifndef __APP_H__
#define __APP_H__

#include <iostream>

// Use namespace + enum for readability
namespace AppState {
  enum Value {
    Initializing = 1,
    Initialized = 2,
    Installing = 3,
    Ready = 4,
    Launching = 5,
    Running = 6,
    Terminating = 7,
    Removing = 8,
    Removed = 9
  };
}

class App {
  public:
    App()
      : mState(AppState::Initializing), mName(NULL),
      mPackageFilePath(NULL), mMainJSFilePath(NULL), mPid(-1) {
    }

    // Change app state
    bool changeState(AppState::Value newState);

  protected:
    // State change callbacks
    void onChangedState();

    void onInitializing();
    void onInitialized();
    void onInstalling();
    void onReady();
    void onLaunching();
    void onRunning();
    void onTerminating();
    void onRemoving();
    void onRemoved();

    // App information
    AppState::Value mState;
    char* mPackageFilePath; // determined at Installing, reset at Ready
    char* mName; // determined at Ready, reset at Removing
    char* mMainJSFilePath; // determined at Ready, reset at Removing
    int mPid; // determined at Launching, reset at Ready
};

#endif // !defined(__APP_H__)
