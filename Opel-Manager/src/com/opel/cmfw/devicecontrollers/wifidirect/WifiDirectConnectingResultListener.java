package com.opel.cmfw.devicecontrollers.wifidirect;

public interface WifiDirectConnectingResultListener {
    public void onConnectingWifiDirectDeviceSuccess();

    public void onConnectingWifiDirectDeviceFail();
}