package com.opel.cmfw.view;

import android.app.Activity;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.wifi.WpsInfo;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pGroup;
import android.net.wifi.p2p.WifiP2pInfo;
import android.net.wifi.p2p.WifiP2pManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.os.ResultReceiver;
import android.util.Log;

import com.opel.cmfw.controller.BluetoothCommPort;
import com.opel.cmfw.controller.CMFWLegacy;
import com.opel.cmfw.controller.CommPortListener;
import com.opel.cmfw.controller.WifiDirectBroadcastReceiver;
import com.opel.cmfw.controller.WifiDirectCommPort;
import com.opel.cmfw.controller.WifiDirectListener;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.util.Set;
import java.util.UUID;

/* OPEL Manager's Communication Path
    - OPEL Manager Internal -> CommChannelService(main thread) -> OPEL device's CMFW
    - OPEL device's CMFW -> CommChannelService(listening thread) -> OPEL Manager Internal
 */

/* Threads of CommChannelService
    - main thread: send messages that are coming from OPEL Manager internal to CMFWLegacy
    - listening thread: poll messages from CMFWLegacy and pass them to OPEL Manager internal
*/

/* Roles of CommChannelService
    - Initialize/disconnectChannel Bluetooth, Wi-fi Direct device (Bluetooth/WifiDirectDeviceController)
    - State Management of Bluetooth, Wi-fi Direct device (Bluetooth/WifiDirectDeviceController)
    - Transfer data via Bluetooth or Wi-fi Direct (CMFWLegacy)
 */

public class CommChannelService extends Service implements CommPortListener {
    private static final String TAG = "CommChannelService";
    private final CommChannelService self = this;
    private final IBinder mBinder = new CommBinder();

    // Device Controller
    private BluetoothDeviceController mBluetoothDeviceController = null;
    private WifiDirectDeviceController mWifiDirectDeviceController = null;

    // Ports
    private BluetoothCommPort mDefaultPort;
    private BluetoothCommPort mControlPort;
    private WifiDirectCommPort mLargeDataPort;

    // Connection state
    private boolean mIsChannelConnected = false;

    // Download path
    private String mDownloadFilePath;

    private int mBindersCount = 0;

    public void setDownloadFilePath(String downloadFilePath) {
        this.mDownloadFilePath = downloadFilePath;
    }

    // Connect CommChannelService = Connect Bluetooth Device + Open Default Port
    public void connectChannel() {
        // (Connect) Step 1. Connect Bluetooth Device
        this.mBluetoothDeviceController.connect();
    }

    protected void onConnectingBluetoothDeviceSuccess(BluetoothDevice bluetoothDevice) {
        // (Connect) Step 2. Open Port
        boolean isOpeningSuccess = this.mDefaultPort.open();

        // Connect the Bluetooth device
        if (isOpeningSuccess) {
            // Handle Channel connection
            this.onChannelConnected();
        } else {
            this.onChannelDisconnected();
        }
    }

    protected void onConnectingBluetoothDeviceFail() {
        // Handle Bluetooth disconnection
        this.onChannelDisconnected();
    }

    // Disconnection = Disconnect all devices + close all ports
    public void disconnectChannel() {
        // Close ports
        if (this.mDefaultPort != null)
            this.mDefaultPort.close();
        if (this.mControlPort != null)
            this.mControlPort.close();
        if (this.mLargeDataPort != null)
            this.mLargeDataPort.close();

        // Disconnect devices
        if (this.mBluetoothDeviceController != null)
            this.mBluetoothDeviceController.disconnect();
        if (this.mWifiDirectDeviceController != null)
            this.mWifiDirectDeviceController.disconnect();

        this.onChannelDisconnected();
    }

    // Bluetooth state change event handlers
    protected void onChannelConnected() {
        this.mIsChannelConnected = true;

        // TODO: LargedataPort?
        // Start listening thread
        if (this.mDefaultPort != null)
            this.mDefaultPort.runListeningThread(this, this.mDownloadFilePath);

        // Run custom handlers
        CommBroadcaster.onCommChannelState(this, this.mIsChannelConnected);
    }

    protected void onChannelDisconnected() {
        this.mIsChannelConnected = false;

        // TODO: LargedataPort?
        // Finish listening thread
        if (this.mDefaultPort != null)
            this.mDefaultPort.stopListeningThread();

        // Run custom handlers
        CommBroadcaster.onCommChannelState(this, this.mIsChannelConnected);
    }

    public boolean isChannelConnected() {
        return this.mIsChannelConnected;
    }

    public boolean isBluetoothDeviceConnected() {
        return this.mBluetoothDeviceController.isConnected();
    }

    public boolean isWifiDirectDeviceConnected() {
        return this.mWifiDirectDeviceController.isConnected();
    }

    // Enable Largedata = Notify Wi-fi direct ON Command + Connect WFD Device + Open Largedata Port
    public void enableLargeData() {
        // TODO: implement it
        // (Enable Largedata) Step 1. Notify Wi-fi direct ON Command
        boolean isOpenSuccess = this.mControlPort.open();
        if(!isOpenSuccess)
            return;
        // TODO: remove sending "on/off" message from OPEL device
        this.sendRawMessageOnControl("on", null);
        this.mControlPort.close();

        // (Enable Largedata) Step 2. Connect WFD Device
        this.mWifiDirectDeviceController.connect();
    }

    public int disableLargeData() {
        // TODO: implement it
    }

    // On received raw message
    @Override
    public void onReceivingRawMessage(byte[] messageData, int messageDataLength, String filePath) {
        String listenedMessage = null;
        try {
            listenedMessage = new String(messageData, "UTF-8");
            CommBroadcaster.onReceivedMessage(self, listenedMessage, filePath);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }

    // Send raw message (via default or largedata port)
    public void sendRawMessage(String messageData, File file) {
        if (!this.isBluetoothDeviceConnected() || !this.isChannelConnected()) {
            this.disconnectChannel();
            return;
        }

        // TODO: LargedataPort?
        int res = this.mDefaultPort.sendRawMessage(messageData.getBytes(),
                messageData.getBytes().length, file);
        if (res < 0) {
            this.disconnectChannel();
        }
    }

    public void sendRawMessageOnControl(String messageData, File file) {
        if (!this.isBluetoothDeviceConnected() || !this.isChannelConnected()) {
            this.disconnectChannel();
            return;
        }

        int res = this.mControlPort.sendRawMessage(messageData.getBytes(),
                messageData.getBytes().length, file);
        if (res < 0) {
            this.disconnectChannel();
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();

        // Initialize device controllers
        this.mBluetoothDeviceController = new BluetoothDeviceController(this);
        this.mWifiDirectDeviceController = new WifiDirectDeviceController(this);

        // Initialize ports
        this.mDefaultPort = new BluetoothCommPort(UUID.fromString(
                "0a1b2c3d-4e5f-6a1c-2d0e-1f2a3b4c5d6d"));
        this.mControlPort = new BluetoothCommPort(UUID.fromString(
                "0a1b2c3d-4e5f-6a1c-2d0e-1f2a3b4c5d6e"));
        this.mLargeDataPort = new WifiDirectCommPort(10001);
    }

    @Override
    public IBinder onBind(Intent intent) {
        this.mBindersCount++;

        // TODO: Hard-coded download file path
        if (mBindersCount == 1) {
            this.mDownloadFilePath = Environment.getExternalStorageDirectory().getPath() + "/OPEL";
        }

        return this.mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        this.mBindersCount--;
        if (this.mBindersCount <= 0) {
            // No one uses CommChannelService
            this.disconnectChannel();
        }
        return false;
    }

    // Binder (for the caller of CommChannelService)
    public class CommBinder extends android.os.Binder {
        public CommChannelService getService() {
            return CommChannelService.this;
        }
    }
}

interface DeviceController {
    public boolean isConnected();

    public void connect();

    public void disconnect();
}

class BluetoothDeviceController implements DeviceController {
    private CommChannelService mService;
    private boolean mIsConnected = false;
    private BluetoothDeviceStatusReceiver mBluetoothDeviceStatusReceiver = null;

    public BluetoothDeviceController(CommChannelService service) {
        this.mService = service;
    }

    public boolean isConnected() {
        return this.mIsConnected;
    }

    // Connect to Bluetooth device
    public void connect() {
        Intent bluetoothConnectorIntent = new Intent(this.mService,
                BluetoothConnectorActivity.class);
        bluetoothConnectorIntent.putExtra(BluetoothConnectorActivity
                .INTENT_KEY_RECEIVER, new BluetoothConnectorResultReceiver(this.mService));
        bluetoothConnectorIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        this.mService.startActivity(bluetoothConnectorIntent);
    }

    class BluetoothConnectorResultReceiver extends ResultReceiver {
        static final String RECEIVER_KEY_FAIL_MESSAGE = "FailMessage";
        static final String RECEIVER_KEY_BT_NAME =
                "BluetooothDeviceName";
        static final String RECEIVER_KEY_BT_ADDRESS =
                "BluetooothDeviceAddress";

        private CommChannelService mService;

        // Receiver from BluetoothConnectorActivity
        private BluetoothConnectorResultReceiver(CommChannelService service) {
            super(null);
            this.mService = service;
        }

        @Override
        protected void onReceiveResult(int resultCode, Bundle resultData) {
            if (resultCode == Activity.RESULT_OK) {
                String bluetoothName = resultData.getString(RECEIVER_KEY_BT_NAME);
                String bluetoothAddress = resultData.getString(RECEIVER_KEY_BT_ADDRESS);
                if (bluetoothName != null && bluetoothAddress != null
                        && !bluetoothName.isEmpty() && !bluetoothAddress.isEmpty()) {
                    // Success
                    onConnectingSuccess(bluetoothName, bluetoothAddress);
                } else {
                    // Fail
                    onConnectingFail();
                }
            } else {
                // Fail
                onConnectingFail();
            }
        }
    }

    private void onConnectingSuccess(String bluetoothName, String bluetoothAddress) {
        BluetoothDevice bluetoothDevice = findBluetoothDevice(bluetoothName, bluetoothAddress);
        this.onConnected(bluetoothDevice);

        // CommChannelService's bluetooth device connection success event handlers
        this.mService.onConnectingBluetoothDeviceSuccess(bluetoothDevice);
    }

    private void onConnectingFail() {
        // CommChannelService's bluetooth device connection fail event handlers
        this.mService.onConnectingBluetoothDeviceFail();

        this.onDisconnected();
    }

    private void onConnected(BluetoothDevice bluetoothDevice) {
        this.mIsConnected = true;

        // Start to watch Bluetooth device's status
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        this.mBluetoothDeviceStatusReceiver = new BluetoothDeviceStatusReceiver(bluetoothDevice);
        this.mService.registerReceiver(this.mBluetoothDeviceStatusReceiver, filter);

        // Custom Bluetooth device connection event handlers
        CommBroadcaster.onBluetoothDeviceStateChanged(this.mService, this.mIsConnected);
    }

    private void onDisconnected() {
        this.mIsConnected = false;

        // Unregister receiver
        this.mService.unregisterReceiver(this.mBluetoothDeviceStatusReceiver);

        // Custom Bluetooth device disconnection event handlers
        CommBroadcaster.onBluetoothDeviceStateChanged(this.mService, this.mIsConnected);
    }

    class BluetoothDeviceStatusReceiver extends BroadcastReceiver {
        private BluetoothDevice mBindingBluetoothDevice;

        public BluetoothDeviceStatusReceiver(BluetoothDevice bindingBluetoothDevice) {
            this.mBindingBluetoothDevice = bindingBluetoothDevice;
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.compareTo(action) == 0) {
                BluetoothDevice device = intent.getParcelableExtra
                        (BluetoothDevice.EXTRA_DEVICE);
                if (this.mBindingBluetoothDevice != null) {
                    if (device.getBondState() != BluetoothDevice.BOND_BONDED) {
                        onDisconnected();
                    }
                }
                // TODO: what if bonding is not done forever?
            }
        }
    }

    // Disconnect from Bluetooth device
    public void disconnect() {
        // not yet implemented
        this.onDisconnected();
    }

    // Utility functions
    private BluetoothDevice findBluetoothDevice(String bluetoothName, String bluetoothAddress) {
        if (bluetoothName == null || bluetoothAddress == null
                || bluetoothName.isEmpty() || bluetoothAddress.isEmpty()) {
            return null;
        }

        Set<BluetoothDevice> bluetoothDevices = BluetoothAdapter
                .getDefaultAdapter().getBondedDevices();
        if (bluetoothDevices.size() > 0) {
            for (BluetoothDevice bluetoothDevice : bluetoothDevices) {
                String deviceName = bluetoothDevice.getName();
                String deviceAddress = bluetoothDevice.getAddress();
                if (deviceName.compareTo(bluetoothName) == 0 &&
                        deviceAddress.compareTo(bluetoothAddress) == 0) {
                    return bluetoothDevice;
                }
            }
        }
        return null;
    }
}

class WifiDirectDeviceController implements DeviceController, WifiDirectListener {
    static private String TAG = "WFDController";
    private CommChannelService mService;
    private boolean mIsConnected = false;
    private WifiDirectBroadcastReceiver mWifiDirectBroadcastReceiver;

    public WifiDirectDeviceController(CommChannelService service) {
        this.mService = service;
    }

    public boolean isConnected() {
        return this.mIsConnected;
    }

    public void connect() {
        // Initialize Wi-fi Direct Broadcast Receiver
        IntentFilter wifiP2PIntentFilter;
        wifiP2PIntentFilter = new IntentFilter();
        wifiP2PIntentFilter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
        wifiP2PIntentFilter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
        wifiP2PIntentFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
        wifiP2PIntentFilter.addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);

        WifiP2pManager wifiP2pManager = (WifiP2pManager) this.mService.getSystemService(
                Activity.WIFI_P2P_SERVICE);
        WifiP2pManager.Channel wifiP2pManagerChannel = wifiP2pManager.initialize(this.mService,
                this.mService.getMainLooper(), null);
        this.mWifiDirectBroadcastReceiver = new WifiDirectBroadcastReceiver(
                wifiP2pManager, wifiP2pManagerChannel);
        this.mWifiDirectBroadcastReceiver.setStateListener(this);
        this.mService.registerReceiver(this.mWifiDirectBroadcastReceiver, wifiP2PIntentFilter);

        // Start Wi-fi direct discovery
        WifiP2pManager.Channel channel = wifiP2pManager.initialize(this.mService,
                this.mService.getMainLooper(), null);
        wifiP2pManager.discoverPeers(channel, new WifiP2pManager
                .ActionListener() {
            @Override
            public void onSuccess() {
                Log.d(TAG, "Scan started");
            }

            @Override
            public void onFailure(int reason) {

            }
        });
        // TODO: implement it
    }

    public void disconnect() {
        if (this.mWifiDirectBroadcastReceiver != null)
            this.mService.unregisterReceiver(this.mWifiDirectBroadcastReceiver);

        // TODO: implement it
    }

    // TODO: Wi-fi Direct state change event handlers

    // TODO: Implement WifiDirectListener
    @Override
    public void onWifiDirectStateChanged(boolean isWifiOn) {
        CommBroadcaster.onWifiDirectDeviceStateChanged(this.mService, isWifiOn);
    }

    // TODO: fuse it into WifiDirectDeviceController
    public class WifiDirectBroadcastReceiver extends BroadcastReceiver {
        private static final String TAG = "WFDBroadcastReceiver";
        private WifiP2pManager mManager;
        private WifiP2pManager.Channel mChannel;
        private WifiP2pDevice opelDevice;

        public WifiP2pManager.ConnectionInfoListener mConnectionListener;
        public WifiP2pManager.PeerListListener mPeerListener;

        boolean sent_connected = false;
        boolean connection = false;

        public boolean removing = false;

        // Wi-fi Direct Device's Name & Address
        private String mWifiDirectName;
        private String mWifiDirectIPAddress;

        private WifiDirectListener mListener = null;

        public void setOwnerController(CMFWLegacy ownerController) {
            this.mOwnerController = ownerController;
        }

        public void setWifiDirectInfo(String wifiDirectName, String
                wifiDirectIPAddress) {
            this.mWifiDirectName = wifiDirectName;
            this.mWifiDirectIPAddress = wifiDirectIPAddress;
        }

        public String getWifiDirectName() {
            return this.mWifiDirectName;
        }
        public String getWifiDirectAddress() {
            return this.mWifiDirectIPAddress;
        }

        public void setStateListener(WifiDirectListener stateListener) {
            this.mListener = stateListener;
        }

        public WifiDirectBroadcastReceiver(WifiP2pManager manager, WifiP2pManager
                .Channel channel) {
            super();

            this.mManager = manager;
            this.mChannel = channel;
            opelDevice = null;

            mPeerListener = new WifiP2pManager.PeerListListener() {
                @Override
                public void onPeersAvailable(WifiP2pDeviceList peers) {
                    if (mWifiDirectName.isEmpty() == true) {
                        Log.d(TAG, "Failed to connectChannel! No Wi-fi Direct " + "name " +
                                "given!");
                        sent_connected = false;
                        return;
                    }

                    WifiP2pConfig config = new WifiP2pConfig();
                    for (WifiP2pDevice device : peers.getDeviceList()) {
                        if (device.deviceName.compareTo(mWifiDirectName) == 0) {
                            Log.d(TAG, "Found device connecting...");
                            opelDevice = device;
                            if (opelDevice.status != WifiP2pDevice.AVAILABLE)
                                return;

                            if (device.status == WifiP2pDevice.AVAILABLE) {
                                config.deviceAddress = device.deviceAddress;

                                if (device.wpsPbcSupported()) {
                                    config.wps.setup = WpsInfo.PBC;
                                    Log.d(TAG, "WPS: PBC");
                                } else if (device.wpsKeypadSupported()) {
                                    config.wps.setup = WpsInfo.KEYPAD;
                                    config.wps.pin = new String("12345670");
                                    Log.d(TAG, "WPS:KeyPad");
                                } else {
                                    config.wps.setup = WpsInfo.DISPLAY;
                                    Log.d(TAG, "WPS:Display");
                                }
                                mManager.connect(mChannel, config, new
                                        WifiP2pManager.ActionListener() {
                                            @Override
                                            public void onSuccess() {
                                                Log.d(TAG, "Succedded to send " +
                                                        "connectChannel msg");
                                                sent_connected = true;
                                            }

                                            @Override
                                            public void onFailure(int reason) {
                                                Log.d(TAG, "Failed to connectChannel" + Integer
                                                        .toString(reason));
                                                sent_connected = false;
                                            }
                                        });
                                break;
                            } else if (device.status == WifiP2pDevice.INVITED) {
                                Log.d(TAG, "Invited");

                            }
                        }
                    }
                }
            };

            mConnectionListener = new WifiP2pManager.ConnectionInfoListener() {
                @Override
                public void onConnectionInfoAvailable(WifiP2pInfo info) {
                }
            };
        }

        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (action.compareTo(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION) == 0) {
                // Check to see if Wi-Fi is enabled and notify appropriate activity
                int state = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE, -1);
                if (state == WifiP2pManager.WIFI_P2P_STATE_ENABLED) {
                    Log.d(TAG, "State: Enabled");
                } else {
                    Log.d(TAG, "State: Disabled");
                }
            } else if (action.compareTo(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION) == 0) {
                // Call WifiP2pManager.requestPeers() to get a list of current peers
                if (mManager != null) {
                    mManager.requestPeers(mChannel, mPeerListener);
                }


            } else if (action.compareTo(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION) == 0) {
                // Respond to new connection or disconnections

                NetworkInfo networkInfo = (NetworkInfo) intent.getParcelableExtra
                        (WifiP2pManager.EXTRA_NETWORK_INFO);

                if (networkInfo.isConnected()) {
                    WifiP2pGroup p2pGroup = (WifiP2pGroup) intent
                            .getParcelableExtra(WifiP2pManager
                                    .EXTRA_WIFI_P2P_GROUP);
                    if (p2pGroup.getOwner().deviceName != null && p2pGroup
                            .getOwner().deviceName.compareTo(mWifiDirectName) ==
                            0) {
                        Log.d(TAG, "Wi-fi Direct Connected");

                        opelDevice = p2pGroup.getOwner();
                        connection = true;

                        if (this.mListener != null)
                            this.mListener.onWifiDirectStateChanged(true);

                        if (isConnected() && this.mOwnerController != null &&
                                this.mOwnerController.wfd_in_use == 0)
                            this.mOwnerController.turnOffWifiDirect();
                    }
                } else if (networkInfo.isConnectedOrConnecting()) {
                    Log.d(TAG, "Wi-fi Direct Connecting");
                } else if (networkInfo.isAvailable()) {
                    if (opelDevice != null) {
                        Log.d(TAG, "Network available");
                        opelDevice = null;
                        removing = false;
                        connection = false;

                        if (this.mListener != null)
                            this.mListener.onWifiDirectStateChanged(false);
                    }
                }

            } else if (action.compareTo(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION) == 0) {
                // Respond to this device's wifi state changing
            }
        }

        public boolean isConnected() {
            if (opelDevice == null) return false;

            Log.d(TAG, Integer.toString(opelDevice.status));

            if (removing == false) return connection;
            return false;
        }
    }
}