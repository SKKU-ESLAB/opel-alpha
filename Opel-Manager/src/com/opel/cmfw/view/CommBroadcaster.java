package com.opel.cmfw.view;

import android.content.Intent;

public class CommBroadcaster {
    public static void onInitializationResult(CommChannelService service, boolean
            isInitSuccess) {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_INITIALIZATION_RESULT);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_IS_INIT_SUCCESS,
                isInitSuccess);
        service.sendBroadcast(broadcastIntent);
    }

    public static void onWifiStateChanged(CommChannelService service, boolean
            isWifiOn) {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_WIFI_STATE_CHANGED);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_IS_WIFI_ON,
                isWifiOn);
        service.sendBroadcast(broadcastIntent);
    }

    public static void onBluetoothStateChanged(CommChannelService service, boolean
            isConnected) {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_BLUETOOTH_STATE_CHANGED);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_IS_CONNECTED,
                isConnected);
        service.sendBroadcast(broadcastIntent);
    }

    public static void onReceivedMessage(CommChannelService service, String message) {
        Intent broadcastIntent = new Intent();
        broadcastIntent.setAction(CommBroadcastReceiver.ACTION);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_EVENT_TYPE,
                CommBroadcastReceiver.EVENT_TYPE_ON_RECEIVED_MESSAGE);
        broadcastIntent.putExtra(CommBroadcastReceiver.KEY_MESSAGE,
                message);
        service.sendBroadcast(broadcastIntent);
    }
}