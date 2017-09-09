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

import android.annotation.SuppressLint;
import android.app.Service;
import android.content.Intent;
import android.net.Uri;
import android.os.IBinder;
import android.util.Log;
import android.util.SparseArray;

import com.opel.cmfw.service.CommChannelService;
import com.opel.opel_manager.model.OPELApp;
import com.opel.opel_manager.model.OPELEvent;
import com.opel.opel_manager.model.OPELEventList;
import com.opel.opel_manager.model.Settings;
import com.opel.opel_manager.model.message.AppAckMessage;
import com.opel.opel_manager.model.message.AppCoreAckMessage;
import com.opel.opel_manager.model.message.BaseMessage;
import com.opel.opel_manager.model.message.CompanionMessage;
import com.opel.opel_manager.model.message.params.ParamAppListEntry;
import com.opel.opel_manager.model.message.params.ParamFileListEntry;
import com.opel.opel_manager.model.message.params.ParamsGetAppList;
import com.opel.opel_manager.model.message.params.ParamsGetFileList;
import com.opel.opel_manager.model.message.params.ParamsGetRootPath;
import com.opel.opel_manager.model.message.params.ParamsInitializeApp;
import com.opel.opel_manager.model.message.params.ParamsListenAppState;
import com.opel.opel_manager.model.message.params.ParamsSendConfigPage;
import com.opel.opel_manager.model.message.params.ParamsSendEventPage;
import com.opel.opel_manager.model.message.params.ParamsUpdateAppConfig;
import com.opel.opel_manager.model.message.params.ParamsUpdateSensorData;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class OPELControllerService extends Service {
    private static String TAG = "OPELControllerService";
    private int mBindersCount = 0;

    // TargetDeviceStub
    private final OPELControllerService self = this;
    private TargetDeviceStub mTargetDeviceStub = null;
    private PrivateAppCoreStubListener mTargetDeviceStubListener = null;

    // Models
    @SuppressLint("UseSparseArrays")
    private HashMap<Integer, OPELApp> mAppList = new HashMap<>();
    private OPELEventList mEventList;
    private Settings mSettings;

    public OPELControllerService() {
        this.mEventList = new OPELEventList();
        this.mSettings = new Settings();
    }

    // Connection with target device
    public void initializeConnectionAsync() {
        if (this.mTargetDeviceStub == null) {
            Log.e(TAG, "AppCoreStub is not initialized");
            return;
        }
        this.mTargetDeviceStub.initializeConnection();
    }

    public void destroyConnectionAsync() {
        if (this.mTargetDeviceStub == null) {
            Log.e(TAG, "AppCoreStub is not initialized");
            return;
        }
        this.mTargetDeviceStub.destroyConnection();
    }

    public int getCommChannelState() {
        if (this.mTargetDeviceStub == null) {
            Log.e(TAG, "AppCoreStub is not initialized");
            return CommChannelService.STATE_DISCONNECTED;
        }
        return this.mTargetDeviceStub.getCommChannelState();
    }

    public void enableLargeDataMode() {
        this.mTargetDeviceStub.enableLargeDataMode();
    }

    public void lockLargeDataMode() {
        this.mTargetDeviceStub.lockLargeDataMode();
    }

    public void unlockLargeDataMode() {
        this.mTargetDeviceStub.unlockLargeDataMode();
    }

    public String getLargeDataIPAddress() {
        return this.mTargetDeviceStub.getLargeDataIPAddress();
    }

    // Control functions (Sync)
    public OPELApp getApp(int appId) {
        return this.mAppList.get(appId);
    }

    public ArrayList<OPELApp> getAppList() {
        ArrayList<OPELApp> appList = new ArrayList<>();
        for (Map.Entry<Integer, OPELApp> appListEntry : this.mAppList.entrySet()) {
            OPELApp app = appListEntry.getValue();
            appList.add(app);
        }
        return appList;
    }

    public ArrayList<OPELEvent> getEventList() {
        return this.mEventList.getAllEventArrayList();
    }

    public Settings getSettings() {
        return this.mSettings;
    }

    // Control functions (Async)
    public int updateAppListAsync() {
        return this.mUpdateAppListProcedure.start();
    }

    public int getFileListAsync(String path) {
        return this.mTargetDeviceStub.getFileList(path);
    }

    public int getFileAsync(String path) {
        return this.mTargetDeviceStub.getFile(path);
    }

    public int getTargetRootPathAsync() {
        return this.mTargetDeviceStub.getRootPath();
    }

    public void updateAppConfigAsync(int appId, String legacyData) {
        this.mTargetDeviceStub.updateAppConfig(appId, legacyData);
    }

    // Control functions (OneWay)
    public void installAppOneWay(String packageFilePath) {
        this.mInstallProcedure.start(packageFilePath);
    }

    public void removeAppOneWay(int appId) {
        this.mTargetDeviceStub.removeApp(appId);
    }

    public void launchAppOneWay(int appId) {
        this.mTargetDeviceStub.launchApp(appId);
    }

    public void terminateOneWay(int appId) {
        this.mTargetDeviceStub.terminateApp(appId);
    }

    public void installApkOneWay(File apkFile) {
        // TODO: it is not used now, but to be used in future
        //For Companion type//
        Uri apkUri = Uri.fromFile(apkFile);
        try {
            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.setDataAndType(apkUri, "OPELApp/vnd.android.package-archive");
            startActivity(intent);
        } catch (Exception e) {
            Log.d("OPEL", e.getMessage());
        }
    }

    private UpdateAppListProcedure mUpdateAppListProcedure = new UpdateAppListProcedure();
    private InstallProcedure mInstallProcedure = new InstallProcedure();

    private class UpdateAppListProcedure {
        // TODO: caching app icons
        // WaitingAppTable: <key: Integer commandMessageId, value: OPELApp waitingApp>
        private HashMap<Integer, OPELApp> mWaitingAppList = new HashMap<>();
        private ArrayList<OPELApp> mReadyAppList = new ArrayList<>();

        public int start() {
            if (!this.mWaitingAppList.isEmpty() || !this.mReadyAppList.isEmpty()) {
                Log.e(TAG, "Cannot update app list since previous request did not finish.");
                return -1;
            }
            int messageId = mTargetDeviceStub.getAppList();
            return messageId;
        }

        public void onAckGetAppList(ArrayList<ParamAppListEntry> originalAppList) {
            // Update AppList of OPELControllerService
            this.mWaitingAppList.clear();
            for (ParamAppListEntry entry : originalAppList) {
                int appId = entry.appId;
                String appName = entry.appName;
                boolean isDefaultApp = entry.isDefaultApp;

                // iconFile is not determined!
                OPELApp app = new OPELApp(appId, appName, "", isDefaultApp);

                // Request app icon
                int requestMessageId = mTargetDeviceStub.getAppIcon(app.getAppId());
                this.mWaitingAppList.put(requestMessageId, app);
            }
        }

        public void onAckGetAppIcon(int commandMessageId, String appIconPath) {
            // Check waiting app list
            OPELApp thisApp = this.mWaitingAppList.get(commandMessageId);
            if (thisApp == null) {
                Log.w(TAG, "There is no app " + commandMessageId + " in waiting list.");
                return;
            }

            // Move to icon directory regardless of icon file caching
            File originalIconFile = new File(appIconPath);
            String iconDirPath = mSettings.getIconDir().getAbsolutePath();
            File targetIconFile = new File(iconDirPath, originalIconFile.getName());
            originalIconFile.renameTo(targetIconFile);

            // Set app icon path
            thisApp.setIconImagePath(targetIconFile.getAbsolutePath());

            // Move the app to ready app list
            this.mWaitingAppList.remove(thisApp);
            this.mReadyAppList.add(thisApp);
            if (mWaitingAppList.isEmpty()) {
                this.finish();
            }
        }

        public void finish() {
            // Update app list
            mAppList.clear();
            for (OPELApp app : this.mReadyAppList) {
                mAppList.put(app.getAppId(), app);
            }

            // Notify Listener
            OPELApp[] appListArray = (OPELApp[]) this.mReadyAppList.toArray();
            OPELControllerBroadcastSender.onResultUpdateAppList(self, appListArray);

            // Finalize
            this.mWaitingAppList.clear();
            this.mReadyAppList.clear();
        }
    }

    private class InstallProcedure {
        // One way procedure: no return
        // InitializeTransaction: <key: Integer initializeMessageId, value: String packageFilePath>
        private SparseArray<String> mInitializeTransactions = new SparseArray<>();

        // InstallTransaction: <key: Integer appId, value: String packageFilePath>
        private SparseArray<String> mInstallTransactions = new SparseArray<>();

        public void start(String packageFilePath) {
            // Command 1: initialize app
            int messageId = mTargetDeviceStub.initializeApp();
            this.mInitializeTransactions.put(messageId, packageFilePath);
        }

        public void onInitializedApp(int initializeMessageId, int appId) {
            // Check if there is transaction
            String packageFilePath = this.mInitializeTransactions.get(initializeMessageId);
            if (packageFilePath == null) return;
            this.mInitializeTransactions.remove(initializeMessageId);

            // Check and register the package to app list
            boolean checkRes = this.registerPackageToAppList(appId, packageFilePath);
            if (!checkRes) return;

            // Command 2: listen app state
            mTargetDeviceStub.listenAppState(appId);

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

            // Add install transaction
            mInstallTransactions.put(appId, packageFilePath);

            // Command 3: install app
            mTargetDeviceStub.installApp(appId, packageFile);
        }

        private boolean registerPackageToAppList(int appId, String packageFilePath) {
            try {
                // Unarchive app package file
                String unarchiveDirPath = mSettings.getTempDir().getAbsolutePath();
                Unzip.unzip(packageFilePath, unarchiveDirPath);
                final String kManifestFileName = "manifest.xml";
                File manifestFile = new File(unarchiveDirPath, kManifestFileName);

                // Get app information
                String name = this.getFieldFromManifest(manifestFile.getAbsolutePath(), "label");
                String iconFilePath = this.getFieldFromManifest(manifestFile.getAbsolutePath(),
                        "icon");
                if (name == null || iconFilePath == null) {
                    Log.e(TAG, "Failed to get app information from manifest file.");
                    return false;
                }

                // Move icon file to icon directory
                File iconFile = new File(iconFilePath);
                File newIconFile = new File(mSettings.getIconDir(), iconFile.getName());
                iconFile.renameTo(newIconFile);

                // Clear the unarchive directory
                File unarchiveDir = new File(unarchiveDirPath);
                for (File file : unarchiveDir.listFiles()) {
                    file.delete();
                }

                // Insert to app list
                mAppList.put(appId, new OPELApp(appId, name, iconFilePath, false));
                return true;
            } catch (IOException e) {
                Log.e(TAG, "Failed to get app information from manifest file.");
                e.printStackTrace();
                return false;
            }
        }

        private String getFieldFromManifest(String manifestFilePath, String fieldName) {
            File manifestFile = new File(manifestFilePath);
            try {
                XmlPullParserFactory xmlParserFactory = XmlPullParserFactory.newInstance();
                XmlPullParser xmlParser = xmlParserFactory.newPullParser();
                FileInputStream fileInputStream = new FileInputStream(manifestFile);
                xmlParser.setInput(fileInputStream, null);
                int event = xmlParser.getEventType();
                while (event != XmlPullParser.END_DOCUMENT) {
                    String tagName = xmlParser.getName();
                    if (event == XmlPullParser.START_TAG && tagName.compareTo(fieldName) == 0) {
                        xmlParser.next();
                        return xmlParser.getText();
                    }
                }
                return null;
            } catch (XmlPullParserException e) {
                e.printStackTrace();
                return null;
            } catch (FileNotFoundException e) {
                e.printStackTrace();
                return null;
            } catch (IOException e) {
                e.printStackTrace();
                return null;
            }
        }

        public void onAppStateChanged(int appId, int appState) {
            String packageFilePath = this.mInstallTransactions.get(appId);
            if (packageFilePath != null) {
                if (appState == OPELApp.State_Ready) {
                    // Remove the package file if the install has done
                    this.mInstallTransactions.remove(appId);
                    File packageFile = new File(packageFilePath);
                    packageFile.delete();
                }
            }
        }
    }

    private class PrivateAppCoreStubListener implements TargetDeviceStubListener {
        // TargetDeviceStubListener
        @Override
        public void onCommChannelStateChanged(int prevState, int newState) {
            OPELControllerBroadcastSender.onCommChannelStateChanged(self, prevState, newState);
        }

        @Override
        public void onAckGetAppList(BaseMessage message) {
            // Get parameters
            String iconArchiveFilePath = message.getStoredFilePath();
            AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
            int commandMessageId = payload.getCommandMessageId();
            ParamsGetAppList params = payload.getParamsGetAppList();
            ArrayList<ParamAppListEntry> originalAppList = params.appList;

            // Listeners
            mUpdateAppListProcedure.onAckGetAppList(originalAppList);
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
            mInstallProcedure.onAppStateChanged(appId, appState);
            OPELControllerBroadcastSender.onAppStateChanged(self, appId, appState);
        }

        @Override
        public void onAckGetFileList(BaseMessage message) {
            // Get parameters
            AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
            int commandMessageId = payload.getCommandMessageId();
            ParamsGetFileList params = payload.getParamsGetFileList();
            String path = params.path;
            ParamFileListEntry[] fileList = (ParamFileListEntry[]) params.fileList.toArray();

            // Listeners
            OPELControllerBroadcastSender.onResultGetFileList(self, commandMessageId, path,
                    fileList);
        }

        @Override
        public void onAckGetFile(BaseMessage message) {
            // Get parameters
            AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
            int commandMessageId = payload.getCommandMessageId();
            String storedFileName = message.getStoredFilePath();

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

        @Override
        public void getAppIcon(BaseMessage message) {
            // Get parameters
            AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
            int commandMessageId = payload.getCommandMessageId();
            String iconFilePath = message.getStoredFilePath();

            // Listeners
            mUpdateAppListProcedure.onAckGetAppIcon(commandMessageId, iconFilePath);
        }

        @Override
        public void onUpdateAppConfig(BaseMessage message) {
            // Get parameters
            AppAckMessage payload = (AppAckMessage) message.getPayload();
            int commandMessageId = payload.getCommandMessageId();
            ParamsUpdateAppConfig params = payload.getParamsUpdateAppConfig();
            boolean isSucceed = params.isSucceed;

            // TODO: success!
            // Listeners
            OPELControllerBroadcastSender.onResultUpdateAppConfig(self, commandMessageId,
                    isSucceed);
        }
    }

    // Android Service
    @Override
    public IBinder onBind(Intent intent) {
        this.mBindersCount++;
        if (this.mTargetDeviceStubListener == null)
            this.mTargetDeviceStubListener = new PrivateAppCoreStubListener();
        if (this.mTargetDeviceStub == null) {
            this.mTargetDeviceStub = new TargetDeviceStub(this, this.mTargetDeviceStubListener);
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