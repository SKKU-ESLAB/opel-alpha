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
    Initialized = 2, // set id
    Installing = 3, // set packageFilePath
    Ready = 4, // set isDefaultApp, name, mainJSFilePath
               // store to DB
               // reset pid
    Launching = 5, // set pid
    Running = 6,
    Terminating = 7,
    Removing = 8,
    Removed = 9 // reset id, packageFilePath, isDefaultApp, name, mainJSFilePath
  };
}

class AppStateListener {
  virtual void onChangedState(int appId, AppState::Value newState) = 0;
};

class App {
  public:
    App()
      : mState(AppState::Initializing), mAppStateListener(NULL),
      mId(-1),
      mPackageFilePath(NULL), 
      mIsDefaultApp(false),
      mName(NULL),
      mMainJSFilePath(NULL), mPid(-1) {
    }
    App(int id, std::string pacakgeFilePath,
        bool isDefaultApp, std::string name,
        std::string mainJSFilePath)
      : mState(AppState::Initializing), mAppStateListener(NULL),
      mId(id),
      mPackageFilePath(packageFilePath), 
      mIsDefaultApp(isDefaultApp),
      mName(name),
      mMainJSFilePath(mainJSFilePath), mPid(-1) {
    }

    // Getters
    AppState::Value getState() { return this->mState; }
    int getId() { return this->mId; }
    std::string getPackageFilePath() { return this->mPackageFilePath; }
    bool isDefaultApp() { return this->mIsDefaultApp; }
    std::string getName() { return this->mName; }
    std::string getMainJSFilePath() { return this->mMainJSFilePath; }
    int getPid() { return this->mPid; }
    
    // Commands
    void initialize(int appId);
    void install(std::string packageFilePath, bool isDefaultApp);
    void launch();
    void terminate();
    void remove();

    // Change app state
    bool changeState(AppState::Value newState);

    // State listener
    void setStateListener(AppStateListener* stateListener) {
      this->mStateListener = stateListener;
    }

  protected:
    // State listener
    AppStateListener* mStateListener;

    // App information
    AppState::Value mState;
    int mId; // determined at Initialized, reset at Removed (stored to DB)
    std::string mPackageFilePath; // determined at Installing, reset at Removed (stored to DB)
    bool mIsDefaultApp; // determined at Ready, reset at Removed (stored to DB)
    std::string mName; // determined at Ready, reset at Removed (stored to DB)
    std::string mMainJSFilePath; // determined at Ready, reset at Removed (stored to DB)
    int mPid; // determined at Launching, reset at Ready
};

#endif // !defined(__APP_H__)
