package com.opel.cmfw.controller;

public interface CommPortListener {
    public void onReceivingRawMessage(byte[] data, int messageDataLength, String filePath);
}