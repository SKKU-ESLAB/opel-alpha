package com.opel.cmfw.view;

import android.app.Activity;
import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.ResultReceiver;
import android.util.Log;

import java.util.Set;

import static android.content.ContentValues.TAG;

public class BluetoothDeviceController {
    private Service mService;

    private ConnectProcedure mConnectProcedureProcedure = new ConnectProcedure();
    private State mState = new State();

    public BluetoothDeviceController(Service service) {
        this.mService = service;
    }

    public boolean isConnected() {
        return this.mState.isConnected();
    }

    public void connect(ConnectingResultListener connectingResultListener) {
        this.mConnectProcedureProcedure.start(connectingResultListener);
    }

    public void disconnect() {
        // not yet implemented
        this.mState.transitToDisconnected();
    }

    interface ConnectingResultListener {
        public void onConnectingBluetoothDeviceSuccess(BluetoothDevice bluetoothDevice);

        public void onConnectingBluetoothDeviceFail();
    }

    class ConnectProcedure {
        private ConnectingResultListener mConnectingResultListener = null;

        // ConnectProcedure to Bluetooth device
        public void start(ConnectingResultListener connectingResultListener) {
            this.mConnectingResultListener = connectingResultListener;

            Intent bluetoothConnectorIntent = new Intent(mService, BluetoothConnectorActivity
                    .class);
            bluetoothConnectorIntent.putExtra(BluetoothConnectorActivity.INTENT_KEY_RECEIVER, new
                    BluetoothConnectorResultReceiver());
            bluetoothConnectorIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            mService.startActivity(bluetoothConnectorIntent);
        }

        class BluetoothConnectorResultReceiver extends ResultReceiver {
            static final String RECEIVER_KEY_FAIL_MESSAGE = "FailMessage";
            static final String RECEIVER_KEY_BT_ADDRESS = "BluetooothDeviceAddress";

            // Receiver from BluetoothConnectorActivity
            private BluetoothConnectorResultReceiver() {
                super(null);
            }

            @Override
            protected void onReceiveResult(int resultCode, Bundle resultData) {
                if (resultCode == Activity.RESULT_OK) {
                    String bluetoothAddress = resultData.getString(RECEIVER_KEY_BT_ADDRESS);
                    if (bluetoothAddress != null && !bluetoothAddress.isEmpty()) {
                        // Success
                        Log.d(TAG, "Connecting success");
                        onSuccess(bluetoothAddress);
                    } else {
                        // Fail
                        Log.d(TAG, "Connecting fail");
                        onFail();
                    }
                } else {
                    // Fail
                    Log.d(TAG, "Connecting fail");
                    onFail();
                }
            }
        }

        private void onSuccess(String bluetoothAddress) {
            BluetoothDevice bluetoothDevice = findBondedBluetoothDevice(bluetoothAddress);

            // State transition
            mState.transitToConnected(bluetoothDevice);

            // Notify result
            this.mConnectingResultListener.onConnectingBluetoothDeviceSuccess(bluetoothDevice);
        }

        private void onFail() {
            // State transition
            mState.transitToDisconnected();

            // Notify result
            this.mConnectingResultListener.onConnectingBluetoothDeviceFail();
        }

        // Utility functions
        private BluetoothDevice findBondedBluetoothDevice(String bluetoothAddress) {
            if (bluetoothAddress == null || bluetoothAddress.isEmpty()) {
                return null;
            }

            Set<BluetoothDevice> bluetoothDevices = BluetoothAdapter.getDefaultAdapter()
                    .getBondedDevices();
            if (bluetoothDevices.size() > 0) {
                for (BluetoothDevice bluetoothDevice : bluetoothDevices) {
                    String deviceName = bluetoothDevice.getName();
                    String deviceAddress = bluetoothDevice.getAddress();
                    if (deviceAddress.compareTo(bluetoothAddress) == 0) {
                        return bluetoothDevice;
                    }
                }
            }
            return null;
        }
    }

    private class State {
        private boolean mIsConnected = false;

        private BluetoothDeviceStatusReceiver mBluetoothDeviceStatusReceiver = null;

        public boolean isConnected() {
            return this.mIsConnected;
        }

        public void transitToConnected(BluetoothDevice bluetoothDevice) {
            this.mIsConnected = true;

            // Start to watch Bluetooth device's status
            this.startToWatchDeviceState(bluetoothDevice);

            // Broadcast Bluetooth device connection event via CommBroadcaster
            CommBroadcaster.onBluetoothDeviceStateChanged(mService, this.mIsConnected);
        }

        public void transitToDisconnected() {
            this.mIsConnected = false;

            // Stop to watch Bluetooth device's status
            this.stopToWatchDeviceState();

            // Broadcast Bluetooth device disconnection event via CommBroadcaster
            CommBroadcaster.onBluetoothDeviceStateChanged(mService, this.mIsConnected);
        }

        private void startToWatchDeviceState(BluetoothDevice bluetoothDevice) {
            IntentFilter filter = new IntentFilter();
            filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
            this.mBluetoothDeviceStatusReceiver = new BluetoothDeviceStatusReceiver
                    (bluetoothDevice);
            mService.registerReceiver(this.mBluetoothDeviceStatusReceiver, filter);
        }

        private void stopToWatchDeviceState() {
            if (this.mBluetoothDeviceStatusReceiver != null) {
                mService.unregisterReceiver(this.mBluetoothDeviceStatusReceiver);
                this.mBluetoothDeviceStatusReceiver = null;
            }
        }

        // Receive Bluetooth device disconnection event from Android Bluetooth framework
        class BluetoothDeviceStatusReceiver extends BroadcastReceiver {
            private BluetoothDevice mBindingBluetoothDevice;

            public BluetoothDeviceStatusReceiver(BluetoothDevice bindingBluetoothDevice) {
                this.mBindingBluetoothDevice = bindingBluetoothDevice;
            }

            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.compareTo(action) == 0) {
                    BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice
                            .EXTRA_DEVICE);
                    if (this.mBindingBluetoothDevice != null) {
                        if (device.getBondState() != BluetoothDevice.BOND_BONDED) {
                            transitToDisconnected();
                        }
                    }
                }
            }
        }
    }
}