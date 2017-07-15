package com.opel.cmfw.view;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/* How to use CommBroadcastReceiver
class A implements CommBroadcastReceiver {
    private CommBroadcastReceiver mReceiver;

    initFunction() {
        IntentFilter broadcastIntentFilter = new IntentFilter();
        broadcastIntentFilter.addAction(CommBroadcastReceiver.ACTION);
        this.mReceiver = new CommBroadcastReceiver(this);
        this.registerReceiver(this.mReceiver, broadcastIntentFilter);
    }

    exitFunction() {
        this.unregisterReceiver(this.mReceiver);
    }

    // Implement CommBroadcastReceiver's member functions
}
 */

public class CommBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = "CommBroadcastReceiver";
    public static final String ACTION = "com.opel.cmfw.broadcastreceiver";
    public static final String KEY_EVENT_TYPE = "eventType";

    public static final String EVENT_TYPE_ON_WIFI_DIRECT_DEVICE_STATE_CHANGED =
            "onWifiDirectDeviceStateChanged";
    public static final String KEY_IS_WIFI_ON = "isWifiOn";

    public static final String EVENT_TYPE_ON_COMM_CHANNEL_STATE_CHANGED =
            "onCommChannelStateChanged";
    public static final String KEY_IS_COMM_CHANNEL_CONNECTED = "isCommChannelConnected";

    public static final String EVENT_TYPE_ON_BLUETOOTH_DEVICE_STATE_CHANGED =
            "onBluetoothDeviceStateChanged";
    public static final String KEY_IS_CONNECTED = "isChannelConnected";

    public static final String EVENT_TYPE_ON_RECEIVED_MESSAGE = "onReceivedMessage";
    public static final String KEY_MESSAGE = "message";
    public static final String KEY_FILE_PATH = "filePath";

    private CommEventListener mListener;

    public CommBroadcastReceiver(CommEventListener listener) {
        this.mListener = listener;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action.compareTo(ACTION) == 0) {
            String eventType = intent.getStringExtra(KEY_EVENT_TYPE);
            if (eventType.compareTo(EVENT_TYPE_ON_COMM_CHANNEL_STATE_CHANGED) == 0) {
                boolean isInitSuccess = intent.getBooleanExtra
                        (KEY_IS_COMM_CHANNEL_CONNECTED, false);
                this.mListener.onCommChannelStateChanged(isInitSuccess);
            } else if (eventType.compareTo(EVENT_TYPE_ON_WIFI_DIRECT_DEVICE_STATE_CHANGED)
                    == 0) {
                boolean isWifiOn = intent.getBooleanExtra(KEY_IS_WIFI_ON,
                        false);
                this.mListener.onWifiDirectDeviceStateChanged(isWifiOn);
            } else if (eventType.compareTo
                    (EVENT_TYPE_ON_BLUETOOTH_DEVICE_STATE_CHANGED) == 0) {
                boolean isConnected = intent.getBooleanExtra
                        (KEY_IS_CONNECTED, false);
                this.mListener.onBluetoothDeviceStateChanged(isConnected);
            } else if (eventType.compareTo(EVENT_TYPE_ON_RECEIVED_MESSAGE) ==
                    0) {
                String message = intent.getStringExtra(KEY_MESSAGE);
                String filePath = intent.getStringExtra(KEY_FILE_PATH);
                this.mListener.onReceivedMessage(message, filePath);
            }
        }
    }
}