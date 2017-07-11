package com.opel.cmfw.view;

import android.app.Activity;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.wifi.p2p.WifiP2pManager;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ResultReceiver;
import android.util.Log;

import com.opel.cmfw.controller.CommController;
import com.opel.cmfw.controller.WifiDirectBroadcastReceiver;
import com.opel.cmfw.controller.WifiDirectListener;

import java.util.Arrays;

/* OPEL Manager's Communication Path
    - OPEL Manager Internal -> CommChannelService(main thread) -> OPEL device's CMFW
    - OPEL device's CMFW -> CommChannelService(polling thread) -> OPEL Manager Internal
 */

/* Threads of CommChannelService
    - main thread: send messages that are coming from OPEL Manager internal to CommController
    - polling thread: poll messages from CommController and pass them to OPEL Manager internal
*/

public class CommChannelService extends Service implements WifiDirectListener {
    private static final String TAG = "CommChannelService";

    // Communication Controller
    private CommController mCommController;

    // Connection state
    private boolean mIsConnected = false;

    // Initialize connection
    public void initializeConnection() {
        CommInitializer.run(this);
    }

    protected void onInitializingSuccess(CommController commController) {
        this.mCommController = commController;

        // Start receiving thread
        // TODO: slimify CommController.connect()
        boolean resConnection = mCommController.connect(CommController
                .CMFW_DEFAULT_PORT);
        if (resConnection == false) {
            // Connection Initialization Fail

            // Broadcast the fail of initialization
            CommBroadcaster.onInitializationResult(this, false);
            // Handle Bluetooth disconnection
            this.onBluetoothDisconnected();
        } else {
            // Connection Initialization Success

            // Broadcast the success of initialization
            CommBroadcaster.onInitializationResult(this, true);
            // Handle Bluetooth connection
            this.onBluetoothConnected();

            // Start to receive message
            this.mPollingThread = new CommChannelService.pollingThread();
            this.mPollingThread.start();
        }
    }

    protected void onInitializingFail() {
        // Broadcast the fail of initialization
        CommBroadcaster.onInitializationResult(this, false);

        // Handle Bluetooth disconnection
        this.onBluetoothDisconnected();
    }

    // Destroy connection
    public void destroyConnection() {
        if (mCommController != null) {
            this.unregisterReceiver(this.mCommController.getWifiDirectBroadcastReceiver());
        }
    }

    // Wi-fi Direct
    public void setWifiDirectInfo(String wifiDirectName, String
            wifiDirectIPAddress) {
        if (mCommController != null) {
            this.mCommController.setWifiDirectInfo(wifiDirectName,
                    wifiDirectIPAddress);
        }
    }

    public String getWifiDirectAddress() {
        if (mCommController != null) {
            return this.mCommController.getWifiDirectAddress();
        } else {
            return "";
        }
    }

    public void turnOnWifiDirect(boolean retry) {
        if (mCommController != null) {
            this.mCommController.cmfw_wfd_on(retry);
        }
    }

    public void turnOffWifiDirect() {
        if (mCommController != null) {
            this.mCommController.cmfw_wfd_off();
        }
    }

    // Connection state management
    protected void onBluetoothConnected() {
        this.mIsConnected = true;
        CommBroadcaster.onBluetoothStateChanged(this, true);
    }

    protected void onBluetoothDisconnected() {
        this.mIsReceiveMessageThreadOn = false;
        if (mCommController != null) {
            this.mCommController.close(CommController.CMFW_DEFAULT_PORT);
        }

        this.mIsConnected = false;
        CommBroadcaster.onBluetoothStateChanged(this, false);
    }

    public boolean isConnected() {
        return this.mIsConnected;
    }

    // Polling Thread
    private pollingThread mPollingThread;
    private boolean mIsReceiveMessageThreadOn = false;
    private final CommChannelService self = this;

    class pollingThread extends Thread {
        @Override
        public void run() {
            mIsReceiveMessageThreadOn = true;
            while (mIsReceiveMessageThreadOn == true) {
                String receivedMessage = pollRawString();

                // Broadcast received message
                CommBroadcaster.onReceivedMessage(self, receivedMessage);
            }
        }
    }

    // Poll raw string
    // TODO: onReceivedMessage
    private String pollRawString() {
        int port = CommController.CMFW_DEFAULT_PORT;
        byte[] polledRawBytes = new byte[4096];
        String rawString;
        if (mCommController == null) {
            return "";
        }
        int res = this.mCommController.cmfw_recv_msg(port, polledRawBytes, 4096);
        if (res < 0) {
            this.onBluetoothDisconnected();
            return "";
        }

        rawString = new String(Arrays.copyOfRange(polledRawBytes, 0, res));

        Log.d(TAG, "pollRawString: " + rawString);
        return rawString;
    }

    // Send raw string
    // TODO: sendMessage
    public void sendRawString(String msg) {
        int port = CommController.CMFW_DEFAULT_PORT;

        Log.d(TAG, "Send Message: " + msg);

        if (this.mCommController != null && this.mCommController
                .cmfw_send_msg(port, msg) < 0) {
            this.onBluetoothDisconnected();
            return;
        }
    }

    // Implement WifiDirectListener
    @Override
    public void onWifiDirectStateChanged(boolean isWifiOn) {
        CommBroadcaster.onWifiStateChanged(this, isWifiOn);
    }

    // Binder (for the caller of CommChannelService)
    public class CommBinder extends android.os.Binder {
        public CommChannelService getService() {
            return CommChannelService.this;
        }
    }

    private int mBindersCount = 0;
    private final IBinder mBinder = new CommBinder();

    @Override
    public IBinder onBind(Intent intent) {
        this.mBindersCount++;
        return this.mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        this.mBindersCount--;
        if (this.mBindersCount <= 0) {
            this.destroyConnection();
        }
        return false;
    }
}

class CommInitializer {
    /* Initialize connection with target device (Initially, in Bluetooth)
       Step 1. Turn on bluetooth device (BluetoothTurningOnActivity)
               (checking communication device permission, turning on bluetooth)
               - On success: proceed to step 2
               - On fail: return fail
       Step 2. Initialize CommController
               - On finish: proceed to step 3
       Step 3. Try to connect default Bluetooth device
               (default = already-paired device of which information is stored)
               - On success: return success
               - On fail: proceed to step 4
       Step 4. Discover and connect Bluetooth device (BluetoothDiscoveryActivity)
               - On success: proceed to step 5
               - On fail: return fail
       Step 5. Store connection information
               - On finish: return success
    */
    private static String TAG = "CommInitializer";
    protected CommChannelService mService;
    protected CommController mCommController = null;

    // Wi-fi Direct
    private IntentFilter mWifiP2PIntentFilter;

    // SharedPreference
    private static final String PREFERENCE_KEY = "CommChannelService";
    private static final String PREFERENCE_ATTR_KEY_BT_NAME =
            "BluetoothDeviceName";
    private static final String PREFERENCE_ATTR_KEY_BT_ADDRESS =
            "BluetoothDeviceAddress";

    private CommInitializer(CommChannelService commChannelService) {
        this.mService = commChannelService;
    }

    public static void run(CommChannelService commChannelService) {
        CommInitializer initializer = new CommInitializer(commChannelService);
        initializer.turnOnBluetoothDevice();
    }

    // Step 1. Turn on bluetooth device (BluetoothTurningOnActivity)
    private void turnOnBluetoothDevice() {
        // Launch BluetoothTurningOnActivity
        Intent btTuringOnIntent = new Intent(this.mService,
                BluetoothTurningOnActivity.class);
        btTuringOnIntent.putExtra(BluetoothTurningOnActivity
                .INTENT_KEY_RECEIVER, new BluetoothTurningOnResultReceiver());
        this.mService.startActivity(btTuringOnIntent);
    }

    // Step 1-a. On success, proceed to Step 2
    private void onTurningOnBluetoothDeviceSuccess() {
        Log.d(TAG, "Bluetooth device is turned on");

        // Proceed to Step 2.
        this.initializeCommController();
    }

    // Step 1-b. On fail, cancel the initialization
    private void onTurningOnBluetoothDeviceFail() {
        Log.e(TAG, "Failed turning on Bluetooth device");

        // Return fail
        this.mService.onInitializingFail();
    }

    // Step 2. Initialize CommController
    private void initializeCommController() {
        // Initialize Wi-fi Direct Broadcast Receiver
        this.mWifiP2PIntentFilter = new IntentFilter();
        this.mWifiP2PIntentFilter.addAction(WifiP2pManager
                .WIFI_P2P_STATE_CHANGED_ACTION);
        this.mWifiP2PIntentFilter.addAction(WifiP2pManager
                .WIFI_P2P_PEERS_CHANGED_ACTION);
        this.mWifiP2PIntentFilter.addAction(WifiP2pManager
                .WIFI_P2P_CONNECTION_CHANGED_ACTION);
        this.mWifiP2PIntentFilter.addAction(WifiP2pManager
                .WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);

        WifiP2pManager wifiP2pManager = (WifiP2pManager) this.mService
                .getSystemService(Activity.WIFI_P2P_SERVICE);
        WifiP2pManager.Channel wifiP2pManagerChannel = wifiP2pManager
                .initialize(this.mService, this.mService.getMainLooper(), null);
        WifiDirectBroadcastReceiver wifiDirectBroadcastReceiver = new
                WifiDirectBroadcastReceiver(wifiP2pManager,
                wifiP2pManagerChannel);
        wifiDirectBroadcastReceiver.setStateListener(this.mService);
        this.mService.registerReceiver(wifiDirectBroadcastReceiver, this
                .mWifiP2PIntentFilter);

        // Initialize CommController
        this.mCommController = new CommController(wifiP2pManager,
                wifiP2pManagerChannel, wifiDirectBroadcastReceiver);

        this.tryConnectingDefaultBluetoothDevice();
    }

    // Step 3. Try connecting default Bluetooth device
    private void tryConnectingDefaultBluetoothDevice() {
        // Restore bluetooth name from local storage
        SharedPreferences sharedPreferences = mService.getSharedPreferences
                (PREFERENCE_KEY, Context.MODE_PRIVATE);
        String bluetoothName = sharedPreferences.getString
                (PREFERENCE_ATTR_KEY_BT_NAME, "");
        String bluetoothAddress = sharedPreferences.getString
                (PREFERENCE_ATTR_KEY_BT_ADDRESS, "");
        Log.d(TAG, "Stored bluetooth name: " + bluetoothName + " / " +
                "bluetooth address: " + bluetoothAddress);

        // TODO: implement it
        // TODO: get BluetoothDevice
        this.mCommController.connect(CommController.CMFW_DEFAULT_PORT);
    }

    // Step 3-a. On success, proceed to Step 5
    private void onTryingConnectingDefaultBluetoothDeviceSuccess() {
        // Return success
        this.mService.onInitializingSuccess(this.mCommController);
    }

    // Step 3-b. On fail, proceed to Step 4
    private void onTryingConnectingDefaultBluetoothDeviceFail() {
        // Proceed to Step 4
        this.discoverBluetoothDevice();
    }

    // Step 4-1. Discover Bluetooth device (BluetoothDiscoveryActivity)
    // TODO: more decoupling is required
    private void discoverBluetoothDevice() {
        // Launch BluetoothDiscoveryActivity for setting communication devices.
        Intent btConnectingIntent = new Intent(mService,
                BluetoothDiscoveryActivity.class);
        btConnectingIntent.putExtra(BluetoothDiscoveryActivity
                .INTENT_KEY_RECEIVER, new BluetoothConnectingResultReceiver());
        btConnectingIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mService.startActivity(btConnectingIntent);
    }

    // Step 4-2. Try connecting Bluetooth device
    private void tryConnectingBluetoothDevice() {
        // TODO: implement it
    }

    // Step 4-2-a. On success, store the bluetooth connection information and
    //             proceed to Step 5
    private void onTryingConnectingBluetoothDeviceSuccess(String bluetoothName,
                                                          String bluetoothAddress) {
        Log.d(TAG, "Pairing request done");

        // Proceed to Step 5.
        this.storeBluetoothDeviceInfo(bluetoothName, bluetoothAddress);
    }

    // Step 4-2-b. On fail, cancel the initialization
    private void onTryingConnectingBluetoothDeviceFail(String failMessage) {
        Log.e(TAG, "Failed pairing operation: " + failMessage);

        // Return fail
        this.mService.onInitializingFail();
    }

    // Step 5. Store connection information
    private void storeBluetoothDeviceInfo(String bluetoothName,
                                          String bluetoothAddress) {
        // Store the name to local storage
        SharedPreferences sharedPreferences = this.mService.getSharedPreferences
                (PREFERENCE_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor sharedPreferencesEditor = sharedPreferences
                .edit();
        sharedPreferencesEditor.putString(PREFERENCE_ATTR_KEY_BT_NAME,
                bluetoothName);
        sharedPreferencesEditor.putString(PREFERENCE_ATTR_KEY_BT_ADDRESS,
                bluetoothAddress);
        sharedPreferencesEditor.commit();

        // Return success
        this.mService.onInitializingSuccess(this.mCommController);
    }

    class BluetoothTurningOnResultReceiver extends ResultReceiver {
        // Receiver from BluetoothDiscoveryActivity
        public BluetoothTurningOnResultReceiver() {
            super(null);
        }

        @Override
        protected void onReceiveResult(int resultCode, Bundle resultData) {
            // Handle the result of Step 1.
            if (resultCode == Activity.RESULT_OK) {
                // On success: Proceed to Step 1-a.
                onTurningOnBluetoothDeviceSuccess();
            } else {
                // On fail: Proceed to Step 1-b.
                onTurningOnBluetoothDeviceFail();
            }
        }
    }

    class BluetoothConnectingResultReceiver extends ResultReceiver {
        // Receiver from BluetoothDiscoveryActivity
        public static final String RECEIVER_KEY_FAIL_MESSAGE = "FailMessage";
        public static final String RECEIVER_KEY_BT_NAME =
                "BluetooothDeviceName";
        public static final String RECEIVER_KEY_BT_ADDRESS =
                "BluetooothDeviceAddress";

        public BluetoothConnectingResultReceiver() {
            super(null);
        }

        @Override
        protected void onReceiveResult(int resultCode, Bundle resultData) {
            // Handle the result of Step 4.
            if (resultCode == Activity.RESULT_OK) {
                String bluetoothName = resultData.getString
                        (RECEIVER_KEY_BT_NAME);
                String bluetoothAddress = resultData.getString
                        (RECEIVER_KEY_BT_ADDRESS);
                if (!bluetoothAddress.isEmpty() && !bluetoothName.isEmpty()) {
                    // On success: Proceed to Step 4-a.
                    onTryingConnectingBluetoothDeviceSuccess(bluetoothName,
                            bluetoothAddress);
                } else {
                    // On fail: Proceed to Step 4-b.
                    onTryingConnectingBluetoothDeviceFail("Cannot get " +
                            "bluetooth device's name and address!");
                }
            } else {
                // On fail: Proceed to Step 4-b.
                String failMessage = resultData.getString
                        (RECEIVER_KEY_FAIL_MESSAGE);
                onTryingConnectingBluetoothDeviceFail(failMessage);
            }
        }
    }
}