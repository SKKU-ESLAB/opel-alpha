package com.opel.cmfw.devicecontrollers.bluetooth;

import android.bluetooth.BluetoothDevice;

public interface BluetoothConnectingResultListener {
    public void onConnectingBluetoothDeviceSuccess(BluetoothDevice bluetoothDevice);

    public void onConnectingBluetoothDeviceFail();
}