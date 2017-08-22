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
import com.opel.opel_manager.model.message.params.ParamAppListEntry;
import com.opel.opel_manager.model.message.params.ParamsGetAppList;
import com.opel.opel_manager.model.message.params.ParamsGetFileList;
import com.opel.opel_manager.model.message.params.ParamsGetRootPath;
import com.opel.opel_manager.model.message.params.ParamsInitializeApp;
import com.opel.opel_manager.model.message.params.ParamsListenAppState;
import com.opel.opel_manager.model.message.params.ParamsSendConfigPage;
import com.opel.opel_manager.model.message.params.ParamsSendEventPage;

import java.io.File;
import java.util.List;

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
        int messageId = this.sendAppCoreMessage(newMessage);
        return messageId;
    }

    public int listenAppState(int appId) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_ListenAppState);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsListenAppState(appId);
        int messageId = this.sendAppCoreMessage(newMessage);
        return messageId;
    }

    public int initializeApp(int appId, String name) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_InitializeApp);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsInitializeApp(appId, name);
        int messageId = this.sendAppCoreMessage(newMessage);
        return messageId;
    }

    public int installApp(int appId, File packageFile) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_InstallApp);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsInstallApp(appId, packageFile.getName());
        newMessage.attachFile(packageFile.getPath());
        int messageId = this.sendAppCoreMessage(newMessage);
        return messageId;
    }

    public int launchApp(int appId) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_LaunchApp);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsLaunchApp(appId);
        int messageId = this.sendAppCoreMessage(newMessage);
        return messageId;
    }

    public int terminateApp(int appId) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_TerminateApp);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsTerminateApp(appId);
        int messageId = this.sendAppCoreMessage(newMessage);
        return messageId;
    }

    public int removeApp(int appId) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_RemoveApp);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsRemoveApp(appId);
        int messageId = this.sendAppCoreMessage(newMessage);
        return messageId;
    }

    public int getFileList(String path) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_GetFileList);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsGetFileList(path);
        int messageId = this.sendAppCoreMessage(newMessage);
        return messageId;
    }

    public int getFile(String path) {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_GetFile);
        AppCoreMessage appCorePayload = (AppCoreMessage) newMessage.getPayload();
        appCorePayload.setParamsGetFile(path);
        int messageId = this.sendAppCoreMessage(newMessage);
        return messageId;
    }

    public int getRootPath() {
        BaseMessage newMessage = MessageFactory.makeAppCoreMessage(kAppCoreURI, AppCoreMessage
                .Type_GetRootPath);
        int messageId = this.sendAppCoreMessage(newMessage);
        return messageId;
    }

    public int updateAppConfig() {
        // TODO: implement it
        return 0;
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
                ParamsGetAppList params = payload.getParamsGetAppList();
                this.mListener.onAckGetAppList(payload.getCommandMessageId(), params.appList);
                break;
            }
            case AppCoreMessage.Type_ListenAppState: {
                ParamsListenAppState params = payload.getParamsListenAppState();
                this.mListener.onAckListenAppState(payload.getCommandMessageId(), params.appId,
                        params.appState);
                break;
            }
            case AppCoreMessage.Type_InitializeApp: {
                ParamsInitializeApp params = payload.getParamsInitializeApp();
                this.mListener.onAckInitializeApp(payload.getCommandMessageId(), params.appId);
                break;
            }
            case AppCoreMessage.Type_GetFileList: {
                ParamsGetFileList params = payload.getParamsGetFileList();
                this.mListener.onAckGetFileList(payload.getCommandMessageId(), params.path,
                        params.fileList);
                break;
            }
            case AppCoreMessage.Type_GetFile: {
                File attachedFile = new File(message.getStoredFileName());
                this.mListener.onAckGetFile(payload.getCommandMessageId(), attachedFile);
                break;
            }
            case AppCoreMessage.Type_GetRootPath: {
                ParamsGetRootPath params = payload.getParamsGetRootPath();
                this.mListener.onAckGetRootPath(payload.getCommandMessageId(), params.rootPath);
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
                ParamsSendConfigPage params = payload.getParamsSendConfigPage();
                this.mListener.onSendConfigPage(message.getMessageId(), params.legacyData);
                break;
            }
            case CompanionMessage.Type_SendEventPage: {
                ParamsSendEventPage params = payload.getParamsSendEventPage();
                this.mListener.onSendEventPage(message.getMessageId(), params.legacyData);
                break;
            }
            // TODO: onUpdateSensorData
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
    public void onAckGetAppList(int commandMessageId, List<ParamAppListEntry> appList);

    public void onAckListenAppState(int commandMessageId, int appId, int appState);

    public void onAckInitializeApp(int commandMessageId, int appId);

    public void onAckGetFileList(int commandMessageId, String path, List<String> fileList);

    public void onAckGetFile(int commandMessageId, File file);

    public void onAckGetRootPath(int commandMessageId, String rootPath);

    // CompanionMessage
    public void onSendEventPage(int messageId, String legacyData);

    public void onSendConfigPage(int messageId, String legacyData);

    public void onUpdateSensorData(int messageId, String legacyData);
}