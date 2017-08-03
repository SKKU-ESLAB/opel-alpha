package com.opel.cmfw.ports;

import java.io.DataInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class CommRawPacketHeader {
    public static final short kMaxPacketSize = 1008;
    public static final short kPacketHeaderSize = 8;
    public static final short kMaxPacketPayloadSize = kMaxPacketSize - kPacketHeaderSize;

    private byte mHeaderId;
    private byte mHeaderFlag;
    private short mPayloadSize;
    private int mCurrOffset;

    public CommRawPacketHeader(byte headerId, short payloadSize, int currOffset, boolean isFile,
                               boolean isData, boolean isEndData, boolean isMetadata) {
        byte headerFlag = 0x00;

        //0x80 : file, 0x40 : data, 0x20 : end data, 0x10 : metadata
        if (isFile) headerFlag = (byte) (headerFlag | 0x80);
        if (isData) headerFlag = (byte) (headerFlag | 0x40);
        if (isEndData) headerFlag = (byte) (headerFlag | 0x20);
        if (isMetadata) headerFlag = (byte) (headerFlag | 0x10);

        this.mHeaderId = headerId;
        this.mHeaderFlag = headerFlag;
        this.mPayloadSize = payloadSize;
        this.mCurrOffset = currOffset;
    }

    public CommRawPacketHeader(byte headerId, short payloadSize, int currOffset, byte headerFlag) {
        this.mHeaderId = headerId;
        this.mHeaderFlag = headerFlag;
        this.mPayloadSize = payloadSize;
        this.mCurrOffset = currOffset;
    }

    public byte[] toByteArray() {
        ByteBuffer bb = ByteBuffer.allocate(this.getBytesSize());
        bb.put(mHeaderId);
        bb.put(mHeaderFlag);
        bb.putShort(mPayloadSize);
        bb.putInt(mCurrOffset);
        return bb.array();
    }

    public static CommRawPacketHeader read(DataInputStream dataInputStream) throws IOException {
        byte headerId = dataInputStream.readByte();
        byte headerFlag = dataInputStream.readByte();
        short payloadSize = dataInputStream.readShort();
        int currOffset = dataInputStream.readInt();
        CommRawPacketHeader header = new CommRawPacketHeader(headerId, payloadSize, currOffset,
                headerFlag);
        return header;
    }

    public byte getHeaderId() {
        return this.mHeaderId;
    }

    public byte getHeaderInfo() {
        return this.mHeaderFlag;
    }

    public boolean getFlagIsFile() {
        return ((this.mHeaderFlag & 0x80) != 0);
    }

    public boolean getFlagIsData() {
        return ((this.mHeaderFlag & 0x40) != 0);
    }

    public boolean getFlagIsEndData() {
        return ((this.mHeaderFlag & 0x20) != 0);
    }

    public boolean getFlagIsMetadata() {
        return ((this.mHeaderFlag & 0x10) != 0);
    }

    public short getPayloadSize() {
        return this.mPayloadSize;
    }

    public int getCurrOffset() {
        return this.mCurrOffset;
    }

    public short getBytesSize() {
        return kPacketHeaderSize;
    }
}