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

/* * Command Path (Bluetooth device setting)
   CMFW Users -(IBinder)-> CommService
     -(Intent)-> BluetoothDeviceSettingActivity

   * Command Path (Send message)
   CMFW Users -(IBinder)-> CommService --> CommController

   * Event Path (Bluetooth device setting)
   BluetoothDeviceSettingActivity
     -(BluetoothDeviceSettingResultReceiver)-> CommService
     -(CommBroadcastReceiver)-> CMFW Users (CommEventListener)

   * Event Path (Listen message, listen state)
   CommController -(BluetoothDeviceSettingResultReceiver)-> CommService
     -(CommBroadcastReceiver)-> CMFW Users (CommEventListener)
*/

public class CommService extends Service implements WifiDirectListener {
    private static final String TAG = "CommService";

    // SharedPreference
    private static final String PREFERENCE_KEY = "CommService";
    private static final String PREFERENCE_ATTR_KEY_BT_NAME =
            "BluetoothDeviceName";
    private static final String PREFERENCE_ATTR_KEY_BT_ADDRESS =
            "BluetoothDeviceAddress";

    // Communication Controller
    private CommController mCommController;

    // Wi-fi Direct
    private IntentFilter mWifiP2PIntentFilter;

    // Connection state
    private boolean mIsConnected = false;

    /* Initialize connection with target device (Initially, in Bluetooth)
       Step 1. Try to set bluetooth device (BluetoothTurningOnActivity)
       (checking communication device permission, turning on bluetooth)
       Step 2. Try to initialize CommController with stored info. (CommService)
               - On success: store connection information and proceed to Step 4.
               - On fail: proceed to Step 3.
       Step 3. Try communication device setting (BluetoothDeviceSettingActivity)
               - On success: store connection information and proceed to Step 4.
       Step 4. Initialize CommController (CommService)
               (Make bluetooth socket connected to the target device)
    */
    public void initializeConnection() {
        this.tryCommDeviceSetting();
    }

    public void setWifiDirectInfo(String wifiDirectName, String
            wifiDirectIPAddress) {
        if (mCommController != null) {
            this.mCommController.setWifiDirectInfo(wifiDirectName,
                    wifiDirectIPAddress);
        }
    }

    public String getWifiDirectName() {
        if (mCommController != null) {
            return this.mCommController.getWifiDirectName();
        } else {
            return "";
        }
    }

    public String getWifiDirectAddress() {
        if (mCommController != null) {
            return this.mCommController.getWifiDirectAddress();
        } else {
            return "";
        }
    }

    // Destroy connection with target device
    public void destroyConnection() {
        if (mCommController != null) {
            this.unregisterReceiver(this.mCommController
                    .getWifiDirectBroadcastReceiver());

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

    // Step 1. Try communication device setting
    private void tryCommDeviceSetting() {
        // Restore bluetooth name from local storage
        SharedPreferences sharedPreferences = this.getSharedPreferences
                (PREFERENCE_KEY, Context.MODE_PRIVATE);
        String bluetoothName = sharedPreferences.getString
                (PREFERENCE_ATTR_KEY_BT_NAME, "");
        String bluetoothAddress = sharedPreferences.getString
                (PREFERENCE_ATTR_KEY_BT_ADDRESS, "");
        Log.d(TAG, "Stored bluetooth name: " + bluetoothName + " / " +
                "bluetooth address: " +
                bluetoothAddress);

        // Launch BluetoothDeviceSettingActivity for setting communication
        // devices.
        Intent btDeviceSettingIntent = new Intent(this,
                BluetoothDeviceSettingActivity.class);
        btDeviceSettingIntent.putExtra(BluetoothDeviceSettingActivity
                .INTENT_KEY_RECEIVER, new
                BluetoothDeviceSettingResultReceiver());
        btDeviceSettingIntent.putExtra(BluetoothDeviceSettingActivity
                .INTENT_KEY_DEFAULT_BT_NAME, bluetoothName);
        btDeviceSettingIntent.putExtra(BluetoothDeviceSettingActivity
                .INTENT_KEY_DEFAULT_BT_ADDRESS, bluetoothAddress);
        btDeviceSettingIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        this.startActivity(btDeviceSettingIntent);
    }

    // Step 1-a. On success, store the bluetooth connection information and
    //             make a connection with target device.
    private void onTryingCommDeviceSettingSuccess(String bluetoothName,
                                                  String bluetoothAddress) {
        Log.d(TAG, "Pairing request done");

        // Store the name to local storage
        SharedPreferences sharedPreferences = this.getSharedPreferences
                (PREFERENCE_KEY, Context.MODE_PRIVATE);
        SharedPreferences.Editor sharedPreferencesEditor = sharedPreferences
                .edit();
        sharedPreferencesEditor.putString(PREFERENCE_ATTR_KEY_BT_NAME,
                bluetoothName);
        sharedPreferencesEditor.putString(PREFERENCE_ATTR_KEY_BT_ADDRESS,
                bluetoothAddress);
        sharedPreferencesEditor.commit();

        // Proceed to Step 2.
        this.initializeCommController(bluetoothName, bluetoothAddress);
    }

    // Step 1-b. On fail, cancel initializing connection
    private void onTryingCommDeviceSettingFail(String failMessage) {
        Log.e(TAG, "Failed pairing operation: " + failMessage);
        CommBroadcaster.onInitializationResult(this, false);
        this.onBluetoothDisconnected();
    }

    // Step 2. Initialize CommController
    //         (Make bluetooth socket connected to the target device)
    private void initializeCommController(String bluetoothName, String
            bluetoothAddress) {
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

        WifiP2pManager wifiP2pManager = (WifiP2pManager) this
                .getSystemService(WIFI_P2P_SERVICE);
        WifiP2pManager.Channel wifiP2pManagerChannel = wifiP2pManager
                .initialize(this, getMainLooper(), null);
        WifiDirectBroadcastReceiver wifiDirectBroadcastReceiver = new
                WifiDirectBroadcastReceiver(wifiP2pManager,
                wifiP2pManagerChannel);
        wifiDirectBroadcastReceiver.setStateListener(this);
        this.registerReceiver(wifiDirectBroadcastReceiver, this
                .mWifiP2PIntentFilter);

        // Initialize CommController
        this.mCommController = new CommController(wifiP2pManager,
                wifiP2pManagerChannel, wifiDirectBroadcastReceiver,
                bluetoothName, bluetoothAddress);

        boolean resConnection = this.mCommController.connect(CommController
                .CMFW_DEFAULT_PORT);
        if (resConnection == false) {
            // Connection Initialization Fail
            CommBroadcaster.onInitializationResult(this, false);
            this.onBluetoothDisconnected();
        } else {
            // Connection Initialization Success
            CommBroadcaster.onInitializationResult(this, true);
            this.onBluetoothConnected();

            // Start to receive message
            this.mReceiveMessageThread = new ReceiveMessageThread();
            this.mReceiveMessageThread.start();
        }
    }

    private void onBluetoothConnected() {
        this.mIsConnected = true;
        CommBroadcaster.onBluetoothStateChanged(this, true);
    }

    private void onBluetoothDisconnected() {
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

    // Receive Message Thread
    private ReceiveMessageThread mReceiveMessageThread;
    private boolean mIsReceiveMessageThreadOn = false;
    private final CommService self = this;

    class ReceiveMessageThread extends Thread {
        @Override
        public void run() {
            mIsReceiveMessageThreadOn = true;
            while (mIsReceiveMessageThreadOn == true) {
                String receivedMessage = receiveMessage();

                // Broadcast received message
                CommBroadcaster.onReceivedMessage(self, receivedMessage);
            }
        }
    }

    // Receive Message
    private String receiveMessage() {
        int port = CommController.CMFW_DEFAULT_PORT;
        byte[] buf = new byte[4096];
        String msg;
        if (mCommController == null) {
            return "";
        }
        int res = this.mCommController.cmfw_recv_msg(port, buf, 4096);
        if (res < 0) {
            this.onBluetoothDisconnected();
            return "";
        }

        msg = new String(Arrays.copyOfRange(buf, 0, res));

        Log.d("OPEL", "COMManager - rcv Msg : " + msg);
        return msg;
    }

    // TODO remake from the bottom
//    private String rcvFile(File destDir, JSONParser jp) {
//        int port = CommController.CMFW_DEFAULT_PORT;
//        String name = "";
//        String size = "";
//
//        byte[] buf = new byte[1024];
//        Arrays.fill(buf, (byte) 0);
//
//        byte[] buf2 = new byte[1024];
//        Arrays.fill(buf2, (byte) 0);
//
//
//        name = receiveMessage();
//        if (name.equals("")) return name;
//        if (jp.getValueByKey("type").equals(INSTALLPKG)) {
//            name = jp.getValueByKey("appID") + ".icon";
//        }
//        size = receiveMessage();
//        if (size.equals("")) return name;
//
//        //File downloaddir = Environment.getExternalStoragePublicDirectory
//        // (Environment.DIRECTORY_DOWNLOADS);
//        //File f = new File(downloaddir, name);
//
//        File f = new File(destDir, name);
//        if (this.mCommController.cmfw_recv_file(port, f) < 0) {
//            Log.d("OPEL", "Recv File failed");
//            return "";
//        }
//
//        return name;
//    }

    // Send Message
    public void sendMessage(String msg) {
        int port = CommController.CMFW_DEFAULT_PORT;

        Log.d(TAG, "Send Message: " + msg);

        if (this.mCommController != null && this.mCommController
                .cmfw_send_msg(port, msg) < 0) {
            this.onBluetoothDisconnected();
            return;
        }
    }

    // TODO remake from the bottom
    // Send File
//    private void sendFile(String fileName) {
//
//        int port = CommController.CMFW_DEFAULT_PORT;
//        String fileSize = "";
//        BufferedInputStream bis;
//
//
//        File fd = new File(Environment.getExternalStoragePublicDirectory
//                (Environment.DIRECTORY_DOWNLOADS), fileName);
//
//        fileSize = String.valueOf(fd.length());
//
//        int len;
//        int size = 4096;
//        byte[] data = new byte[size];
//
//        sendMessage(fileName);
//        sendMessage(fileSize);
//        if (this.mCommController.cmfw_send_file(port, fd) < 0) {
//            this.onBluetoothDisconnected();
//            return;
//        }
//
//        Log.d("OPEL", "send File size : " + fileSize);
//
//        return;
//    }

    // Implement WifiDirectListener
    @Override
    public void onWifiDirectStateChanged(boolean isWifiOn) {
        CommBroadcaster.onWifiStateChanged(this, isWifiOn);
    }

    class BluetoothDeviceSettingResultReceiver extends ResultReceiver {
        // Receiver from BluetoothDeviceSettingActivity
        public static final String RECEIVER_KEY_FAIL_MESSAGE = "FailMessage";
        public static final String RECEIVER_KEY_BT_NAME =
                "BluetooothDeviceName";
        public static final String RECEIVER_KEY_BT_ADDRESS =
                "BluetooothDeviceAddress";

        public BluetoothDeviceSettingResultReceiver() {
            super(null);
        }

        @Override
        protected void onReceiveResult(int resultCode, Bundle resultData) {
            // Handle the result of Step 1.
            if (resultCode == Activity.RESULT_OK) {
                String bluetoothName = resultData.getString
                        (RECEIVER_KEY_BT_NAME);
                String bluetoothAddress = resultData.getString
                        (RECEIVER_KEY_BT_ADDRESS);
                if (!bluetoothAddress.isEmpty() && !bluetoothName.isEmpty()) {
                    // On success: Proceed to Step 1-a.
                    onTryingCommDeviceSettingSuccess(bluetoothName,
                            bluetoothAddress);
                } else {
                    // On fail: Proceed to Step 1-b.
                    onTryingCommDeviceSettingFail("Cannot get " + "bluetooth " +
                            "device's name and address!");
                }
            } else {
                // On fail: Proceed to Step 1-b.
                String failMessage = resultData.getString
                        (RECEIVER_KEY_FAIL_MESSAGE);
                onTryingCommDeviceSettingFail(failMessage);
            }
        }
    }

    // Binder (for the caller of CommService)
    public class CommBinder extends android.os.Binder {
        public CommService getService() {
            return CommService.this;
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