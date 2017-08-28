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
import com.opel.opel_manager.model.message.params.ParamsUpdateSensorData;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class OPELControllerService extends Service {
    private static String TAG = "OPELControllerService";
    private int mBindersCount = 0;

    // OPELAppCoreStub
    private final OPELControllerService self = this;
    private OPELAppCoreStub mAppCoreStub = null;
    private PrivateAppCoreStubListener mAppCoreStubListener = null;

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
        // TODO: make it to async function
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

    public void updateAppConfigOneWay(String legacyData) {
        this.mAppCoreStub.updateAppConfig(legacyData);
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

    private static void unzip(String zipFile, String location) throws IOException {
        try {
            File f = new File(location);
            if (!f.isDirectory()) {
                f.mkdirs();
            }
            ZipInputStream zin = new ZipInputStream(new FileInputStream(zipFile));
            try {
                ZipEntry ze = null;
                while ((ze = zin.getNextEntry()) != null) {
                    String path = location + ze.getName();

                    if (ze.isDirectory()) {
                        File unzipFile = new File(path);
                        if (!unzipFile.isDirectory()) {
                            unzipFile.mkdirs();
                        }
                    } else {
                        FileOutputStream fout = new FileOutputStream(path, false);
                        try {
                            for (int c = zin.read(); c != -1; c = zin.read()) {
                                fout.write(c);
                            }
                            zin.closeEntry();
                        } finally {
                            fout.close();
                        }
                    }
                }
            } finally {
                zin.close();
            }
        } catch (Exception e) {
            Log.e(TAG, "Unzip exception", e);
        }
    }

    private InstallProcedure mInstallProcedure = new InstallProcedure();

    private class InstallProcedure {
        // TODO: make it to async function
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

            // Check and register the package to app list
            boolean checkRes = this.registerPackageToAppList(appId, packageFilePath);
            if (!checkRes) return;

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

        private boolean registerPackageToAppList(int appId, String packageFilePath) {
            try {
                // Unarchive app package file
                String unarchiveDirPath = mSettings.getTempDir().getAbsolutePath();
                unzip(packageFilePath, unarchiveDirPath);
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
    }

    private class PrivateAppCoreStubListener implements OPELAppCoreStubListener {
        // OPELAppCoreStubListener
        @Override
        public void onCommChannelStateChanged(int prevState, int newState) {
            OPELControllerBroadcastSender.onCommChannelStateChanged(self, prevState, newState);
        }

        @Override
        public void onAckGetAppList(BaseMessage message) {
            // Get parameters
            String iconArchiveFilePath = message.getStoredFileName();
            AppCoreAckMessage payload = (AppCoreAckMessage) message.getPayload();
            int commandMessageId = payload.getCommandMessageId();
            ParamsGetAppList params = payload.getParamsGetAppList();
            ArrayList<ParamAppListEntry> originalAppList = params.appList;

            // Update AppList of OPELControllerService
            ArrayList<OPELApp> appArrayList = new ArrayList<>();

            // unarchive icon archive file
            String iconDirPath = mSettings.getIconDir().getAbsolutePath();
            try {
                unzip(iconArchiveFilePath, iconDirPath);
            } catch (IOException e) {
                e.printStackTrace();
                return;
            }

            for (ParamAppListEntry entry : originalAppList) {
                int appId = entry.appId;
                String appName = entry.appName;
                boolean isDefaultApp = entry.isDefaultApp;
                File iconFile = new File(iconDirPath, entry.appId + ".png");

                OPELApp app = new OPELApp(appId, appName, iconFile.getAbsolutePath(), isDefaultApp);

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