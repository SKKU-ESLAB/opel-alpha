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

import com.opel.cmfw.service.CommChannelService;

abstract public class OPELControllerBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = "OPELControllerBroadcastReceiver";
    public static final String ACTION = "com.opel.controller.broadcastreceiver";
    public static final String KEY_EVENT_TYPE = "eventType";

    public static final String EVENT_TYPE_ON_COMM_CHANNEL_STATE_CHANGED =
            "onCommChannelStateChanged";
    public static final String KEY_ON_COMM_CHANNEL_STATE_CHANGED_PREV_STATE = "prevState";
    public static final String KEY_ON_COMM_CHANNEL_STATE_CHANGED_NEW_STATE = "newState";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action.compareTo(ACTION) == 0) {
            String eventType = intent.getStringExtra(KEY_EVENT_TYPE);
            if (eventType.compareTo(EVENT_TYPE_ON_COMM_CHANNEL_STATE_CHANGED) == 0) {
                // TODO: implement it
                int prevState = intent.getIntExtra(KEY_ON_COMM_CHANNEL_STATE_CHANGED_PREV_STATE,
                        CommChannelService.STATE_DISCONNECTED);
                int newState = intent.getIntExtra(KEY_ON_COMM_CHANNEL_STATE_CHANGED_NEW_STATE,
                        CommChannelService.STATE_DISCONNECTED);
                this.onCommChannelStateChanged(prevState, newState);
            }
        }
    }

    abstract public void onCommChannelStateChanged(int prevState, int newState);
}