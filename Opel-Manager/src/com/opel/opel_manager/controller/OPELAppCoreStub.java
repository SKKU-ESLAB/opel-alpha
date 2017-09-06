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
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.util.Log;

import com.opel.cmfw.glue.CommBroadcastReceiver;
import com.opel.cmfw.service.CommChannelService;
import com.opel.opel_manager.model.message.AppCoreAckMessage;
import com.opel.opel_manager.model.message.AppCoreMessage;
import com.opel.opel_manager.model.message.BaseMessage;
import com.opel.opel_manager.model.message.CompanionMessage;
import com.opel.opel_manager.model.message.MessageFactory;

import java.io.File;

import static android.content.ContentValues.TAG;

public class OPELAppCoreStub {
    // RPC on CommChannelService
    private CommChannelService mCommChannelServiceStub = null;
    private PrivateCommBroadcastReceiver mCommBroadcastReceiver;

    private final String kAppCoreURI = "/thing/appcore";

    // OPEL Manager (Android) -> AppCore Daemon (OPEL Device)
    private int sendAppCoreMessage(BaseMessage message) {
        if (this.mCommChannelServiceStub == null) {
            Log.e(TAG, "CommChannel is not initialized");
            return -1;
        }
        this.mCommChannelServiceStub.sendRawMessage(message.toJSONString());
        return message.getMessageId();
    }

    public int getAppList() {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_GetAppList);
        return this.sendAppCoreMessage(newMessage);
    }

    public int listenAppState(int appId) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_ListenAppState);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsListenAppState(appId);
        return this.sendAppCoreMessage(newMessage);
    }

    public int initializeApp() {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_InitializeApp);
        return this.sendAppCoreMessage(newMessage);
    }

    public int installApp(int appId, File packageFile) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_InstallApp);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsInstallApp(appId, packageFile.getName());
        newMessage.attachFile(packageFile.getPath());
        return this.sendAppCoreMessage(newMessage);
    }

    public int launchApp(int appId) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_LaunchApp);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsLaunchApp(appId);
        return this.sendAppCoreMessage(newMessage);
    }

    public int terminateApp(int appId) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_TerminateApp);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsTerminateApp(appId);
        return this.sendAppCoreMessage(newMessage);
    }

    public int removeApp(int appId) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_RemoveApp);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsRemoveApp(appId);
        return this.sendAppCoreMessage(newMessage);
    }

    public int getFileList(String path) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_GetFileList);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsGetFileList(path);
        return this.sendAppCoreMessage(newMessage);
    }

    public int getFile(String path) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_GetFile);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsGetFile(path);
        return this.sendAppCoreMessage(newMessage);
    }

    public int getRootPath() {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_GetRootPath);
        return this.sendAppCoreMessage(newMessage);
    }

    public int updateAppConfig(String legacyData) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_UpdateAppConfig);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsGetFile(legacyData);
        return this.sendAppCoreMessage(newMessage);
    }

    public int getAppIcon(int appId) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_GetAppIcon);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsGetAppIcon(appId);
        return this.sendAppCoreMessage(newMessage);
    }

    // AppCore Daemon (OPEL Device) -> OPEL Manager (Android)
    private void onReceivedAppCoreAckMessage(BaseMessage message) {
        AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
        switch (payload.getCommandType()) {
            case AppCoreMessage.Type_GetAppList: {
                this.mListener.onAckGetAppList(message);
                break;
            }
            case AppCoreMessage.Type_ListenAppState: {
                this.mListener.onAckListenAppState(message);
                break;
            }
            case AppCoreMessage.Type_InitializeApp: {
                this.mListener.onAckInitializeApp(message);
                break;
            }
            case AppCoreMessage.Type_GetFileList: {
                this.mListener.onAckGetFileList(message);
                break;
            }
            case AppCoreMessage.Type_GetFile: {
                this.mListener.onAckGetFile(message);
                break;
            }
            case AppCoreMessage.Type_GetRootPath: {
                this.mListener.onAckGetRootPath(message);
                break;
            }
            case AppCoreMessage.Type_GetAppIcon: {
                this.mListener.getAppIcon(message);
            }
            case AppCoreMessage.Type_NotDetermined:
            default: {
                Log.e(TAG, "Cannot receive that command!: " + message.toJSONString());
                break;
            }
        }
    }

    private void onReceivedCompanionMessage(BaseMessage message) {
        CompanionMessage payload = (CompanionMessage) message.getPayload();
        switch (payload.getCommandType()) {
            case CompanionMessage.Type_SendConfigPage: {
                this.mListener.onSendConfigPage(message);
                break;
            }
            case CompanionMessage.Type_SendEventPage: {
                this.mListener.onSendEventPage(message);
                break;
            }
            case CompanionMessage.Type_UpdateSensorData: {
                this.mListener.onUpdateSensorData(message);
            }
            case CompanionMessage.Type_NotDetermined:
            default: {
                Log.e(TAG, "Cannot receive that command!: " + message.toJSONString());
                break;
            }
        }
    }

    // Interface to CommChannelService
    public void initializeConnection() {
        if (this.mCommChannelServiceStub == null) {
            // Launch CommChannelService for setting connection with target OPEL device.
            Intent serviceIntent = new Intent(this.mOwnerService, OPELControllerService.class);
            this.mOwnerService.bindService(serviceIntent, this.mCommServiceConnection, Context
                    .BIND_AUTO_CREATE);
        } else {
            // Request to connect channel
            mCommChannelServiceStub.connectChannel(); // RPC to CommChannelService
        }
    }

    public void destroyConnection() {
        this.mOwnerService.unbindService(mCommServiceConnection);
    }

    public int getCommChannelState() {
        if (this.mCommChannelServiceStub == null) {
            Log.e(TAG, "CommChannelService is not connected");
            return CommChannelService.STATE_DISCONNECTED;
        }
        return this.mCommChannelServiceStub.getCommChannelState();
    }

    public void enableLargeDataMode() {
        this.mCommChannelServiceStub.enableLargeDataMode();
    }

    public void lockLargeDataMode() {
        this.mCommChannelServiceStub.lockLargeDataMode();
    }

    public void unlockLargeDataMode() {
        this.mCommChannelServiceStub.unlockLargeDataMode();
    }

    public String getLargeDataIPAddress() {
        return this.mCommChannelServiceStub.getLargeDataIPAddress();
    }

    private ServiceConnection mCommServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder inputBinder) {
            CommChannelService.CommBinder serviceBinder = (CommChannelService.CommBinder)
                    inputBinder;
            mCommChannelServiceStub = serviceBinder.getService();

            // Set BroadcastReceiver
            IntentFilter broadcastIntentFilter = new IntentFilter();
            broadcastIntentFilter.addAction(CommBroadcastReceiver.ACTION);
            mCommBroadcastReceiver = new PrivateCommBroadcastReceiver();
            mOwnerService.registerReceiver(mCommBroadcastReceiver, broadcastIntentFilter);

            // Request to connect channel
            mCommChannelServiceStub.connectChannel(); // RPC to CommChannelService
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.d(TAG, "onServiceDisconnected()");
            mOwnerService.unregisterReceiver(mCommBroadcastReceiver);
            mCommChannelServiceStub = null;
        }
    };

    class PrivateCommBroadcastReceiver extends CommBroadcastReceiver {
        @Override
        public void onCommChannelStateChanged(int prevState, int newState) {
            mListener.onCommChannelStateChanged(prevState, newState);
        }

        // AppCore Daemon (OPEL Device) -> OPEL Manager (Android)
        @Override
        public void onReceivedRawMessage(String messageStr, String filePath) {
            BaseMessage message = MessageFactory.makeMessageFromJSONString(messageStr);
            switch (message.getType()) {
                case BaseMessage.Type_AppCoreAck:
                    onReceivedAppCoreAckMessage(message);
                    break;
                case BaseMessage.Type_Companion:
                    onReceivedCompanionMessage(message);
                    break;
                case BaseMessage.Type_App:
                case BaseMessage.Type_AppCore:
                case BaseMessage.Type_NotDetermined:
                default:
                    Log.e(TAG, "Cannot receive that kind of message!: " + messageStr);
                    break;
            }
        }
    }

    public OPELAppCoreStub(Service ownerService, OPELAppCoreStubListener listener) {
        this.mOwnerService = ownerService;
        this.mListener = listener;
    }

    private Service mOwnerService;
    private OPELAppCoreStubListener mListener;
}
