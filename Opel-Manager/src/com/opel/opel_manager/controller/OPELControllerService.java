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

// TODO: parcellable BaseMessage
// TODO: OPELControllerBroadcastReceiver
// TODO: Convert to OPELControllerService
// TODO: merge OPELContext

public class OPELControllerService extends Service {

    // RPC on CommChannelService
    private CommChannelService mCommChannelServiceStub = null;
    private PrivateCommBroadcastReceiver mCommBroadcastReceiver;
    private final OPELControllerService self = this;

    // TODO: convert Listener to OPELControllerBroadcaster

    private final String kAppCoreURI = "/thing/appcore";

    // OPEL Manager (Android) -> AppCore Daemon (OPEL Device)
    private int sendAppCoreMessage(BaseMessage message) {
        if(this.mCommChannelServiceStub == null) {
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

    // AppCore Daemon (OPEL Device) -> OPEL Manager (Android)
    public void onReceivedMessage(String messageStr, String filePath) {
        BaseMessage message = MessageFactory.makeMessageFromJSONString(messageStr);
        switch (message.getType()) {
            case BaseMessage.Type_AppCoreAck:
                this.onReceivedAppCoreAckMessage(message);
                break;
            case BaseMessage.Type_Companion:
                this.onReceivedCompanionMessage(message);
                break;
            case BaseMessage.Type_App:
            case BaseMessage.Type_AppCore:
            case BaseMessage.Type_NotDetermined:
            default:
                Log.e(TAG, "Cannot receive that kind of message!: " + messageStr);
                break;
        }
    }

    private void onReceivedAppCoreAckMessage(BaseMessage message) {
        AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
        switch (payload.getCommandType()) {
            case AppCoreMessage.Type_GetAppList: {
//                ParamsGetAppList params = payload.getParamsGetAppList();
//                this.mListener.onAckGetAppList(payload.getCommandMessageId(), params.appList);
                this.mListener.onAckGetAppList(message);
                break;
            }
            case AppCoreMessage.Type_ListenAppState: {
//                ParamsListenAppState params = payload.getParamsListenAppState();
//                this.mListener.onAckListenAppState(payload.getCommandMessageId(), params.appId,
//                        params.appState);
                this.mListener.onAckListenAppState(message);
                break;
            }
            case AppCoreMessage.Type_InitializeApp: {
//                ParamsInitializeApp params = payload.getParamsInitializeApp();
//                this.mListener.onAckInitializeApp(payload.getCommandMessageId(), params.appId);
                this.mListener.onAckInitializeApp(message);
                break;
            }
            case AppCoreMessage.Type_GetFileList: {
//                ParamsGetFileList params = payload.getParamsGetFileList();
//                this.mListener.onAckGetFileList(payload.getCommandMessageId(), params.path,
//                        params.fileList);
                this.mListener.onAckGetFileList(message);
                break;
            }
            case AppCoreMessage.Type_GetFile: {
//                File attachedFile = new File(message.getStoredFileName());
//                this.mListener.onAckGetFile(payload.getCommandMessageId(), attachedFile);
                this.mListener.onAckGetFile(message);
                break;
            }
            case AppCoreMessage.Type_GetRootPath: {
//                ParamsGetRootPath params = payload.getParamsGetRootPath();
//                this.mListener.onAckGetRootPath(payload.getCommandMessageId(), params.rootPath);
                this.mListener.onAckGetRootPath(message);
                break;
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
//                ParamsSendConfigPage params = payload.getParamsSendConfigPage();
//                this.mListener.onSendConfigPage(message.getMessageId(), params.legacyData);
                this.mListener.onSendConfigPage(message);
                break;
            }
            case CompanionMessage.Type_SendEventPage: {
//                ParamsSendEventPage params = payload.getParamsSendEventPage();
//                this.mListener.onSendEventPage(message.getMessageId(), params.legacyData);
                this.mListener.onSendEventPage(message);
                break;
            }
            case CompanionMessage.Type_UpdateSensorData: {
//                ParamsUpdateSensorData params = payload.getParamsSendEventPage();
//                this.mListener.onUpdateSensorData(message.getMessageId(), params.legacyData);
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
        // Launch CommChannelService for setting connection with target OPEL device.
        Intent serviceIntent = new Intent(this, CommChannelService.class);
        this.bindService(serviceIntent, this.mCommServiceConnection, Context.BIND_AUTO_CREATE);
    }

    private ServiceConnection mCommServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder inputBinder) {
            CommChannelService.CommBinder serviceBinder = (CommChannelService.CommBinder)
                    inputBinder;
            mCommChannelServiceStub = serviceBinder.getService();

            // Set CommBroadcastReceiver and CommChannelEventListener
            IntentFilter broadcastIntentFilter = new IntentFilter();
            broadcastIntentFilter.addAction(CommBroadcastReceiver.ACTION);
            mCommBroadcastReceiver = new PrivateCommBroadcastReceiver();
            registerReceiver(mCommBroadcastReceiver, broadcastIntentFilter);

            // Request to connect channel
            OPELContext.getAppCore().setCommService(mCommChannelServiceStub);
            mCommChannelServiceStub.connectChannel(); // RPC to CommChannelService
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.d (TAG, "onServiceDisconnected()");
            unregisterReceiver(mCommBroadcastReceiver);
            mCommChannelServiceStub = null;
        }
    };

    class PrivateCommBroadcastReceiver extends CommBroadcastReceiver {
        // CommChannelEventSender
        @Override
        public void onCommChannelStateChanged(int prevState, int newState) {
            // TODO: Add to OPELControllerListener
//            Log.d(TAG, "CommChannel State change: " + prevState + " -> " + newState);
//            switch (newState) {
//                case CommChannelService.STATE_DISCONNECTED:
//                    mDefaultPortIndicator.setDisconnected();
//                    mLargeDataPortIndicator.setDisconnected();
//
//                    switch (prevState) {
//                        case CommChannelService.STATE_CONNECTING_DEFAULT:
//                            Toast.makeText(getApplicationContext(), "Failed connecting to OPEL "
//                                    + "device. Retry it.", Toast.LENGTH_LONG).show();
//
//                            break;
//                        case CommChannelService.STATE_CONNECTED_DEFAULT:
//                        case CommChannelService.STATE_CONNECTING_LARGE_DATA:
//                        case CommChannelService.STATE_CONNECTED_LARGE_DATA:
//                            Toast.makeText(getApplicationContext(), "OPEL device is disconnected" +
//                                    ".", Toast.LENGTH_LONG).show();
//                            break;
//                    }
//                    break;
//                case CommChannelService.STATE_CONNECTING_DEFAULT:
//                    mDefaultPortIndicator.setConnecting();
//                    mLargeDataPortIndicator.setDisconnected();
//                    break;
//                case CommChannelService.STATE_CONNECTED_DEFAULT:
//                    mDefaultPortIndicator.setConnected();
//                    mLargeDataPortIndicator.setDisconnected();
//
//                    switch (prevState) {
//                        case CommChannelService.STATE_CONNECTING_DEFAULT:
//                            Toast.makeText(getApplicationContext(), "OPEL device is connected.",
//                                    Toast.LENGTH_LONG).show();
//                            OPELContext.getAppCore().requestUpdateAppInfomation();
//                            break;
//                        case CommChannelService.STATE_CONNECTING_LARGE_DATA:
//                            Toast.makeText(getApplicationContext(), "Opening large data port is "
//                                    + "failed.", Toast.LENGTH_LONG).show();
//                            break;
//                        case CommChannelService.STATE_CONNECTED_LARGE_DATA:
//                            Toast.makeText(getApplicationContext(), "Large data port is closed.",
//                                    Toast.LENGTH_LONG).show();
//                            break;
//                    }
//                    break;
//                case CommChannelService.STATE_CONNECTING_LARGE_DATA:
//                    mDefaultPortIndicator.setConnected();
//                    mLargeDataPortIndicator.setConnecting();
//                    break;
//                case CommChannelService.STATE_CONNECTED_LARGE_DATA:
//                    mDefaultPortIndicator.setConnected();
//                    mLargeDataPortIndicator.setConnected();
//
//                    Toast.makeText(getApplicationContext(), "Large data port is opened.", Toast
//                            .LENGTH_LONG).show();
//
//                    if (mIsWaitingWifiDirectOnForCamera) {
//                        launchCameraAfterWifiDirectConnected();
//                        mIsWaitingWifiDirectOnForCamera = false;
//                    }
//                    break;
//            }
        }

        @Override
        public void onReceivedRawMessage(String message, String filePath) {
            onReceivedMessage(message, filePath);
        }
    }

    // Android service
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public boolean onUnbind(Intent intent) {

    }
}