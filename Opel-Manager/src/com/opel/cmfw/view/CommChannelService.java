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
import android.net.wifi.p2p.WifiP2pManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.os.ResultReceiver;
import android.util.Log;

import com.opel.cmfw.controller.BluetoothCommPort;
import com.opel.cmfw.controller.CommPortListener;
import com.opel.cmfw.controller.WifiDirectCommPort;

import java.io.File;
import java.io.UnsupportedEncodingException;
import java.util.Set;
import java.util.UUID;

/* Threads of CommChannelService
    - main thread: send messages that are coming from OPEL Manager internal to CMFWLegacy
    - listening thread: poll messages from CMFWLegacy and pass them to OPEL Manager internal
*/

/* Roles of CommChannelService
    - Initialize/disconnectChannel Bluetooth, Wi-fi Direct device
    (BluetoothDeviceController, WifiDirectDeviceController)

    - State Management of Bluetooth, Wi-fi Direct device
    (BluetoothDeviceController.State, WifiDirectDeviceController.State)

    - Transfer data via Bluetooth or Wi-fi Direct
    (BlutoothCommPort, WifiDirectCommPort)
 */

public class CommChannelService extends Service implements CommPortListener {
    private static final String TAG = "CommChannelService";
    private final CommChannelService self = this;
    private final IBinder mBinder;
    private int mBindersCount = 0;

    private State mState;
    private ConnectChannelProcedure mConnectChannelProcedure;
    private EnableLargeDataModeProcedure mEnableLargeDataModeProcedure;

    // Device Controller
    private BluetoothDeviceController mBluetoothDeviceController = null;
    private WifiDirectDeviceController mWifiDirectDeviceController = null;

    // Ports
    private BluetoothCommPort mDefaultPort;
    private BluetoothCommPort mControlPort;
    private WifiDirectCommPort mLargeDataPort;

    // Download path
    private String mDownloadFilePath;

    static public final int STATE_DISCONNECTED = 0;
    static public final int STATE_CONNECTED_DEFAULT = 1;
    static public final int STATE_CONNECTED_LARGEDATA = 2;

    static private String DEFAULT_PORT_BLUETOOTH_UUID = "0a1b2c3d-4e5f-6a1c-2d0e-1f2a3b4c5d6d";
    static private String CONTROL_PORT_BLUETOOTH_UUID = "0a1b2c3d-4e5f-6a1c-2d0e-1f2a3b4c5d6e";
    static private int LARGEDATA_PORT_TCP_PORT = 10001;

    public CommChannelService() {
        this.mBinder = new CommBinder();
        this.mState = new State();
        this.mConnectChannelProcedure = new ConnectChannelProcedure();
        this.mEnableLargeDataModeProcedure = new EnableLargeDataModeProcedure();
    }

    public boolean isChannelConnected() {
        return this.isChannelConnectedDefault() || this.isChannelConnectedLargeData();
    }

    public boolean isChannelConnectedDefault() {
        return this.mState.get() == STATE_CONNECTED_DEFAULT;
    }

    public boolean isChannelConnectedLargeData() {
        return this.mState.get() == STATE_CONNECTED_LARGEDATA;
    }

    public boolean isBluetoothDeviceConnected() {
        return this.mBluetoothDeviceController.isConnected();
    }

    public boolean isWifiDirectDeviceConnected() {
        return this.mWifiDirectDeviceController.isConnected();
    }

    public void setDownloadFilePath(String downloadFilePath) {
        this.mDownloadFilePath = downloadFilePath;
    }

    public void connectChannel() {
        this.mConnectChannelProcedure.start();
    }

    public void disconnectChannel() {
        // Finish listening threads
        if (this.mDefaultPort != null) this.mDefaultPort.stopListeningThread();
        if (this.mControlPort != null) this.mControlPort.stopListeningThread();
        if (this.mLargeDataPort != null) this.mLargeDataPort.stopListeningThread();

        // Close ports
        if (this.mDefaultPort != null) this.mDefaultPort.close();
        if (this.mControlPort != null) this.mControlPort.close();
        if (this.mLargeDataPort != null) this.mLargeDataPort.close();

        // Disconnect devices
        if (this.mBluetoothDeviceController != null) this.mBluetoothDeviceController.disconnect();
        if (this.mWifiDirectDeviceController != null) this.mWifiDirectDeviceController.disconnect();

        this.mState.transitToDisconnected();
    }

    public void enableLargeDataMode() {
        this.mEnableLargeDataModeProcedure.start();
    }

    private void disableLargeDataMode() {
        // Finish listening threads
        if (this.mControlPort != null) this.mControlPort.stopListeningThread();
        if (this.mLargeDataPort != null) this.mLargeDataPort.stopListeningThread();

        // Close ports
        if (this.mControlPort != null) this.mControlPort.close();
        if (this.mLargeDataPort != null) this.mLargeDataPort.close();

        // Disconnect device
        if (this.mWifiDirectDeviceController != null) this.mWifiDirectDeviceController.disconnect();
    }

    // On received raw message (via default or largedata port)
    @Override
    public void onReceivingRawMessage(byte[] messageData, int messageDataLength, String filePath) {
        String listenedMessage = null;
        try {
            listenedMessage = new String(messageData, "UTF-8");
            CommBroadcaster.onReceivedRawMessage(self, listenedMessage, filePath);
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

        if (this.isChannelConnectedLargeData()) {
            int res;
            res = this.mLargeDataPort.sendRawMessage(messageData.getBytes(), messageData.getBytes
                    ().length, file);
            if (res < 0) {
                this.disableLargeDataMode();
            }
        } else {
            int res;
            res = this.mDefaultPort.sendRawMessage(messageData.getBytes(), messageData.getBytes()
                    .length, file);
            if (res < 0) {
                this.disconnectChannel();
            }
        }

    }

    // Connect CommChannelService = Connect Bluetooth Device + Open Default Port
    private class ConnectChannelProcedure {
        private ConnectingResultListener mConnectingResultListener = null;

        public void start() {
            // (Connect) Step 1. Connect Bluetooth Device
            this.mConnectingResultListener = new ConnectingResultListener();
            mBluetoothDeviceController.connect(this.mConnectingResultListener);
        }

        private class ConnectingResultListener implements BluetoothDeviceController
                .ConnectingResultListener {
            @Override
            public void onConnectingBluetoothDeviceSuccess(BluetoothDevice bluetoothDevice) {
                openDefaultPort();
            }

            @Override
            public void onConnectingBluetoothDeviceFail() {
                onFail();
            }
        }

        private void openDefaultPort() {
            // (Connect) Step 2. Open Port
            boolean isOpeningSuccess = mDefaultPort.open();

            // Connect the Bluetooth device
            if (isOpeningSuccess) {
                onSuccess();
            } else {
                onFail();
            }
        }

        private void onSuccess() {
            // Start listening thread
            if (mDefaultPort != null) mDefaultPort.runListeningThread(self, mDownloadFilePath);

            // Handle Channel connection
            mState.transitToConnectedDefault();
        }

        private void onFail() {
            disconnectChannel();
        }
    }

    // Enable Largedata = Notify Wi-fi direct ON Command + Connect WFD Device + Open
    // Largedata Port
    private class EnableLargeDataModeProcedure {
        private ControlPortListener mControlPortListener = null;
        private ConnectingResultListener mConnectingResultListener = null;
        private String mIpAddress = null;
        static private final String kWifiDirectOnMessage = "on";

        public void start() {
            // (Enable Largedata) Step 1. Notify Wi-fi direct ON Command
            boolean isOpenSuccess = mControlPort.open();
            if (!isOpenSuccess) return;

            // Start listening threa
            this.mControlPortListener = new ControlPortListener();
            mControlPort.runListeningThread(this.mControlPortListener, mDownloadFilePath);

            // Send Wi-fi direct on message
            sendRawMessageOnControl(kWifiDirectOnMessage, null);
        }

        private void sendRawMessageOnControl(String messageData, File file) {
            if (!isBluetoothDeviceConnected() || !isChannelConnected()) {
                onFail();
                return;
            }

            int res = mControlPort.sendRawMessage(messageData.getBytes(), messageData.getBytes()
                    .length, file);
            if (res < 0) {
                onFail();
            }
        }

        class ControlPortListener implements CommPortListener {
            @Override
            public void onReceivingRawMessage(byte[] messageData, int messageDataLength, String
                    filePath) {
                String rawMessage = null;
                try {
                    rawMessage = new String(messageData, "UTF-8");
                    String[] words = rawMessage.split("\\r?\\n");
                    mIpAddress = words[0];
                    String wifiDirectName = words[1];
                    discoverAndConnectWifiDirectDevice(wifiDirectName);
                } catch (UnsupportedEncodingException e) {
                    e.printStackTrace();
                    onFail();
                }
            }
        }

        private void discoverAndConnectWifiDirectDevice(String wifiDirectName) {
            // (Enable Largedata) Step 2. Connect WFD Device
            this.mConnectingResultListener = new ConnectingResultListener();
            mWifiDirectDeviceController.connect(this.mConnectingResultListener, wifiDirectName);
        }

        class ConnectingResultListener implements WifiDirectDeviceController
                .ConnectingResultListener {
            @Override
            public void onConnectingWifiDirectDeviceSuccess() {
                openLargeDataPort();
            }

            @Override
            public void onConnectingWifiDirectDeviceFail() {
                onFail();
            }
        }

        private void openLargeDataPort() {
            // (Enable Largedata) Step 3. Open largedata port
            boolean isOpeningSuccess = mLargeDataPort.open(this.mIpAddress);

            // Connect the Bluetooth device
            if (isOpeningSuccess) {
                this.onSuccess();
            } else {
                this.onFail();
            }
        }

        private void onSuccess() {
            // Close control port
            if (mControlPort != null) {
                mControlPort.stopListeningThread();
                mControlPort.close();
            }

            // Start listening thread
            if (mLargeDataPort != null) mLargeDataPort.runListeningThread(self, mDownloadFilePath);

            // Handle Channel connection
            mState.transitToConnectedLargedata();
        }

        private void onFail() {
            disableLargeDataMode();
        }
    }

    private class State {
        private CommBroadcastReceiver mCommBroadcastReceiver;
        private DeviceControllerListener mDeviceControllerListener;
        private int mState = STATE_DISCONNECTED;

        public void transitToConnectedDefault() {
            this.mState = STATE_CONNECTED_DEFAULT;

            // Start to watch Bluetooth & Wi-fi Direct device's status
            this.startToWatchDeviceState();

            // Broadcast CommChannel default connection event via CommBroadcaster
            CommBroadcaster.onCommChannelState(self, this.mState);
        }

        public void transitToConnectedLargedata() {
            this.mState = STATE_CONNECTED_LARGEDATA;

            // Broadcast CommChannel largedata connection event via CommBroadcaster
            CommBroadcaster.onCommChannelState(self, this.mState);
        }

        public void transitToDisconnected() {
            this.mState = STATE_DISCONNECTED;

            // Stop to watch Bluetooth & Wi-fi Direct device's status
            this.stopToWatchDeviceState();

            // Broadcast CommChannel disconnection event via CommBroadcaster
            CommBroadcaster.onCommChannelState(self, this.mState);
        }

        private void startToWatchDeviceState() {
            IntentFilter broadcastIntentFilter = new IntentFilter();
            broadcastIntentFilter.addAction(CommBroadcastReceiver.ACTION);
            this.mDeviceControllerListener = new DeviceControllerListener();
            this.mCommBroadcastReceiver = new CommBroadcastReceiver(this.mDeviceControllerListener);
            self.registerReceiver(this.mCommBroadcastReceiver, broadcastIntentFilter);
        }

        private void stopToWatchDeviceState() {
            if (this.mCommBroadcastReceiver != null)
                self.unregisterReceiver(this.mCommBroadcastReceiver);
        }

        class DeviceControllerListener implements CommChannelEventListener {
            @Override
            public void onWifiDirectDeviceStateChanged(boolean isConnected) {
                if (!isConnected) {
                    if (mState == STATE_CONNECTED_LARGEDATA) transitToConnectedDefault();
                }
            }

            @Override
            public void onBluetoothDeviceStateChanged(boolean isConnected) {
                if (!isConnected) {
                    if (mState == STATE_CONNECTED_DEFAULT || mState == STATE_CONNECTED_LARGEDATA)
                        transitToDisconnected();
                }
            }

            @Override
            public void onCommChannelStateChanged(int commChannelState) {
                // not used
            }

            @Override
            public void onReceivedRawMessage(String message, String filePath) {
                // not used
            }
        }

        public int get() {
            return this.mState;
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();

        // Initialize device controllers
        this.mBluetoothDeviceController = new BluetoothDeviceController(this);
        this.mWifiDirectDeviceController = new WifiDirectDeviceController(this);

        // Initialize ports
        this.mDefaultPort = new BluetoothCommPort(UUID.fromString(DEFAULT_PORT_BLUETOOTH_UUID));
        this.mControlPort = new BluetoothCommPort(UUID.fromString(CONTROL_PORT_BLUETOOTH_UUID));
        this.mLargeDataPort = new WifiDirectCommPort(LARGEDATA_PORT_TCP_PORT);
    }

    @Override
    public IBinder onBind(Intent intent) {
        this.mBindersCount++;

        // Hard-coded download file path
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

class BluetoothDeviceController {
    private Service mService;

    private ConnectProcedure mConnectProcedureProcedure = new ConnectProcedure();
    private State mState = new State();

    public BluetoothDeviceController(Service service) {
        this.mService = service;
    }

    public boolean isConnected() {
        return this.mState.isConnected();
    }

    public void connect(ConnectingResultListener connectingResultListener) {
        this.mConnectProcedureProcedure.start(connectingResultListener);
    }

    public void disconnect() {
        // not yet implemented
        this.mState.transitToDisconnected();
    }

    interface ConnectingResultListener {
        public void onConnectingBluetoothDeviceSuccess(BluetoothDevice bluetoothDevice);

        public void onConnectingBluetoothDeviceFail();
    }

    class ConnectProcedure {
        private ConnectingResultListener mConnectingResultListener = null;

        // ConnectProcedure to Bluetooth device
        public void start(ConnectingResultListener connectingResultListener) {
            this.mConnectingResultListener = connectingResultListener;

            Intent bluetoothConnectorIntent = new Intent(mService, BluetoothConnectorActivity
                    .class);
            bluetoothConnectorIntent.putExtra(BluetoothConnectorActivity.INTENT_KEY_RECEIVER, new
                    BluetoothConnectorResultReceiver());
            bluetoothConnectorIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mService.startActivity(bluetoothConnectorIntent);
        }

        class BluetoothConnectorResultReceiver extends ResultReceiver {
            static final String RECEIVER_KEY_FAIL_MESSAGE = "FailMessage";
            static final String RECEIVER_KEY_BT_NAME = "BluetooothDeviceName";
            static final String RECEIVER_KEY_BT_ADDRESS = "BluetooothDeviceAddress";

            // Receiver from BluetoothConnectorActivity
            private BluetoothConnectorResultReceiver() {
                super(null);
            }

            @Override
            protected void onReceiveResult(int resultCode, Bundle resultData) {
                if (resultCode == Activity.RESULT_OK) {
                    String bluetoothName = resultData.getString(RECEIVER_KEY_BT_NAME);
                    String bluetoothAddress = resultData.getString(RECEIVER_KEY_BT_ADDRESS);
                    if (bluetoothName != null && bluetoothAddress != null && !bluetoothName
                            .isEmpty() && !bluetoothAddress.isEmpty()) {
                        // Success
                        onSuccess(bluetoothName, bluetoothAddress);
                    } else {
                        // Fail
                        onFail();
                    }
                } else {
                    // Fail
                    onFail();
                }
            }
        }

        private void onSuccess(String bluetoothName, String bluetoothAddress) {
            BluetoothDevice bluetoothDevice = findBluetoothDevice(bluetoothName, bluetoothAddress);

            // State transition
            mState.transitToConnected(bluetoothDevice);

            // Notify result
            this.mConnectingResultListener.onConnectingBluetoothDeviceSuccess(bluetoothDevice);
        }

        private void onFail() {
            // State transition
            mState.transitToDisconnected();

            // Notify result
            this.mConnectingResultListener.onConnectingBluetoothDeviceFail();
        }

        // Utility functions
        private BluetoothDevice findBluetoothDevice(String bluetoothName, String bluetoothAddress) {
            if (bluetoothName == null || bluetoothAddress == null || bluetoothName.isEmpty() ||
                    bluetoothAddress.isEmpty()) {
                return null;
            }

            Set<BluetoothDevice> bluetoothDevices = BluetoothAdapter.getDefaultAdapter()
                    .getBondedDevices();
            if (bluetoothDevices.size() > 0) {
                for (BluetoothDevice bluetoothDevice : bluetoothDevices) {
                    String deviceName = bluetoothDevice.getName();
                    String deviceAddress = bluetoothDevice.getAddress();
                    if (deviceName.compareTo(bluetoothName) == 0 && deviceAddress.compareTo
                            (bluetoothAddress) == 0) {
                        return bluetoothDevice;
                    }
                }
            }
            return null;
        }
    }

    private class State {
        private boolean mIsConnected = false;

        private BluetoothDeviceStatusReceiver mBluetoothDeviceStatusReceiver = null;

        public boolean isConnected() {
            return this.mIsConnected;
        }

        public void transitToConnected(BluetoothDevice bluetoothDevice) {
            this.mIsConnected = true;

            // Start to watch Bluetooth device's status
            this.startToWatchDeviceState(bluetoothDevice);

            // Broadcast Bluetooth device connection event via CommBroadcaster
            CommBroadcaster.onBluetoothDeviceStateChanged(mService, this.mIsConnected);
        }

        public void transitToDisconnected() {
            this.mIsConnected = false;

            // Stop to watch Bluetooth device's status
            this.stopToWatchDeviceState();

            // Broadcast Bluetooth device disconnection event via CommBroadcaster
            CommBroadcaster.onBluetoothDeviceStateChanged(mService, this.mIsConnected);
        }

        private void startToWatchDeviceState(BluetoothDevice bluetoothDevice) {
            IntentFilter filter = new IntentFilter();
            filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
            this.mBluetoothDeviceStatusReceiver = new BluetoothDeviceStatusReceiver
                    (bluetoothDevice);
            mService.registerReceiver(this.mBluetoothDeviceStatusReceiver, filter);
        }

        private void stopToWatchDeviceState() {
            if (this.mBluetoothDeviceStatusReceiver != null)
                mService.unregisterReceiver(this.mBluetoothDeviceStatusReceiver);
        }

        // Receive Bluetooth device disconnection event from Android Bluetooth framework
        class BluetoothDeviceStatusReceiver extends BroadcastReceiver {
            private BluetoothDevice mBindingBluetoothDevice;

            public BluetoothDeviceStatusReceiver(BluetoothDevice bindingBluetoothDevice) {
                this.mBindingBluetoothDevice = bindingBluetoothDevice;
            }

            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.compareTo(action) == 0) {
                    BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice
                            .EXTRA_DEVICE);
                    if (this.mBindingBluetoothDevice != null) {
                        if (device.getBondState() != BluetoothDevice.BOND_BONDED) {
                            transitToDisconnected();
                        }
                    }
                }
            }
        }
    }
}

class WifiDirectDeviceController {
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
            // wifiP2PIntentFilter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
            // wifiP2PIntentFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
            // wifiP2PIntentFilter.addAction(WifiP2pManager
            // .WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);

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
            if (this.mWifiDirectDeviceStatusReceiver != null)
                mService.unregisterReceiver(this.mWifiDirectDeviceStatusReceiver);
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