package com.opel.opel_manager.controller;

import android.content.IntentFilter;
import android.net.wifi.p2p.WifiP2pManager;

import com.opel.opel_manager.controller.selectiveconnection.WifiDirectBroadcastReceiver;
import com.opel.opel_manager.model.OPELAppList;
import com.opel.opel_manager.model.OPELEventList;

import java.io.File;

public class GlobalContext {

    private boolean isInit;
    private boolean isLoading;

    private CommManager mCommManager = new CommManager();
    private OPELAppList mOPELAppList = new OPELAppList();
    private OPELEventList mOPELEventList = new OPELEventList();
    private com.opel.opel_manager.controller.MQTTManager mMQTTManager = new
            MQTTManager();

    /* esevan */
    private WifiP2pManager mManager = null;
    private WifiP2pManager.Channel mChannel = null;
    private WifiDirectBroadcastReceiver mReceiver = null;
    private IntentFilter mIntentFilter = null;

    private String device_ip = "N/A";

    private File opelDirectory;
    private File opelIconDirectory;
    private File opelRemoteUIDirectory;
    private File opelRemoteStorageDirectory;
    private File opelCloudServiceDirectory;

    public MQTTManager getMQTTManager() {    //ES Conflict
        return this.mMQTTManager;
    }

    public void setIntentFilter(IntentFilter i) {
        mIntentFilter = i;
    }

    public IntentFilter getIntentFilter() {
        return mIntentFilter;
    }

    public void setWifiP2pManager(WifiP2pManager manager) {
        mManager = manager;
    }

    public WifiP2pManager getWifiP2pManager() {
        return mManager;
    }

    public void setWifiChannel(WifiP2pManager.Channel channel) {
        mChannel = channel;
    }

    public WifiP2pManager.Channel getChannel() {
        return mChannel;
    }

    public void setWifiReceiver(WifiDirectBroadcastReceiver receiver) {
        mReceiver = receiver;
    }

    public WifiDirectBroadcastReceiver getWifiReceiver() {
        return mReceiver;
    }

    public String getDeviceIP() {
        return this.device_ip;
    }

    public void setDeviceIP(String device_ip) {
        this.device_ip = device_ip;
    }

    public void setOpelStoragePath(File f) {
        this.opelDirectory = f;
    }

    private String mOpelDataDir;

    public String getOpelDataDir() {
        return this.mOpelDataDir;
    }

    public void setOpelDataDir(String opelDataDir) {
        this.mOpelDataDir = opelDataDir;
    }

    public File getOpelStoragePath() {
        return opelDirectory;
    }

    public void setRUIStoragePath(File f) {
        this.opelRemoteUIDirectory = f;
    }

    public File getRUIStoragePath() {
        return opelRemoteUIDirectory;
    }

    public void setIconDirectoryPath(File f) {
        this.opelIconDirectory = f;
    }

    public File getIconDirectoryPath() {
        return opelIconDirectory;
    }

    public void setCloudStoragePath(File f) {
        this.opelCloudServiceDirectory = f;
    }

    public File getCloudStoragePath() {
        return opelCloudServiceDirectory;
    }


    public File getRemoteStorageStoragePath() {
        return opelRemoteStorageDirectory;
    }

    public void setRemoteStorageStoragePath(File f) {
        this.opelRemoteStorageDirectory = f;
    }

    public boolean getIsLoading() {
        return this.isLoading;
    }

    public void setIsLoading(boolean b) {
        this.isLoading = b;
    }

    public OPELAppList getAppList() {
        return mOPELAppList;
    }

    public CommManager getCommManager() {
        return this.mCommManager;
    }

    public OPELEventList getEventList() {
        return this.mOPELEventList;
    }


    private GlobalContext() {
        isInit = false;
        isLoading = false;
    }

    public boolean isInit() {
        return this.isInit;
    }

    public void initComplete() {
        this.isInit = true;
    }

    private volatile static GlobalContext instance = null;

    public static GlobalContext get() {
        if (instance == null) {
            synchronized (GlobalContext.class) {
                if (instance == null) {
                    instance = new GlobalContext();
                }
            }
        }
        return instance;
    }

    public void exitApp() {
        this.mOPELEventList.close();
    }
}