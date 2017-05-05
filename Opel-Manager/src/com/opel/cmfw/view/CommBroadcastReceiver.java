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

    public static final String EVENT_TYPE_ON_WIFI_STATE_CHANGED =
            "onWifiStateChanged";
    public static final String KEY_IS_WIFI_ON = "isWifiOn";

    public static final String EVENT_TYPE_ON_INITIALIZATION_RESULT =
            "onInitializationResult";
    public static final String KEY_IS_INIT_SUCCESS = "isInitSuccess";

    public static final String EVENT_TYPE_ON_BLUETOOTH_STATE_CHANGED =
            "onBluetoothStateChanged";
    public static final String KEY_IS_CONNECTED = "isConnected";

    public static final String EVENT_TYPE_ON_RECEIVED_MESSAGE =
            "onReceivedMessage";
    public static final String KEY_MESSAGE = "message";

    private CommEventListener mListener;

    public CommBroadcastReceiver(CommEventListener listener) {
        this.mListener = listener;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action.compareTo(ACTION) == 0) {
            String eventType = intent.getStringExtra(KEY_EVENT_TYPE);
            if (eventType.compareTo(EVENT_TYPE_ON_INITIALIZATION_RESULT) == 0) {
                boolean isInitSuccess = intent.getBooleanExtra
                        (KEY_IS_INIT_SUCCESS, false);
                this.mListener.onInitializationResult(isInitSuccess);
            } else if (eventType.compareTo(EVENT_TYPE_ON_WIFI_STATE_CHANGED)
                    == 0) {
                boolean isWifiOn = intent.getBooleanExtra(KEY_IS_WIFI_ON,
                        false);
                this.mListener.onWifiDirectStateChanged(isWifiOn);
            } else if (eventType.compareTo
                    (EVENT_TYPE_ON_BLUETOOTH_STATE_CHANGED) == 0) {
                boolean isConnected = intent.getBooleanExtra
                        (KEY_IS_CONNECTED, false);
                this.mListener.onBluetoothStateChanged(isConnected);
            } else if (eventType.compareTo(EVENT_TYPE_ON_RECEIVED_MESSAGE) ==
                    0) {
                String message = intent.getStringExtra(KEY_MESSAGE);
                this.mListener.onReceivedMessage(message);
            }
        }
    }
}

class CommBroadcaster {
    public static void onInitializationResult(CommService service, boolean
            isInitSuccess) {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_INITIALIZATION_RESULT);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_IS_INIT_SUCCESS,
                isInitSuccess);
        service.sendBroadcast(broadcastIntent);
    }

    public static void onWifiStateChanged(CommService service, boolean
            isWifiOn) {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_WIFI_STATE_CHANGED);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_IS_WIFI_ON,
                isWifiOn);
        service.sendBroadcast(broadcastIntent);
    }

    public static void onBluetoothStateChanged(CommService service, boolean
            isConnected) {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_BLUETOOTH_STATE_CHANGED);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_IS_CONNECTED,
                isConnected);
        service.sendBroadcast(broadcastIntent);
    }

    public static void onReceivedMessage(CommService service, String message) {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_RECEIVED_MESSAGE);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_MESSAGE,
                message);
        service.sendBroadcast(broadcastIntent);
    }
}