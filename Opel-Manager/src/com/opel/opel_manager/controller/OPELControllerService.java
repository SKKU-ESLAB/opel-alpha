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
import com.opel.opel_manager.model.OPELEvent;
import com.opel.opel_manager.model.OPELEventList;
import com.opel.opel_manager.model.Settings;
import com.opel.opel_manager.model.message.AppCoreAckMessage;
import com.opel.opel_manager.model.message.BaseMessage;
import com.opel.opel_manager.model.message.CompanionMessage;
import com.opel.opel_manager.model.message.params.ParamAppListEntry;
import com.opel.opel_manager.model.message.params.ParamsGetAppList;
import com.opel.opel_manager.model.message.params.ParamsGetFileList;
import com.opel.opel_manager.model.message.params.ParamsGetRootPath;
import com.opel.opel_manager.model.message.params.ParamsInitializeApp;
import com.opel.opel_manager.model.message.params.ParamsListenAppState;
import com.opel.opel_manager.model.message.params.ParamsSendConfigPage;
import com.opel.opel_manager.model.message.params.ParamsSendEventPage;
import com.opel.opel_manager.model.message.params.ParamsUpdateSensorData;

import java.io.File;
import java.util.ArrayList;

public class OPELControllerService extends Service {
    private static String TAG = "OPELControllerService";
    private int mBindersCount = 0;

    // OPELAppCoreStub
    private final OPELControllerService self = this;
    private OPELAppCoreStub mAppCoreStub = null;
    private PrivateAppCoreStubListener mAppCoreStubListener = null;

    // Models
    private SparseArray<OPELApp> mAppList = new SparseArray<>(); // TODO: add/remove
    private OPELEventList mEventList; // TODO: initialize
    private Settings mSettings; // TODO: initialize

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
        return this.mAppCoreStub.getLargeDataIPAddress();
    }

    // Control functions (Sync)
    public OPELApp getApp(int appId) {
        return this.mAppList.get(appId);
    }

    public ArrayList<OPELEvent> getEventList() {
        return this.mEventList.getAllEventArrayList();
    }

    // Control functions (Async)
    public int updateAppListAsync() {
        return this.mAppCoreStub.getAppList();
    }

    public int getFileListAsync(String path) {
        return this.mAppCoreStub.getFileList(path);
    }

    public int getFileAsync(String path) {
        return this.mAppCoreStub.getFile(path);
    }

    public int getTargetRootPathAsync() {
        return this.mAppCoreStub.getRootPath();
    }

    // Control functions (OneWay)
    public void installAppOneWay(String packageFilePath) {
        this.mInstallProcedure.start(packageFilePath);
    }

    public void removeAppOneWay(int appId) {
        this.mAppCoreStub.removeApp(appId);
    }

    public void launchAppOneWay(int appId) {
        this.mAppCoreStub.launchApp(appId);
    }

    public void terminateOneWay(int appId) {
        this.mAppCoreStub.terminateApp(appId);
    }

    private InstallProcedure mInstallProcedure = new InstallProcedure();

    private class InstallProcedure {
        // One way procedure: no results are produced
        // InitializeTransaction: <key: Integer initializeMessageId, value: String packageFilePath>
        private SparseArray<String> mInitializeTransactions = new SparseArray<String>();

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
    }

    class PrivateAppCoreStubListener implements OPELAppCoreStubListener {
        // OPELAppCoreStubListener
        @Override
        public void onCommChannelStateChanged(int prevState, int newState) {
            OPELControllerBroadcastSender.onCommChannelStateChanged(self, prevState, newState);
        }

        @Override
        public void onAckGetAppList(BaseMessage message) {
            // Get parameters
            AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
            int commandMessageId = payload.getCommandMessageId();
            ParamsGetAppList params = payload.getParamsGetAppList();
            ArrayList<ParamAppListEntry> originalAppList = params.appList;

            // Update AppList of OPELControllerService
            ArrayList<OPELApp> appArrayList = new ArrayList<>();

            // TODO: unarchive icon archive file
            // TODO: move the files to icon directory
            // TODO: use the icon path
            for (ParamAppListEntry entry : originalAppList) {
                int appId = entry.appId;
                String appName = entry.appName;
                boolean isDefaultApp = entry.isDefaultApp;
                String iconPath = entry.appId + ".png"; // TODO: unfold hardcoding

                OPELApp app = new OPELApp(appId, appName, iconPath, isDefaultApp);

                mAppList.put(appId, app);
                appArrayList.add(app);
            }
            OPELApp[] appList = (OPELApp[]) appArrayList.toArray();

            OPELControllerBroadcastSender.onResultUpdateAppList(self, commandMessageId, appList);
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
            AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
            ParamsListenAppState params = payload.getParamsListenAppState();
            int appId = params.appId;
            int appState = params.appState;

            // Listeners
            mAppList.get(appId).setState(appState);
            OPELControllerBroadcastSender.onAppStateChanged(self, appId, appState);
        }

        @Override
        public void onAckGetFileList(BaseMessage message) {
            // Get parameters
            AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
            int commandMessageId = payload.getCommandMessageId();
            ParamsGetFileList params = payload.getParamsGetFileList();
            String path = params.path;
            String[] fileList = (String[]) params.fileList.toArray();

            // Listeners
            OPELControllerBroadcastSender.onResultGetFileList(self, commandMessageId, path,
                    fileList);
        }

        @Override
        public void onAckGetFile(BaseMessage message) {
            // Get parameters
            AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
            int commandMessageId = payload.getCommandMessageId();
            String storedFileName = message.getStoredFileName();

            // Listeners
            OPELControllerBroadcastSender.onResultGetFile(self, commandMessageId, storedFileName);
        }

        @Override
        public void onAckGetRootPath(BaseMessage message) {
            // Get parameters
            AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
            int commandMessageId = payload.getCommandMessageId();
            ParamsGetRootPath params = payload.getParamsGetRootPath();
            String rootPath = params.rootPath;

            // Listeners
            OPELControllerBroadcastSender.onResultGetTargetRootPath(self, commandMessageId,
                    rootPath);
        }

        @Override
        public void onSendEventPage(BaseMessage message) {
            // Get parameters
            CompanionMessage payload = (CompanionMessage) message.getPayload();
            ParamsSendEventPage params = payload.getParamsSendEventPage();
            String legacyData = params.legacyData;

            // Listeners
            OPELControllerBroadcastSender.onReceivedEvent(self, legacyData);
        }

        @Override
        public void onSendConfigPage(BaseMessage message) {
            // Get parameters
            CompanionMessage payload = (CompanionMessage) message.getPayload();
            ParamsSendConfigPage params = payload.getParamsSendConfigPage();
            String legacyData = params.legacyData;

            // Listeners
            OPELControllerBroadcastSender.onReceivedAppConfig(self, legacyData);
        }

        @Override
        public void onUpdateSensorData(BaseMessage message) {
            // Get parameters
            CompanionMessage payload = (CompanionMessage) message.getPayload();
            ParamsUpdateSensorData params = payload.getParamsUpdateSensorData();
            String legacyData = params.legacyData;

            // Listeners
            OPELControllerBroadcastSender.onReceivedSensorData(self, legacyData);
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