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

import android.util.Log;

import com.opel.cmfw.service.CommChannelService;
import com.opel.opel_manager.model.message.AppCoreAckMessage;
import com.opel.opel_manager.model.message.AppCoreMessage;
import com.opel.opel_manager.model.message.BaseMessage;
import com.opel.opel_manager.model.message.CompanionMessage;
import com.opel.opel_manager.model.message.MessageFactory;

import java.io.File;

import static android.content.ContentValues.TAG;

public class OPELAppCoreStub {
    private CommChannelService mServiceStub;
    private OPELAppCoreListener mListener;

    private int mCurrentMessageId;
    private final String kAppCoreURI = "/thing/appcore";

    public OPELAppCoreStub(CommChannelService serviceStub, OPELAppCoreListener listener) {
        this.mServiceStub = serviceStub;
        this.mListener = listener;
    }

    // OPEL Manager (Android) -> AppCore Daemon (OPEL Device)
    private int sendAppCoreMessage(BaseMessage message) {
        this.mServiceStub.sendRawMessage(message.toJSONString());
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
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage( kAppCoreURI, AppCoreMessage
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
}

interface OPELAppCoreListener {
    // AppCoreAckMessage
    public void onAckGetAppList(BaseMessage message);

    public void onAckListenAppState(BaseMessage message);

    public void onAckInitializeApp(BaseMessage message);

    public void onAckGetFileList(BaseMessage message);

    public void onAckGetFile(BaseMessage message);

    public void onAckGetRootPath(BaseMessage message);

    // CompanionMessage
    public void onSendEventPage(BaseMessage message);

    public void onSendConfigPage(BaseMessage message);

    public void onUpdateSensorData(BaseMessage message);
}