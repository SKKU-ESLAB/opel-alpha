package com.opel.cmfw.devicecontrollers.bluetooth;

public interface BluetoothDeviceStateListener {
    public void onBluetoothDeviceStateChanged(boolean isConnected);
}
