package com.opel.cmfw.view;

public interface CommEventListener {
    // TODO: CMFW events
    public void onWifiDirectStateChanged(boolean isWifiOn);
    public void onInitializationResult(boolean isInitSuccess);
    public void onBluetoothStateChanged(boolean isConnected);
    public void onReceivedMessage(String message);
}
