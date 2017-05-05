package com.opel.opel_manager.controller;

import android.content.IntentFilter;
import android.net.wifi.p2p.WifiP2pManager;

import com.opel.cmfw.controller.WifiDirectBroadcastReceiver;
import com.opel.opel_manager.model.OPELAppList;
import com.opel.opel_manager.model.OPELEventList;
import com.opel.opel_manager.model.Settings;

public class OPELContext {
    // OPELContext contains objects which belong to Companion device's context.
    // Models
    private Settings mSettings;
    private MainController mMainController;
    private OPELAppList mOPELAppList;
    private OPELEventList mOPELEventList;

    // Controllers
    private OPELDevice mOPELDevice;
    private MQTTController mMQTTController;

    // Wi-fi Direct
    // TODO: move to CommController
    private WifiP2pManager mWifiP2pManager = null;
    private WifiP2pManager.Channel mWifiP2pManagerChannel = null;
    private WifiDirectBroadcastReceiver mWifiDirectBroadcastReceiver = null;
    private IntentFilter mIntentFilter = null;
    private String mWifiDirectIP = "N/A";

    private OPELContext() {
        this.mSettings = new Settings();
        this.mMainController = new MainController();

        this.mOPELDevice = new OPELDevice();
        this.mOPELAppList = new OPELAppList();
        this.mOPELEventList = new OPELEventList();
        this.mMQTTController = new MQTTController();
    }

    static private OPELContext singleton = null;
    private static OPELContext get() {
        if(singleton == null) {
            singleton = new OPELContext();
        }
        return singleton;
    }

    // TODO: refactor getter/setter name
    // Getters/Setters
    public static Settings getSettings() {
        return OPELContext.get().mSettings;
    }

    public static MainController getManagerState() {
        return OPELContext.get().mMainController;
    }

    public static OPELAppList getAppList() {
        return OPELContext.get()
                .mOPELAppList;
    }

    public static OPELEventList getEventList() {
        return OPELContext.get().mOPELEventList;
    }

    public static OPELDevice getOPELDevice() {
        return OPELContext.get().mOPELDevice;
    }

    public static MQTTController getMQTTController() {
        return OPELContext.get().mMQTTController;
    }

    // TODO: move to CommController
    public void setIntentFilter(IntentFilter i) {
        mIntentFilter = i;
    }

    // TODO: move to CommController
    public IntentFilter getIntentFilter() {
        return mIntentFilter;
    }

    // TODO: move to CommController
    public void setWifiP2pManager(WifiP2pManager manager) {
        mWifiP2pManager = manager;
    }

    // TODO: move to CommController
    public WifiP2pManager getWifiP2pManager() {
        return mWifiP2pManager;
    }

    // TODO: move to CommController
    public void setWifiChannel(WifiP2pManager.Channel channel) {
        mWifiP2pManagerChannel = channel;
    }

    // TODO: move to CommController
    public WifiP2pManager.Channel getChannel() {
        return mWifiP2pManagerChannel;
    }

    // TODO: move to CommController
    public void setWifiReceiver(WifiDirectBroadcastReceiver receiver) {
        mWifiDirectBroadcastReceiver = receiver;
    }

    // TODO: move to CommController
    public WifiDirectBroadcastReceiver getWifiReceiver() {
        return mWifiDirectBroadcastReceiver;
    }

    // TODO: move to CommController
    public String getDeviceIP() {
        return this.mWifiDirectIP;
    }

    // TODO: move to CommController
    public void setDeviceIP(String device_ip) {
        this.mWifiDirectIP = device_ip;
    }

    // TODO: move to OPELEventList
//    public void exitApp() {
//        this.mOPELEventList.close();
//    }
}