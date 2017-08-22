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

import android.app.Service;
import android.content.Intent;

import com.opel.cmfw.glue.CommBroadcastReceiver;

public class OPELControllerBroadcastSender extends {

    public static void onCommChannelStateChanged(Service service, int prevState, int newState) {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_COMM_CHANNEL_STATE_CHANGED);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_COMM_CHANNEL_PREV_STATE,
                prevState);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_COMM_CHANNEL_NEW_STATE,
                newState);
        service.sendBroadcast(broadcastIntent);
    }
}
