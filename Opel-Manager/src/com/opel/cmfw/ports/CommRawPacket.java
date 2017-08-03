package com.opel.cmfw.ports;

import java.io.File;

public class CommRawPacket {
    private CommRawPacketHeader mRawPacketHeader;
    private CommRawPacketPayload mRawPacketPayload;

    private CommRawPacket(CommRawPacketHeader header, CommRawPacketPayload payload) {
        this.mRawPacketHeader = header;
        this.mRawPacketPayload = payload;
    }

    static public CommRawPacket makeMessageMetadataPacket(byte headerId, int totalDataSize,
                                                          boolean isFileAttached) {
        CommPayloadMessageMetadata payload = new CommPayloadMessageMetadata(totalDataSize, isFileAttached);
        CommRawPacketHeader header = new CommRawPacketHeader(headerId, payload.getBytesSize(), 0,
                false, false, false, true);

        return new CommRawPacket(header, payload);
    }

    static public CommRawPacket makeFileMetadataPacket(byte headerId, File srcFile) {
        CommPayloadFileMetadata payload = new CommPayloadFileMetadata((int) srcFile.length(), (char) srcFile
                .getName().length(), srcFile.getName().toCharArray());
        CommRawPacketHeader header = new CommRawPacketHeader(headerId, payload.getBytesSize(), 0,
                false, false, false, true);
        return new CommRawPacket(header, payload);
    }

    static public CommRawPacket makeDataPacket(byte headerId, byte[] data, int currOffset, short
            size, boolean isEnd, boolean isFile) {
        CommPayloadData payload = new CommPayloadData(data, size);
        CommRawPacketHeader header = new CommRawPacketHeader(headerId, (byte) payload
                .getBytesSize(), currOffset, isFile, !isFile, isEnd, false);

        return new CommRawPacket(header, payload);
    }

    public byte[] toByteArray() {
        int resLength = this.getBytesSize();
        byte resBytes[] = new byte[resLength];
        byte headerBytes[] = null;
        byte payloadBytes[] = null;

        // Payload header
        headerBytes = this.mRawPacketHeader.toByteArray();
        if (headerBytes == null) return null;
        System.arraycopy(headerBytes, 0, resBytes, 0, this.mRawPacketHeader.getBytesSize());

        // Payload
        payloadBytes = this.mRawPacketPayload.toByteArray();
        if (headerBytes == null) return null;
        System.arraycopy(payloadBytes, 0, resBytes, this.mRawPacketHeader.getBytesSize(), this
                .mRawPacketPayload.getBytesSize());

        return resBytes;
    }

    public int getBytesSize() {
        return this.mRawPacketHeader.getBytesSize() + this.mRawPacketPayload.getBytesSize();
    }
}