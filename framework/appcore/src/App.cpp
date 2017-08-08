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
  }

  return isChangeApproved;
}

void App::onChangedState() {
  switch(this->mState) {
    case AppState::Initializing:
      this->onInitializing();
      break;
    case AppState::Initialized:
      this->onInitialized();
      break;
    case AppState::Installing:
      this->onInstalling();
      break;
    case AppState::Ready:
      this->onReady();
      break;
    case AppState::Launching:
      this->onLaunching();
      break;
    case AppState::Running:
      this->onRunning();
      break;
    case AppState::Terminating:
      this->onTerminating();
      break;
    case AppState::Removing:
      this->onRemoving();
      break;
    case AppState::Removed:
      this->onRemoved();
      break;
  }
}

void App::onInitializing() {
  // TODO: not yet implemented
}
void App::onInitialized() {
  // TODO: not yet implemented
}
void App::onInstalling() {
  // TODO: not yet implemented
}
void App::onReady() {
  // TODO: not yet implemented
}
void App::onLaunching() {
  // TODO: not yet implemented
}
void App::onRunning() {
  // TODO: not yet implemented
}
void App::onTerminating() {
  // TODO: not yet implemented
}
void App::onRemoving() {
  // TODO: not yet implemented
}
void App::onRemoved() {
  // TODO: not yet implemented
}
