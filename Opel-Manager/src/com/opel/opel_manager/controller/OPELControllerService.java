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
import android.os.IBinder;
import android.util.Log;
import android.util.SparseArray;

import com.opel.cmfw.service.CommChannelService;
import com.opel.opel_manager.model.OPELApp;
import com.opel.opel_manager.model.message.AppCoreAckMessage;
import com.opel.opel_manager.model.message.BaseMessage;
import com.opel.opel_manager.model.message.params.ParamsInitializeApp;
import com.opel.opel_manager.model.message.params.ParamsListenAppState;

import java.io.File;
import java.util.ArrayList;

// TODO: merge OPELContext

public class OPELControllerService extends Service {
    private static String TAG = "OPELControllerService";
    private int mBindersCount = 0;

    private final OPELControllerService self = this;
    private OPELAppCoreStub mAppCoreStub = null;
    private PrivateAppCoreStubListener mAppCoreStubListener = null;

    // Connection with target device
    public void initializeConnectionAsync() {
        if (this.mAppCoreStub == null) {
            Log.e(TAG, "AppCoreStub is not initialized");
            return;
        }
        this.mAppCoreStub.initializeConnection();
    }

    public void destroyConnectionAsync() {
        if (this.mAppCoreStub == null) {
            Log.e(TAG, "AppCoreStub is not initialized");
            return;
        }
        this.mAppCoreStub.destroyConnection();
    }

    public int getCommChannelState() {
        if (this.mAppCoreStub == null) {
            Log.e(TAG, "AppCoreStub is not initialized");
            return CommChannelService.STATE_DISCONNECTED;
        }
        return this.mAppCoreStub.getCommChannelState();
    }

    public void enableLargeDataMode() {
        this.mAppCoreStub.enableLargeDataMode();
    }

    public void lockLargeDataMode() {
        this.mAppCoreStub.lockLargeDataMode();
    }

    public void unlockLargeDataMode() {
        this.mAppCoreStub.unlockLargeDataMode();
    }

    public String getLargeDataIPAddress() {
        this.mAppCoreStub.getLargeDataIPAddress();
    }

    // Controlling functions
    // TODO: use it
    public int getAppListAsync() {
        return this.mAppCoreStub.getAppList();
    }

    // TODO: use it
    public void installAppAsync(String packageFilePath) {
        this.mInstallProcedure.start(packageFilePath);
    }

    // TODO: use it
    public void removeAppAsync(int appId) {
        this.mAppCoreStub.removeApp(appId);
    }

    // TODO: use it
    public void launchAppAsync(int appId) {
        this.mAppCoreStub.launchApp(appId);
    }

    // TODO: use it
    public void terminateAppAsync(int appId) {
        this.mAppCoreStub.terminateApp(appId);
    }

    // TODO: use it
    public int getFileListAsync(String path) {
        return this.mAppCoreStub.getFileList(path);
    }

    // TODO: use it
    public int getFileAsync(String path) {
        return this.mAppCoreStub.getFile(path);
    }

    // TODO: use it
    public int getTargetRootPathAsync() {
        return this.mAppCoreStub.getRootPath();
    }

    // TODO: use it
    public int updateAppConfigAsync(String legacyData) {
        return this.mAppCoreStub.updateAppConfig(legacyData);
    }

    private InstallProcedure mInstallProcedure = new InstallProcedure();

    private class InstallProcedure {
        // InitializeTransaction: <key: Integer initializeMessageId, value: String packageFilePath>
        private SparseArray<String> mInitializeTransactions = new SparseArray<String>();
        // ListenStateTransactions: <Integer appId>
        private ArrayList<Integer> mListenStateTransactions = new ArrayList<>();

        public void start(String packageFilePath) {
            // Command 1: initialize app
            int messageId = mAppCoreStub.initializeApp();
            this.mInitializeTransactions.put(messageId, packageFilePath);
        }

        public void onInitializedApp(int initializeMessageId, int appId) {
            // Check if there is transaction
            String packageFilePath = this.mInitializeTransactions.get(initializeMessageId);
            if (packageFilePath == null) return;
            this.mInitializeTransactions.remove(initializeMessageId);

            // Command 2: listen app state
            mAppCoreStub.listenAppState(appId);
            mListenStateTransactions.add(appId);

            // Check package file
            File packageFile = new File(packageFilePath);
            if (!packageFile.exists()) {
                Log.e(TAG, "App package file does not exist!");
                return;
            }
            if (!packageFile.isFile()) {
                Log.e(TAG, "Package file path does not indicate a file!");
                return;
            }

            // Command 3: install app
            mAppCoreStub.installApp(appId, packageFile);
        }

        // Called by onAckListenAppState
        public void onListenedAppState(int appId, int appState) {
            // Check if there is transaction
            boolean isListening = false;
            for (Integer transactionAppId : this.mListenStateTransactions) {
                if (transactionAppId == appId) {
                    isListening = true;
                    break;
                }
            }

            if (isListening) {
                switch (appState) {
                    case OPELApp.State_Installing:
                        onProcessing(appId);
                        break;
                    case OPELApp.State_Ready:
                        onSuccess(appId);
                        break;
                    case OPELApp.State_Removed:
                        onFail(appId);
                        break;
                }
            }
        }

        private void onProcessing(int appId) {
            // TODO: OPELControllerBroadcastSender.onResultInstallApp(onProcessing)
        }

        // TODO: Called by onAckListenAppState
        private void onSuccess(int appId) {
            // TODO: OPELControllerBroadcastSender.onResultInstallApp(success)
        }

        // TODO: Called by onAckListenAppState
        private void onFail(int appId) {
            // TODO: OPELControllerBroadcastSender.onResultInstallApp(fail)
        }
    }

    class PrivateAppCoreStubListener implements OPELAppCoreStubListener {
        // OPELAppCoreStubListener
        @Override
        public void onCommChannelStateChanged(int prevState, int newState) {
            OPELControllerBroadcastSender.onCommChannelStateChanged(self, prevState, newState);
        }

        @Override
        public void onAckGetAppList(BaseMessage message) {
            // TODO: OPELControllerBroadcastSender.onResultGetAppList()
        }

        @Override
        public void onAckInitializeApp(BaseMessage message) {
            // Get parameters
            int messageId = message.getMessageId();
            AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
            ParamsInitializeApp params = payload.getParamsInitializeApp();
            int appId = params.appId;

            // Listeners
            mInstallProcedure.onInitializedApp(messageId, appId);
        }

        @Override
        public void onAckListenAppState(BaseMessage message) {
            // Get parameters
            int messageId = message.getMessageId();
            AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
            ParamsListenAppState params = payload.getParamsListenAppState();
            int appId = params.appId;
            int appState = params.appState;

            // Listeners
            mInstallProcedure.onListenedAppState(appId, appState);
        }

        @Override
        public void onAckGetFileList(BaseMessage message) {
            // TODO: OPELControllerBroadcastSender.onResultGetFileList()
        }

        @Override
        public void onAckGetFile(BaseMessage message) {
            // TODO: OPELControllerBroadcastSender.onResultGetFile()
        }

        @Override
        public void onAckGetRootPath(BaseMessage message) {
            // TODO: OPELControllerBroadcastSender.onResultGetRootPath()
        }

        @Override
        public void onSendEventPage(BaseMessage message) {
            // TODO: OPELControllerBroadcastSender.onSendEventPage()
        }

        @Override
        public void onSendConfigPage(BaseMessage message) {
            // TODO: OPELControllerBroadcastSender.onSendConfigPage()
        }

        @Override
        public void onUpdateSensorData(BaseMessage message) {
            // TODO: OPELControllerBroadcastSender.onUpdateSensorData()
        }
    }

    // Android Service
    @Override
    public IBinder onBind(Intent intent) {
        this.mBindersCount++;
        if (this.mAppCoreStubListener == null)
            this.mAppCoreStubListener = new PrivateAppCoreStubListener();
        if (this.mAppCoreStub == null) {
            this.mAppCoreStub = new OPELAppCoreStub(this, this.mAppCoreStubListener);
        }
        return null;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        this.mBindersCount--;
        return false;
    }

    // Binder (for the caller of CommChannelService)
    public class ControllerBinder extends android.os.Binder {
        public OPELControllerService getService() {
            return OPELControllerService.this;
        }
    }
}