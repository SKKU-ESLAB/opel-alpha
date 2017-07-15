package com.opel.cmfw.view;

public interface CommEventListener {
    // TODO: CMFW events
    public void onWifiDirectDeviceStateChanged(boolean isWifiOn);
    public void onCommChannelStateChanged(boolean isInitSuccess);
    public void onBluetoothDeviceStateChanged(boolean isConnected);
    public void onReceivedMessage(String message, String filePath);
}