package com.opel.cmfw.glue;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.opel.cmfw.service.CommChannelService;

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

abstract public class CommBroadcastReceiver extends BroadcastReceiver {
    private static final String TAG = "CommBroadcastReceiver";
    public static final String ACTION = "com.opel.cmfw.broadcastreceiver";
    public static final String KEY_EVENT_TYPE = "eventType";

    public static final String EVENT_TYPE_ON_COMM_CHANNEL_STATE_CHANGED =
            "onCommChannelStateChanged";
    public static final String KEY_COMM_CHANNEL_PREV_STATE = "commChannelPrevState";
    public static final String KEY_COMM_CHANNEL_NEW_STATE = "commChannelNewState";

    public static final String EVENT_TYPE_ON_RECEIVED_RAW_MESSAGE = "onReceivedRawMessage";
    public static final String KEY_MESSAGE = "message";
    public static final String KEY_FILE_PATH = "filePath";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action.compareTo(ACTION) == 0) {
            String eventType = intent.getStringExtra(KEY_EVENT_TYPE);
            if (eventType.compareTo(EVENT_TYPE_ON_COMM_CHANNEL_STATE_CHANGED) == 0) {
                int prevState = intent.getIntExtra(KEY_COMM_CHANNEL_PREV_STATE,
                        CommChannelService.STATE_DISCONNECTED);
                int newState = intent.getIntExtra(KEY_COMM_CHANNEL_NEW_STATE,
                        CommChannelService.STATE_DISCONNECTED);
                this.onCommChannelStateChanged(prevState, newState);
            } else if (eventType.compareTo(EVENT_TYPE_ON_RECEIVED_RAW_MESSAGE) == 0) {
                String message = intent.getStringExtra(KEY_MESSAGE);
                String filePath = intent.getStringExtra(KEY_FILE_PATH);
                this.onReceivedRawMessage(message, filePath);
            }
        }
    }

    abstract public void onCommChannelStateChanged(int prevState, int newState);
    abstract public void onReceivedRawMessage(String message, String filePath);
}