package com.opel.cmfw.ports;

import android.util.Log;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;

abstract public class CommPort {
    static private String TAG = "CommPort";
    protected String mPortName;

    private byte mPresentHeaderId = 0;

    private ListeningThread mListeningThread;
    private boolean mIsListeningThreadOn;
    private CommPortListener mListener;

    private String mDownloadFilePath;

    public CommPort(String portName) {
        this.mPortName = portName;
    }

    abstract public boolean isOpened();

    abstract public void close();

    abstract protected OutputStream getOutputStream();

    abstract protected InputStream getInputStream();

    public void runListeningThread(CommPortListener listener, String downloadFilePath) {
        this.mListener = listener;
        this.mDownloadFilePath = downloadFilePath;

        this.mIsListeningThreadOn = true;

        this.mListeningThread = new ListeningThread();
        this.mListeningThread.start();
    }

    public void stopListeningThread() {
        this.mIsListeningThreadOn = false;
    }

    class ListeningThread extends Thread {
        @Override
        public void run() {
            listenRawMessage();
        }
    }

    private void suddenlyClose(String reasonMessage) {
        Log.e(TAG, "[" + this.mPortName + "] Suddenly closed: " + reasonMessage);
        this.close();

        this.mListener.onSuddenlyClosed(this);
    }

    public void listenRawMessage() {
        InputStream inputStream = this.getInputStream();
        if (inputStream == null) return;
        DataInputStream dataInputStream = new DataInputStream(inputStream);

        final int kMessageMetadata = 1;
        final int kMessageData = 2;
        final int kFileMetadata = 3;
        final int kFileData = 4;

        boolean isMessageCompleted = false;
        int expectedPacketType = kMessageMetadata;
        CommMessageMetadata messageMetadata = null;
        CommFileMetadata fileMetadata = null;
        byte[] totalMessageData = null;
        BufferedOutputStream bufferedOutputStream = null;
        int loadedBytesSize = 0;

        while (this.mIsListeningThreadOn) {
            try {
                // Read header
                CommRawPacketHeader header = CommRawPacketHeader.read(dataInputStream);
                if (!header.getFlagIsMetadata()) {
                    // Not expected
                    return;
                }

                // Check if header is same as expected and Read payload
                switch (expectedPacketType) {
                    case kMessageMetadata:
                        // Message metadata is expected
                        if (!header.getFlagIsMetadata()) return; // not expected
                        messageMetadata = CommMessageMetadata.read(dataInputStream);

                        totalMessageData = new byte[messageMetadata.getMessageDataLength()];
                        expectedPacketType = kMessageData;
                        break;

                    case kMessageData:
                        // Message data is expected
                        if (!header.getFlagIsData()) return; // not expected
                        CommDataPayload messageData = CommDataPayload.read(dataInputStream,
                                header.getPayloadSize());
                        byte[] messageDataBytes = messageData.toByteArray();
                        if (messageDataBytes == null) {
                            this.suddenlyClose("sendRawMessage: Failed to make message data " +
                                    "packet");
                        }
                        System.arraycopy(messageDataBytes, 0, totalMessageData, loadedBytesSize,
                                messageData.getBytesSize());
                        loadedBytesSize += messageData.getBytesSize();
                        if (header.getFlagIsEndData()) {
                            if (messageMetadata.getIsFileAttached() != 0) {
                                expectedPacketType = kFileMetadata;
                            } else {
                                // Message without file completed
                                expectedPacketType = kMessageMetadata;
                                isMessageCompleted = true;
                            }
                        } else {
                            expectedPacketType = kMessageData;
                        }
                        break;

                    case kFileMetadata:
                        // File metadata is expected
                        if (!header.getFlagIsMetadata()) return; // not expected
                        fileMetadata = CommFileMetadata.read(dataInputStream);

                        // Open file
                        String fileName = String.copyValueOf(fileMetadata.getSrcFileName());
                        File fileToWrite = new File(this.mDownloadFilePath, fileName);
                        bufferedOutputStream = null;
                        try {
                            bufferedOutputStream = new BufferedOutputStream(new FileOutputStream
                                    (fileToWrite));
                        } catch (IOException e) {
                            this.suddenlyClose("listenRawMessage: Failed to get stream for " +
                                    "downloading attached file");
                            return;
                        }

                        // Expect next packet
                        expectedPacketType = kFileData;
                        break;

                    case kFileData:
                        // File data is expected
                        if (!header.getFlagIsFile()) return; // not expected
                        CommDataPayload fileData = CommDataPayload.read(dataInputStream, header
                                .getPayloadSize());

                        // Write to white
                        byte[] fileDataBytes = fileData.toByteArray();
                        if (fileDataBytes == null) {
                            this.suddenlyClose("sendRawMessage: Failed to make file data packet");
                        }
                        bufferedOutputStream.write(fileDataBytes, 0, fileData.getBytesSize());

                        // Expect next packet
                        if (header.getFlagIsEndData()) {
                            // Message with file completed
                            expectedPacketType = kMessageMetadata;
                            isMessageCompleted = true;
                        } else {
                            expectedPacketType = kFileData;
                        }
                        break;
                }
            } catch (IOException e) {
                this.suddenlyClose("listenRawMessage: I/O exception");
                return;
            }

            if (isMessageCompleted) {
                // Notify the message
                if (fileMetadata != null)
                    this.mListener.onReceivingRawMessage(totalMessageData, messageMetadata
                            .getMessageDataLength(), String.copyValueOf(fileMetadata
                            .getSrcFileName()));
                else
                    this.mListener.onReceivingRawMessage(totalMessageData, messageMetadata
                            .getMessageDataLength(), null);

                // Initialize variables
                expectedPacketType = kMessageMetadata;
                messageMetadata = null;
                fileMetadata = null;
                totalMessageData = null;
                loadedBytesSize = 0;
            }
        }
    }

    public int sendRawMessage(byte[] dataBytes, int messageDataLength, File file) {
        if (!this.isOpened()) return -1;

        // Get DataOutputStream
        OutputStream outputStream = this.getOutputStream();
        if (outputStream == null) return -1;
        DataOutputStream dataOutputStream = new DataOutputStream(outputStream);

        // Send message metadata and message data
        {
            // Send message metadata
            this.mPresentHeaderId++;
            CommRawPacket messageMetadataPacket = CommRawPacket.makeMessageMetadataPacket(this
                    .mPresentHeaderId, messageDataLength, (file != null));
            byte[] messageMetadataBytes = messageMetadataPacket.toByteArray();
            if (messageMetadataBytes == null) {
                this.suddenlyClose("sendRawMessage: Failed to make message metadata packet");
            }
            try {
                dataOutputStream.write(messageMetadataBytes, 0, messageMetadataPacket
                        .getBytesSize());
            } catch (Exception e) {
                this.suddenlyClose("sendRawMessage: Header write failed");
                return -1;
            }

            // Send message data (split by limited-size packets)
            int sentBytes = 0;
            while (sentBytes < messageDataLength) {
                // Make message data packet
                CommRawPacket messageDataPacket;
                short messageDataPayloadSize;
                boolean isEnd;
                if (CommRawPacketHeader.kMaxPacketPayloadSize < messageDataLength - sentBytes) {
                    // Not end data
                    messageDataPayloadSize = CommRawPacketHeader.kMaxPacketPayloadSize;
                    isEnd = false;
                } else {
                    // end data
                    messageDataPayloadSize = (short) (messageDataLength - sentBytes);
                    isEnd = true;
                }
                byte[] messageDataPayloadBytes = new byte[messageDataPayloadSize];
                System.arraycopy(dataBytes, sentBytes, messageDataPayloadBytes, 0,
                        messageDataPayloadSize);
                messageDataPacket = CommRawPacket.makeDataPacket(this.mPresentHeaderId,
                        messageDataPayloadBytes, sentBytes, messageDataPayloadSize, isEnd, false);
                Log.d(TAG, "[" + this.mPortName + "] sendRawMessage(message): mPayloadSize = " +
                        messageDataPayloadSize);

                // Send
                byte[] messageDataPacketBytes = messageDataPacket.toByteArray();
                if (messageDataPacketBytes == null) {
                    this.suddenlyClose("sendRawMessage: Failed to make message data packet");
                }
                try {
                    dataOutputStream.write(messageDataPacketBytes, 0, messageDataPacket
                            .getBytesSize());
                } catch (Exception e) {
                    this.suddenlyClose("sendRawMessage: Header write failed");
                    return -1;
                }
                sentBytes += messageDataPayloadSize;
            }
        }

        // If file is attached, send file metadata and file data
        if (file != null) {
            // Send file metadata
            CommRawPacket fileMetadataPacket = CommRawPacket.makeFileMetadataPacket(this
                    .mPresentHeaderId, file);
            byte[] fileMetadataBytes = fileMetadataPacket.toByteArray();
            if (fileMetadataBytes == null) {
                this.suddenlyClose("sendRawMessage: Failed to make file metadata packet");
            }
            try {
                dataOutputStream.write(fileMetadataBytes, 0, fileMetadataPacket.getBytesSize());
            } catch (Exception e) {
                this.suddenlyClose("sendRawMessage: Header write failed");
                return -1;
            }

            // Send file data
            int fileDataLength = (int) file.length();
            int sentBytes = 0;
            BufferedInputStream bufferedInputStream = null;
            try {
                bufferedInputStream = new BufferedInputStream(new FileInputStream(file));
            } catch (IOException e) {
                this.suddenlyClose("sendRawMessage: failed to get stream for reading file to be "
                        + "attached");
                return -1;
            }
            while (sentBytes < fileDataLength) {
                // Make file data packet
                CommRawPacket fileDataPacket;
                int fileDataPayloadSize;
                boolean isEnd;
                if (CommRawPacketHeader.kMaxPacketPayloadSize < fileDataLength - sentBytes) {
                    // Not end data
                    fileDataPayloadSize = CommRawPacketHeader.kMaxPacketPayloadSize;
                    isEnd = false;
                } else {
                    // end data
                    fileDataPayloadSize = (short) (fileDataLength - sentBytes);
                    isEnd = true;
                }
                byte[] fileDataPayloadBytes = new byte[fileDataPayloadSize];
                try {
                    int readSize = bufferedInputStream.read(fileDataPayloadBytes, sentBytes,
                            fileDataPayloadSize);
                    if (readSize <= 0) break;
                } catch (IOException e) {
                    if (sentBytes < fileDataLength) {
                        this.suddenlyClose("sendRawMessage: I/O exception during reading file");
                        return -1;
                    } else break;
                }
                fileDataPacket = CommRawPacket.makeDataPacket(this.mPresentHeaderId,
                        fileDataPayloadBytes, sentBytes, CommRawPacketHeader
                                .kMaxPacketPayloadSize, isEnd, true);
                Log.d(TAG, "[" + this.mPortName + "] sendRawMessage(file): mPayloadSize = " +
                        fileDataPayloadSize);

                // Send
                byte[] fileDataPacketBytes = fileDataPacket.toByteArray();
                if (fileDataPacketBytes == null) {
                    this.suddenlyClose("sendRawMessage: Failed to make file data packet");
                }
                try {
                    dataOutputStream.write(fileDataPacketBytes, 0, fileDataPacket.getBytesSize());
                } catch (Exception e) {
                    this.suddenlyClose("sendRawMessage: Header write failed");
                    return -1;
                }
                sentBytes += fileDataPayloadSize;
            }
        }
        return 0;
    }
}

class CommRawPacket {
    private CommRawPacketHeader mRawPacketHeader;
    private CommRawPacketPayload mRawPacketPayload;

    private CommRawPacket(CommRawPacketHeader header, CommRawPacketPayload payload) {
        this.mRawPacketHeader = header;
        this.mRawPacketPayload = payload;
    }

    static public CommRawPacket makeMessageMetadataPacket(byte headerId, int totalDataSize,
                                                          boolean isFileAttached) {
        CommMessageMetadata payload = new CommMessageMetadata(totalDataSize, isFileAttached);
        CommRawPacketHeader header = new CommRawPacketHeader(headerId, payload.getBytesSize(), 0,
                false, false, false, true);

        return new CommRawPacket(header, payload);
    }

    static public CommRawPacket makeFileMetadataPacket(byte headerId, File srcFile) {
        CommFileMetadata payload = new CommFileMetadata((int) srcFile.length(), (char) srcFile
                .getName().length(), srcFile.getName().toCharArray());
        CommRawPacketHeader header = new CommRawPacketHeader(headerId, payload.getBytesSize(), 0,
                false, false, false, true);
        return new CommRawPacket(header, payload);
    }

    static public CommRawPacket makeDataPacket(byte headerId, byte[] data, int currOffset, short
            size, boolean isEnd, boolean isFile) {
        CommDataPayload payload = new CommDataPayload(data, size);
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

// Communication Framework Payload
class CommRawPacketHeader {
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

abstract class CommRawPacketPayload {
    abstract public byte[] toByteArray();

    abstract public short getBytesSize();
}

class CommMessageMetadata extends CommRawPacketPayload {
    // TODO: (on OPEL device side) add 'isFileAttached'
    public static final short kMessageHeaderSize = 8;
    private int mMessageDataLength;
    private int mIsFileAttached;

    public CommMessageMetadata(int messageDataLength, boolean isFileAttached) {
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

    public static CommMessageMetadata read(DataInputStream dataInputStream) throws IOException {
        int messageDataLength = dataInputStream.readInt();
        int isFileAttached = dataInputStream.readInt();
        CommMessageMetadata messageMetadata = new CommMessageMetadata(messageDataLength,
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

class CommFileMetadata extends CommRawPacketPayload {
    // TODO: (on OPEL device side) shrink fields
    private int mFileSize;
    private char mFileNameLength;
    private char mFileName[];

    public CommFileMetadata(int fileSize, char fileNameLength, char[] fileName) {
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

    public static CommFileMetadata read(DataInputStream dataInputStream) throws IOException {
        int fileSize = dataInputStream.readInt();
        char fileNameLength = dataInputStream.readChar();
        byte[] fileNameBytes = new byte[fileNameLength];
        dataInputStream.readFully(fileNameBytes, 0, fileNameLength);
        String fileNameStr = new String(fileNameBytes, "UTF-8");
        char[] fileName = fileNameStr.toCharArray();

        CommFileMetadata fileMetadata = new CommFileMetadata(fileSize, fileNameLength, fileName);
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

class CommDataPayload extends CommRawPacketPayload {
    private short mSize;
    private byte mData[];

    public CommDataPayload(byte[] data, short size) {
        this.mData = data;
        this.mSize = size;
    }

    public byte[] toByteArray() {
        return this.mData;
    }

    public static CommDataPayload read(DataInputStream dataInputStream, short dataSize) throws
            IOException {
        byte[] data = new byte[dataSize];
        dataInputStream.readFully(data, 0, dataSize);
        CommDataPayload dataPayload = new CommDataPayload(data, dataSize);
        return dataPayload;
    }

    public short getBytesSize() {
        return this.mSize;
    }
}