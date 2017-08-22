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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.opel.opel_manager.model.message.BaseMessage;

abstract public class OPELControllerBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = "CommBroadcastReceiver";
    public static final String ACTION = "com.opel.controller.broadcastreceiver";
    public static final String KEY_EVENT_TYPE = "eventType";

    public static final String EVENT_TYPE_ON_ACK_GET_APP_LIST = "onAckGetAppList";
    public static final String EVENT_TYPE_ON_ACK_LISTEN_APP_STATE = "onAckListenAppState";
    public static final String EVENT_TYPE_ON_ACK_INITIALIZE_APP = "onAckInitializeApp";
    public static final String EVENT_TYPE_ON_ACK_GET_FILE_LIST = "onAckGetFileList";
    public static final String EVENT_TYPE_ON_ACK_GET_FILE = "onAckGetFile";
    public static final String EVENT_TYPE_ON_ACK_GET_ROOT_PATH = "onAckGetRootPath";
    public static final String EVENT_TYPE_ON_SEND_EVENT_PAGE = "onSendEventPage";
    public static final String EVENT_TYPE_ON_SEND_CONFIG_PAGE = "onSendConfigPage";
    public static final String EVENT_TYPE_ON_UPDATE_SENSOR_DATA = "onUpdateSensorData";
    public static final String KEY_COMM_MESSAGE = "message";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action.compareTo(ACTION) == 0) {
            String eventType = intent.getStringExtra(KEY_EVENT_TYPE);
            BaseMessage message = (BaseMessage) intent.getParcelableExtra(KEY_COMM_MESSAGE);
            if (eventType.compareTo(EVENT_TYPE_ON_ACK_GET_APP_LIST) == 0) {
                this.onAckGetAppList(message);
            } else if (eventType.compareTo(EVENT_TYPE_ON_ACK_LISTEN_APP_STATE) == 0) {
                this.onAckListenAppState(message);
            } else if (eventType.compareTo(EVENT_TYPE_ON_ACK_INITIALIZE_APP) == 0) {
                this.onAckInitializeApp(message);
            } else if (eventType.compareTo(EVENT_TYPE_ON_ACK_GET_FILE_LIST) == 0) {
                this.onAckGetFileList(message);
            } else if (eventType.compareTo(EVENT_TYPE_ON_ACK_GET_FILE) == 0) {
                this.onAckGetFile(message);
            } else if (eventType.compareTo(EVENT_TYPE_ON_ACK_GET_ROOT_PATH) == 0) {
                this.onAckGetRootPath(message);
            } else if (eventType.compareTo(EVENT_TYPE_ON_SEND_EVENT_PAGE) == 0) {
                this.onSendEventPage(message);
            } else if (eventType.compareTo(EVENT_TYPE_ON_SEND_CONFIG_PAGE) == 0) {
                this.onSendConfigPage(message);
            } else if (eventType.compareTo(EVENT_TYPE_ON_UPDATE_SENSOR_DATA) == 0) {
                this.onUpdateSensorData(message);
            }
        }
    }

    // AppCoreAckMessage
    abstract public void onAckGetAppList(BaseMessage message);

    abstract public void onAckListenAppState(BaseMessage message);

    abstract public void onAckInitializeApp(BaseMessage message);

    abstract public void onAckGetFileList(BaseMessage message);

    abstract public void onAckGetFile(BaseMessage message);

    abstract public void onAckGetRootPath(BaseMessage message);

    // CompanionMessage
    abstract public void onSendEventPage(BaseMessage message);

    abstract public void onSendConfigPage(BaseMessage message);

    abstract public void onUpdateSensorData(BaseMessage message);
}