package com.opel.cmfw.controller;

import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;

public class WifiDirectCommPort extends CommPort {
    static private String TAG = "WifiDirectCommPort";
    static private int kTimeoutMillisecs = 1500;

    private boolean mIsOpened = false;
    private Socket mSocket = null;

    private int mTcpPortNum;

    public WifiDirectCommPort(int tcpPortNum) {
        this.mTcpPortNum = tcpPortNum;
    }

    public boolean isOpened() {
        return this.mIsOpened;
    }

    public boolean open(String ipAddress) {
        this.mSocket = new Socket();

        try {
            this.mSocket.bind(null);
            Log.d(TAG, "Try Socket connectChannel");
            this.mSocket.connect((new InetSocketAddress(ipAddress, this.mTcpPortNum)),
                    kTimeoutMillisecs);
        } catch (IOException e) {
            e.printStackTrace();
            this.mSocket = null;
            return false;
        }

        if (this.mSocket.isConnected()) {
            Log.d(TAG, "Connected to device");
            this.mIsOpened = true;
            return true;
        } else {
            return false;
        }
    }

    public void close() {
        if (this.isOpened()) {
            try {
                this.mSocket.close();
                Log.d(TAG, "WFD Port closed");
            } catch (IOException e) {
                e.printStackTrace();
            }
            this.mSocket = null;
        }

        this.mIsOpened = false;
    }

    protected OutputStream getOutputStream() {
        if (this.isOpened()) {
            OutputStream outputStream;
            try {
                outputStream = this.mSocket.getOutputStream();
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
                inputStream = this.mSocket.getInputStream();
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