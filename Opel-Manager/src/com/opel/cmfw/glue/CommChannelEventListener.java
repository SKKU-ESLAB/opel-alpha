package com.opel.cmfw.glue;

public interface CommChannelEventListener {
    public void onCommChannelStateChanged(int prevState, int newState);
    public void onReceivedRawMessage(String message, String filePath);
}