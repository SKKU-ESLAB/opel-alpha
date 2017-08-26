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

#include "App.h"

bool App::changeState(AppState::Value newState) {
  // Check new state
  bool isChangeApproved = false;
  switch(this->mState) {
    case AppState::Initializing:
      if(newState == AppState::Initialized)
        isChangeApproved = true;
      break;
    case AppState::Initialized:
      if(newState == AppState::Installing
          || newState == AppState::Removed)
        isChangeApproved = true;
      break;
    case AppState::Installing:
      if(newState == AppState::Ready
          || newState == AppState::Removed)
        isChangeApproved = true;
      break;
    case AppState::Ready:
      if(newState == AppState::Launching
          || newState == AppState::Removing)
        isChangeApproved = true;
      break;
    case AppState::Launching:
      if(newState == AppState::Running
          || newState == AppState::Ready)
        isChangeApproved = true;
      break;
    case AppState::Running:
      if(newState == AppState::Terminating
          || newState == AppState::Ready)
        isChangeApproved = true;
      break;
    case AppState::Terminating:
      if(newState == AppState::Ready)
        isChangeApproved = true;
      break;
    case AppState::Removing:
      if(newState == AppState::Removed)
        isChangeApproved = true;
      break;
    case AppState::Removed:
      break;
  }

  // Apply new state
  if(isChangeApproved) {
    this->mState = newState;
    if(this->mStateListener != NULL) {
      this->mStateListener->onChangedState(this->mState);
    }
  }
  return isChangeApproved;
}

// Commands
void App::initialize(int appId) {
  this->mAppId = appId;

  this->changeState(AppState::Initialized);
}

void App::install(std::string packageFilePath, bool isDefaultApp) {
  boolean isSuccess = false;
  this->mPackageFilePath = packageFilePath;
  this->changeState(AppState::Installing);

  // TODO: implement it (AppPackageManager::installPackage)

  // On Success
  if(isSuccess) {
    this->mIsDefaultApp = isDefaultApp;
    // TODO: determine mName
    this->changeState(AppState::Ready);
  } else {
    // On Fail
    this->changeState(AppState::Removed);
  }
}

void App::launch() {
  // TODO: determine mPid
  this->changeState(AppState::Launching);

  // TODO: implement it (AppStatusManager::runNewApplication)

  // On Success (Async)
  // TODO: implement it
  // this->changeState(AppState::Running);

  // On Fail (Async)
  // TODO: implement it
  // this->changeState(AppState::Ready);
}

void App::terminate() {
  // TODO: implement it (AppStatusManager::exitApplication)
  this->changeState(AppState::Ready);
}

void App::remove() {
  this->changeState(AppState::Removing);
  // TODO: implement it (AppPackageManager::deletePackage)

  this->changeState(AppState::Removed);
}
