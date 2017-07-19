package com.opel.cmfw.view;

public interface CommChannelEventListener {
    public void onWifiDirectDeviceStateChanged(boolean isConnected);
    public void onCommChannelStateChanged(int commChannelState);
    public void onBluetoothDeviceStateChanged(boolean isConnected);
    public void onReceivedRawMessage(String message, String filePath);
}