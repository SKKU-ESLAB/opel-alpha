package com.opel.cmfw.view;

public interface CommChannelEventListener {
    // TODO: CMFW events
    public void onWifiDirectDeviceStateChanged(boolean isWifiOn);
    public void onCommChannelStateChanged(boolean isInitSuccess);
    public void onBluetoothDeviceStateChanged(boolean isConnected);
    public void onReceivedRawMessage(String message, String filePath);
}