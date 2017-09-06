package com.opel.opel_manager.controller;

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

import com.opel.opel_manager.model.message.BaseMessage;

public interface OPELAppCoreStubListener {
    // CommChannel
    public void onCommChannelStateChanged(int prevState, int newState);

    // AppCoreAckMessage
    public void onAckGetAppList(BaseMessage message);

    public void onAckListenAppState(BaseMessage message);

    public void onAckInitializeApp(BaseMessage message);

    public void onAckGetFileList(BaseMessage message);

    public void onAckGetFile(BaseMessage message);

    public void onAckGetRootPath(BaseMessage message);

    public void getAppIcon(BaseMessage message);

    // CompanionMessage
    public void onSendEventPage(BaseMessage message);

    public void onSendConfigPage(BaseMessage message);

    public void onUpdateSensorData(BaseMessage message);
}
