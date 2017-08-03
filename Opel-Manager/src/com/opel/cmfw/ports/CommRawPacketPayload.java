package com.opel.cmfw.ports;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;

public abstract class CommRawPacketPayload {
    abstract public byte[] toByteArray();

    abstract public short getBytesSize();
}

class CommPayloadMessageMetadata extends CommRawPacketPayload {
    // TODO: (on OPEL device side) add 'isFileAttached'
    public static final short kMessageHeaderSize = 8;
    private int mMessageDataLength;
    private int mIsFileAttached;

    public CommPayloadMessageMetadata(int messageDataLength, boolean isFileAttached) {
        this.mMessageDataLength = messageDataLength;
        this.mIsFileAttached = (isFileAttached) ? 1 : 0;
    }

    public byte[] toByteArray() {
        if (mMessageDataLength == 0) return null;
        ByteBuffer bb = ByteBuffer.allocate(this.getBytesSize());
        bb.putInt(mMessageDataLength);
        bb.putInt(mIsFileAttached);
        return bb.array();
    }

    public static CommPayloadMessageMetadata read(DataInputStream dataInputStream) throws IOException {
        int messageDataLength = dataInputStream.readInt();
        int isFileAttached = dataInputStream.readInt();
        CommPayloadMessageMetadata messageMetadata = new CommPayloadMessageMetadata(messageDataLength,
                (isFileAttached != 0));
        return messageMetadata;
    }

    public short getBytesSize() {
        return kMessageHeaderSize;
    }

    public int getMessageDataLength() {
        return this.mMessageDataLength;
    }

    public int getIsFileAttached() {
        return this.mIsFileAttached;
    }
}

class CommPayloadFileMetadata extends CommRawPacketPayload {
    // TODO: (on OPEL device side) shrink fields
    private int mFileSize;
    private char mFileNameLength;
    private char mFileName[];

    public CommPayloadFileMetadata(int fileSize, char fileNameLength, char[] fileName) {
        mFileSize = fileSize;
        mFileNameLength = fileNameLength;
        mFileName = fileName;
    }

    public byte[] toByteArray() {
        try {
            ByteBuffer bb = ByteBuffer.allocate(this.getBytesSize());
            bb.putInt(this.mFileSize);
            bb.putChar(this.mFileNameLength);
            bb.put(new String(this.mFileName).getBytes("UTF-8"));
            return bb.array();
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
            return null;
        }
    }

    public static CommPayloadFileMetadata read(DataInputStream dataInputStream) throws IOException {
        int fileSize = dataInputStream.readInt();
        char fileNameLength = dataInputStream.readChar();
        byte[] fileNameBytes = new byte[fileNameLength];
        dataInputStream.readFully(fileNameBytes, 0, fileNameLength);
        String fileNameStr = new String(fileNameBytes, "UTF-8");
        char[] fileName = fileNameStr.toCharArray();

        CommPayloadFileMetadata fileMetadata = new CommPayloadFileMetadata(fileSize, fileNameLength, fileName);
        return fileMetadata;
    }

    public short getBytesSize() {
        return (short) (4 + 1 + this.mFileNameLength);
    }

    public int getFileSize() {
        return this.mFileSize;
    }

    public char getSrcFileNameLength() {
        return this.mFileNameLength;
    }

    public char[] getSrcFileName() {
        return this.mFileName;
    }
}

class CommPayloadData extends CommRawPacketPayload {
    private short mSize;
    private byte mData[];

    public CommPayloadData(byte[] data, short size) {
        this.mData = data;
        this.mSize = size;
    }

    public byte[] toByteArray() {
        return this.mData;
    }

    public static CommPayloadData read(DataInputStream dataInputStream, short dataSize) throws
            IOException {
        byte[] data = new byte[dataSize];
        dataInputStream.readFully(data, 0, dataSize);
        CommPayloadData dataPayload = new CommPayloadData(data, dataSize);
        return dataPayload;
    }

    public short getBytesSize() {
        return this.mSize;
    }
}