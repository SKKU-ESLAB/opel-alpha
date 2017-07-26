package com.opel.cmfw.view;

import android.app.Activity;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.wifi.WpsInfo;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pManager;
import android.util.Log;

public class WifiDirectDeviceController {
    static private String TAG = "WFDController";
    private Service mService;
    private String mWifiDirectName;
    private WifiP2pManager mWifiP2pManager = null;
    private WifiP2pManager.Channel mWifiP2pManagerChannel = null;

    private ConnectProcedure mConnectProcedure;
    private State mState;

    public WifiDirectDeviceController(Service service) {
        this.mService = service;

        this.mConnectProcedure = new ConnectProcedure();
        this.mState = new State();

        this.mWifiP2pManager = (WifiP2pManager) this.mService.getSystemService(Activity
                .WIFI_P2P_SERVICE);
        this.mWifiP2pManagerChannel = this.mWifiP2pManager.initialize(this.mService, this
                .mService.getMainLooper(), null);
    }

    public boolean isConnected() {
        return this.mState.isConnected();
    }

    public void connect(ConnectingResultListener connectingResultListener, String wifiDirectName) {
        this.mWifiDirectName = wifiDirectName;

        this.mConnectProcedure.start(connectingResultListener);
    }

    public void disconnect() {
        // TODO: implement it
    }

    interface ConnectingResultListener {
        public void onConnectingWifiDirectDeviceSuccess();

        public void onConnectingWifiDirectDeviceFail();
    }

    private class ConnectProcedure {
        private ConnectingResultListener mConnectingResultListener;

        public void start(ConnectingResultListener connectingResultListener) {
            this.mConnectingResultListener = connectingResultListener;

            this.discoverPeers();
        }

        private void discoverPeers() {
            // Step 1. Discover peers
            IntentFilter wifiP2PIntentFilter;
            wifiP2PIntentFilter = new IntentFilter();
            wifiP2PIntentFilter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
            mService.registerReceiver(new WifiDirectPeerChangedEventReceiver(),
                    wifiP2PIntentFilter);

            // Start Wi-fi direct discovery
            mWifiP2pManager.discoverPeers(mWifiP2pManagerChannel, null);
        }

        class WifiDirectPeerChangedEventReceiver extends BroadcastReceiver {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.compareTo(action) == 0) {
                    onPeerChanged();
                }
            }
        }

        private void onPeerChanged() {
            // Step 2. Request peer list
            mWifiP2pManager.requestPeers(mWifiP2pManagerChannel, new WifiDirectPeerListListener());
        }

        class WifiDirectPeerListListener implements WifiP2pManager.PeerListListener {
            @Override
            public void onPeersAvailable(WifiP2pDeviceList peerDeviceList) {
                onPeerListReceived(peerDeviceList);
            }
        }

        private void onPeerListReceived(WifiP2pDeviceList peerDeviceList) {
            // Step 3. Check if there is peer device that we want
            if (mWifiDirectName.isEmpty()) {
                Log.d(TAG, "Failed to connectChannel! No Wi-fi Direct name given!");
                this.onFail();
                return;
            }

            for (WifiP2pDevice peerDevice : peerDeviceList.getDeviceList()) {
                if (peerDevice.deviceName.compareTo(mWifiDirectName) == 0) {
                    Log.d(TAG, "Found device");
                    if (peerDevice.status == WifiP2pDevice.AVAILABLE) {
                        Log.d(TAG, "Connecting...");
                        requestConnection(peerDevice);
                    } else {
                        this.onFail();
                    }
                    return;
                }
            }
        }

        private void requestConnection(WifiP2pDevice peerDevice) {
            // Step 4. Request for connecting to the peer device
            WifiP2pConfig wifiP2pConfig = new WifiP2pConfig();
            wifiP2pConfig.deviceAddress = peerDevice.deviceAddress;
            if (peerDevice.wpsPbcSupported()) {
                wifiP2pConfig.wps.setup = WpsInfo.PBC;
                Log.d(TAG, "WPS: PBC");
            } else if (peerDevice.wpsKeypadSupported()) {
                wifiP2pConfig.wps.setup = WpsInfo.KEYPAD;
                wifiP2pConfig.wps.pin = "12345670";
                Log.d(TAG, "WPS:KeyPad");
            } else {
                wifiP2pConfig.wps.setup = WpsInfo.DISPLAY;
                Log.d(TAG, "WPS:Display");
            }
            mWifiP2pManager.connect(mWifiP2pManagerChannel, wifiP2pConfig, new WifiP2pManager
                    .ActionListener() {
                @Override
                public void onSuccess() {
                    ConnectProcedure.this.onSuccess();
                }

                @Override
                public void onFailure(int reason) {
                    onFail();
                }
            });
        }

        private void onSuccess() {
            // State transition
            mState.transitToConnected();

            // Notify result
            this.mConnectingResultListener.onConnectingWifiDirectDeviceSuccess();
        }

        private void onFail() {
            // State transition
            mState.transitToDisconnected();

            // Notify result
            this.mConnectingResultListener.onConnectingWifiDirectDeviceFail();
        }
    }

    private class State {
        private WifiDirectDeviceStatusReceiver mWifiDirectDeviceStatusReceiver;

        private boolean mIsConnected = false;

        public boolean isConnected() {
            return this.mIsConnected;
        }

        public void transitToConnected() {
            this.mIsConnected = true;

            // Start to watch Bluetooth device's status
            this.startToWatchDeviceState();

            // Broadcast Wi-fi direct device connection event via CommBroadcaster
            CommBroadcaster.onWifiDirectDeviceStateChanged(mService, this.mIsConnected);
        }

        public void transitToDisconnected() {
            this.mIsConnected = false;

            // Stop to watch Bluetooth device's status
            this.stopToWatchDeviceState();

            // Broadcast Wi-fi direct device disconnection event via CommBroadcaster
            CommBroadcaster.onWifiDirectDeviceStateChanged(mService, this.mIsConnected);
        }

        private void startToWatchDeviceState() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
            this.mWifiDirectDeviceStatusReceiver = new WifiDirectDeviceStatusReceiver();
            mService.registerReceiver(this.mWifiDirectDeviceStatusReceiver, filter);
        }

        private void stopToWatchDeviceState() {
            if (this.mWifiDirectDeviceStatusReceiver != null) {
                mService.unregisterReceiver(this.mWifiDirectDeviceStatusReceiver);
                this.mWifiDirectDeviceStatusReceiver = null;
            }
        }

        // Receive Wi-fi direct device disconnection event from Android Bluetooth framework
        class WifiDirectDeviceStatusReceiver extends BroadcastReceiver {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.compareTo(action) == 0) {
                    NetworkInfo networkInfo = (NetworkInfo) intent.getParcelableExtra
                            (WifiP2pManager.EXTRA_NETWORK_INFO);
                    if (!networkInfo.isConnectedOrConnecting() || !networkInfo.isAvailable()) {
                        transitToDisconnected();
                    }
                }
            }
        }
    }
}