package com.opel.cmfw.glue;

public interface CommChannelEventListener {
    public void onWifiDirectDeviceStateChanged(boolean isConnected);
    public void onCommChannelStateChanged(int prevState, int newState);
    public void onBluetoothDeviceStateChanged(boolean isConnected);
    public void onReceivedRawMessage(String message, String filePath);
}