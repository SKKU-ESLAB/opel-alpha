package com.opel.cmfw.ports.bluetooth;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;

import com.opel.cmfw.ports.CommPort;
import com.opel.cmfw.ports.bluetooth.socket.BluetoothSocketWrapper;
import com.opel.cmfw.ports.bluetooth.socket.FallbackBluetoothSocket;
import com.opel.cmfw.ports.bluetooth.socket.FallbackException;
import com.opel.cmfw.ports.bluetooth.socket.NativeBluetoothSocket;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

public class BluetoothCommPort extends CommPort {
    static private String TAG = "BluetoothCommPort";
    private BluetoothSocket mBluetoothSocket = null;
    private UUID mUUID;
    private boolean mIsOpened = false;
    private BluetoothDevice mBluetoothDevice = null;

    public BluetoothCommPort(String portName, UUID uuid) {
        super(portName);
        this.mUUID = uuid;
    }

    public boolean isOpened() {
        return this.mIsOpened;
    }

    public void setBluetoothDevice(BluetoothDevice bluetoothDevice) {
        this.mBluetoothDevice = bluetoothDevice;
    }

    public boolean open() {
        boolean isSucceed = false;
        if (this.isOpened()) return true;

        if (this.mBluetoothDevice == null) {
            Log.w(TAG, "There is no bluetooth device!");
            return false;
        }

        // RedCarrottt: Fix Bluetooth connection failure bug (Issue #103)
        BluetoothSocketWrapper newBluetoothSocket = null;
        try {
            BluetoothSocket rawBtSocket = this.mBluetoothDevice.createRfcommSocketToServiceRecord
                    (mUUID);
            newBluetoothSocket = new NativeBluetoothSocket(rawBtSocket);
            newBluetoothSocket.connect();
            isSucceed = true;
        } catch (IOException e) {
            try {
                newBluetoothSocket = new FallbackBluetoothSocket(newBluetoothSocket
                        .getUnderlyingSocket());
                Thread.sleep(500);
                newBluetoothSocket.connect();
                isSucceed = true;
            } catch (FallbackException e1) {
                Log.w(TAG, "Could not connectChannel FallbackBluetoothSocket class");
                isSucceed = false;
            } catch (InterruptedException e1) {
                Log.w(TAG, e1.getMessage());
                isSucceed = false;
            } catch (IOException e1) {
                Log.w(TAG, "Fallback failed. Cancelling it.");
                isSucceed = false;
            }
        }

        if (isSucceed && newBluetoothSocket.getUnderlyingSocket().isConnected()) {
            Log.d(TAG, "Connected");
        } else {
            Log.d(TAG, "Connection failed");
            newBluetoothSocket = null;
        }

        if (newBluetoothSocket == null) {
            isSucceed = false;
            Log.w(TAG, "Cannot get BluetoothSocket");
        } else {
            this.mBluetoothSocket = newBluetoothSocket.getUnderlyingSocket();
        }

        if (isSucceed) this.mIsOpened = true;
        return isSucceed;
    }

    public void close() {
        if (this.isOpened()) {
            try {
                if (this.mBluetoothSocket != null) this.mBluetoothSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
                Log.d(TAG, "Close failed");
            }
            this.mBluetoothSocket = null;
        }

        this.mIsOpened = false;
    }

    protected OutputStream getOutputStream() {
        if (this.isOpened()) {
            OutputStream outputStream;
            try {
                outputStream = this.mBluetoothSocket.getOutputStream();
            } catch (IOException e) {
                e.printStackTrace();
                outputStream = null;
            }
            return outputStream;
        } else {
            return null;
        }
    }

    protected InputStream getInputStream() {
        if (this.isOpened()) {
            InputStream inputStream;
            try {
                inputStream = this.mBluetoothSocket.getInputStream();
            } catch (IOException e) {
                e.printStackTrace();
                inputStream = null;
            }
            return inputStream;
        } else {
            return null;
        }
    }
}