package com.opel.cmfw.view;

import android.app.Service;
import android.content.Intent;

public class CommBroadcaster {
    public static void onCommChannelState(Service service, int
            commChannelState) {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_COMM_CHANNEL_STATE_CHANGED);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_COMM_CHANNEL_STATE,
                commChannelState);
        service.sendBroadcast(broadcastIntent);
    }

    public static void onWifiDirectDeviceStateChanged(Service service, boolean
            isWifiDirectOn) {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_WIFI_DIRECT_DEVICE_STATE_CHANGED);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_IS_WIFI_ON,
                isWifiDirectOn);
        service.sendBroadcast(broadcastIntent);
    }

    public static void onBluetoothDeviceStateChanged(Service service, boolean
            isConnected) {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_BLUETOOTH_DEVICE_STATE_CHANGED);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_IS_CONNECTED,
                isConnected);
        service.sendBroadcast(broadcastIntent);
    }

    public static void onReceivedRawMessage(Service service,
                                            String message, String filePath) {
        // TODO: add file
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_RECEIVED_RAW_MESSAGE);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_MESSAGE,
                message);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_FILE_PATH,
                filePath);
        service.sendBroadcast(broadcastIntent);
    }
}