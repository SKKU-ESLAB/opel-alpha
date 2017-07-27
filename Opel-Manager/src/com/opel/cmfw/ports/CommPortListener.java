package com.opel.cmfw.ports;

public interface CommPortListener {
    public void onReceivingRawMessage(byte[] data, int messageDataLength, String filePath);
    public void onSuddenlyClosed(CommPort port);
}