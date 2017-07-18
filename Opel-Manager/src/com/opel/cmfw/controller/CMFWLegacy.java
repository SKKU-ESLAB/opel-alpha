/* Copyright (c) 2015-2016 CISS, and contributors. All rights reserved.
 *
 * Contributor: Eunsoo Park <esevan.park@gmail.com>
 *              Dongig Sin <dongig@skku.edu>
 *              Gyeonghwan Hong <redcarrottt@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.opel.cmfw.controller;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.net.wifi.p2p.WifiP2pManager;
import android.util.Log;

import com.opel.cmfw.controller.bluetooth.BluetoothSocketWrapper;
import com.opel.cmfw.controller.bluetooth.FallbackBluetoothSocket;
import com.opel.cmfw.controller.bluetooth.FallbackException;
import com.opel.cmfw.controller.bluetooth.NativeBluetoothSocket;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.UUID;

// TODO: shrink into 3 mPorts - default(BT), control(BT), largedata(WFD)
// TODO: move WFD initialization code to CommChannelService
// TODO: remove BT/WFD status management -> replace it with state change event listener
public class CMFWLegacy {
    // Status of Communication Framework
    public static finalint CMFW_STAT_DISCON = 0;
    public static final int CMFW_STAT_BT_CONNECTED = 1;
    public static final int CMFW_STAT_WFD_CONNECTED = 2;
    public static final int CMFW_WFD_OFF_TIME = 10;

    public static final int CMFW_CONTROL_PORT = 0;
    public static final int CMFW_DEFAULT_PORT = 1;

    public static final short CMFW_PACKET_SIZE = 1008;
    public static final short CMFW_PACKET_HEADER_SIZE = 8;
    public static byte mHeaderId[];
    private final int DEFINED_PORT_NUM = 2;

    // Comm Ports
    private CommLegacyPort mPorts[];
    // Bluetooth Port = UUID
    private final String mPortsUUID[] =
            {"0a1b2c3d-4e5f-6a1c-2d0e-1f2a3b4c5d6d",
                    "0a1b2c3d-4e5f-6a1c-2d0e-1f2a3b4c5d6e"};
    // Wi-fi Port = TCP Port Num
    private final int ports_port_num[] = {10001, 10002};

    // Wi-fi Direct
    public int wfd_in_use;

    private HashMap<Byte, CMFWMessageHeader> msg_header_store[];
    private HashMap<Byte, CMFWFileHeader> file_header_store[];
    private LinkedList<CMFWQueueNode> msg_recv_queue[];
    private LinkedList<CMFWQueueNode> file_recv_queue[];

    // TODO: move to CommChannelService
//    private WifiP2pManager mWifiP2pManager;
//    private WifiP2pManager.Channel mWifiP2pManagerChannel;
//    private WifiDirectBroadcastReceiver mWifiDirectBroadcastReceiver;

    // BluetoothDevice
    private BluetoothDevice mBluetoothDevice = null;
    private WfdOffThread wfd_off_thread;

    public CMFWLegacy() {
        // TODO: move to CommChannelService
        // Setting Wi-fi Direct members
        this.mWifiP2pManager = wifiP2pManager;
        this.mWifiP2pManagerChannel = wifiP2pManagerChannel;
        this.mWifiDirectBroadcastReceiver = wifiDirectBroadcastReceiver;
        this.mWifiDirectBroadcastReceiver.setOwnerController(this);

        mPorts = new CommLegacyPort[DEFINED_PORT_NUM];

        mHeaderId = new byte[DEFINED_PORT_NUM];
        msg_header_store = new HashMap[DEFINED_PORT_NUM];
        file_header_store = new HashMap[DEFINED_PORT_NUM];

        msg_recv_queue = new LinkedList[DEFINED_PORT_NUM];
        file_recv_queue = new LinkedList[DEFINED_PORT_NUM];

        for (int i = 0; i < DEFINED_PORT_NUM; i++) {
            mHeaderId[i] = 1;

            mPorts[i] = new CommLegacyPort(UUID.fromString(mPortsUUID[i]),
                    ports_port_num[i]);

            msg_header_store[i] = new HashMap<Byte, CMFWMessageHeader>();
            file_header_store[i] = new HashMap<Byte, CMFWFileHeader>();

            msg_recv_queue[i] = new LinkedList<CMFWQueueNode>();
            file_recv_queue[i] = new LinkedList<CMFWQueueNode>();
        }

        wfd_in_use = 0;

        wfd_off_thread = new WfdOffThread();
    }

    // TODO: move to CommChannelService
    public void setWifiDirectInfo(String wifiDirectName, String
            wifiDirectIPAddress) {
        this.mWifiDirectBroadcastReceiver.setWifiDirectInfo(wifiDirectName,
                wifiDirectIPAddress);
    }

    public String getWifiDirectName() {
        return this.mWifiDirectBroadcastReceiver.getWifiDirectName();
    }

    public String getWifiDirectAddress() {
        return this.mWifiDirectBroadcastReceiver.getWifiDirectAddress();
    }

    public WifiDirectBroadcastReceiver getWifiDirectBroadcastReceiver() {
        return this.mWifiDirectBroadcastReceiver;
    }

    public boolean connect(BluetoothDevice bluetoothDevice, int port) {
        if (port >= DEFINED_PORT_NUM) {
            return false;
        }

        if (mPorts[port].connect(bluetoothDevice)) {
            mBluetoothDevice = bluetoothDevice;
            return true;
        } else {
            return false;
        }
    }

    public void close(int port) {
        mPorts[port].close();
    }

    public int cmfw_send_file(int port, File file) {
        OutputStream os;
        InputStream is;
        if (by_wfd) {
            os = ports[port].get_output_stream(true);
            is = ports[port].get_input_stream(true);
        } else {
            os = ports[CMFW_RFS_PORT].get_output_stream(false);
            is = ports[CMFW_RFS_PORT].get_input_stream(false);
        }

        if (os == null) return -1;

        DataOutputStream dos = new DataOutputStream(os);
        int bytes = 0;
        BufferedInputStream bis = null;
        try {
            Log.d("File", "File send");
            bis = new BufferedInputStream(new FileInputStream(file));
        } catch (IOException e) {
            e.printStackTrace();
            res = -1;
        }
        if (res == -1) return -1;
        try {
            byte buff[] = new byte[CMFW_PACKET_SIZE];
            int read_size;
            Log.d("File", "Name Len:" + Integer.toString((int) ((byte) file
                    .getName().length())));
            dos.writeByte((byte) file.getName().length());
            dos.write(file.getName().getBytes(), 0, file.getName().length());
            Log.d("File", "Name:" + file.getName());
            dos.writeInt((int) file.length());
            Log.d("File", "File Size:" + Integer.toString((int) file.length()));
            while ((read_size = bis.read(buff, 0, CMFW_PACKET_SIZE)) != -1) {
                dos.write(buff, 0, read_size);
                bytes += read_size;
            }
        } catch (IOException e) {
            e.printStackTrace();
            if (bytes < file.length()) res = -1;
        }

        Log.d("WFD", "Sent file");
        byte[] ha = new byte[256];
        try {
            is.read(ha, 0, 1);
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (!by_wfd) {
            ports[CMFW_RFS_PORT].close();
        } else cmfw_wfd_off();

        return res;
    }

    private int sendRawString(int port, byte[] buf, int len) {
        int res = 0;

        OutputStream out_stream = mPorts[port].get_output_stream(false);

        if (out_stream == null) return -1;

        DataOutputStream dos = new DataOutputStream(out_stream);

        byte to_write[] = null;
        byte payload_header_data[] = null;

        // Header's header
        CMFWPayloadHeader payload_header = new CMFWPayloadHeader();
        payload_header.headerId = mHeaderId[port]++;
        //0x80 : file, 0x40 : data, 0x20 : end data
        payload_header.headerInfo = 0x00 | 0x00 | 0x00;
        payload_header.payloadSize = 4;
        payload_header.currOffset = 0;

        to_write = new byte[CMFW_PACKET_HEADER_SIZE + payload_header
                .payloadSize];
        payload_header_data = payload_header.toByteArray();

        if (res == -1) return res;

        System.arraycopy(payload_header_data, 0, to_write, 0,
                CMFW_PACKET_HEADER_SIZE);

        // Header
        CMFWMessageHeader msg_header = new CMFWMessageHeader();
        msg_header.totalDataSize = len;

        byte header_data[] = msg_header.toByteArray();
        System.arraycopy(header_data, 0, to_write, CMFW_PACKET_HEADER_SIZE,
                payload_header.payloadSize);


        try {
            // synchronized (mPorts[port]) {
            dos.write(to_write, 0, CMFW_PACKET_HEADER_SIZE + payload_header
                    .payloadSize);
            // }

        } catch (Exception e) {
            e.printStackTrace();
            Log.d("sendRawMessage", "Header write failed");
            close(port);
            res = -1;
        }
        if (res < 0) return res;

        // Data
        short max_payload_size = CMFW_PACKET_SIZE - CMFW_PACKET_HEADER_SIZE;

        int bytes = 0;
        while (bytes < len) {
            if (max_payload_size < len - bytes) {
                payload_header.payloadSize = max_payload_size;
                payload_header.headerInfo = 0x00 | 0x40 | 0x20;
            } else {
                payload_header.payloadSize = (short) (len - bytes);
                payload_header.headerInfo = 0x00 | 0x40 | 0x20;
            }
            Log.d("sendRawMessage", "payloadSize = " + Integer.toString
                    (payload_header.payloadSize));

            payload_header.currOffset = bytes;
            to_write = new byte[CMFW_PACKET_HEADER_SIZE + payload_header
                    .payloadSize];
            payload_header_data = payload_header.toByteArray();

            System.arraycopy(payload_header_data, 0, to_write, 0,
                    CMFW_PACKET_HEADER_SIZE);
            System.arraycopy(buf, bytes, to_write, CMFW_PACKET_HEADER_SIZE,
                    payload_header.payloadSize);
            try {
                // synchronized (mPorts[port]) {
                dos.write(to_write, 0, CMFW_PACKET_HEADER_SIZE +
                        payload_header.payloadSize);
                // }
            } catch (Exception e) {
                e.printStackTrace();
                Log.d("sendRawMessage", "Header write failed");
                res = -1;
                close(port);
            }
            if (res < 0) break;
            bytes += payload_header.payloadSize;
        }

        return res;
    }

    public int sendRawString(int port, String buf) {
        byte buf_bytes[] = new byte[buf.length() + 1];
        byte buf_bytes_str[] = buf.getBytes();
        System.arraycopy(buf_bytes_str, 0, buf_bytes, 0, buf_bytes_str.length);
        buf_bytes[buf_bytes_str.length] = 0;

        return sendRawString(port, buf_bytes, buf_bytes.length);
    }

    public int listenRawString(int port, byte[] buf, int len) {
        Log.d("RECV_MSG", "Receiving");
        int res = 0;
        while (true) {
            //Check msg queue nodes first
            byte header_id;
            byte header_flag;
            byte payload_size;
            byte offset;

            if (msg_recv_queue[port].isEmpty()) break;

            CMFWQueueNode msg_node;
            synchronized (msg_recv_queue[port]) {
                msg_node = msg_recv_queue[port].pollFirst();
            }
            if (msg_node == null) return -1;

            header_id = msg_node.headerId;
            if ((msg_node.headerFlag & 0x40) == 0x00) {
                //If this is header
                CMFWMessageHeader msg_header = msg_header_store[port].get
                        (header_id);
                if (msg_header != null) {
                    Log.d("RECV_MSG", "Header found: error");
                    msg_header_store[port].remove(header_id);
                }
                msg_header = new CMFWMessageHeader();
                ByteBuffer wrapper = ByteBuffer.wrap(msg_node.buffer);
                msg_header.totalDataSize = wrapper.getInt();
                Log.d("recvMsg", "Total data size: " + Integer.toString
                        (msg_header.totalDataSize));
                msg_header.data = new byte[msg_header.totalDataSize];

                msg_header_store[port].put(header_id, msg_header);
            } else {
                //If this is data
                CMFWMessageHeader msg_header = msg_header_store[port].get
                        (header_id);
                if (msg_header == null) {
                    Log.d("RECV_MSG", "Header not found:error");
                    return -1;
                }

                System.arraycopy(msg_node.buffer, 0, msg_header.data, msg_node
                        .offset, msg_node.length);
                if (msg_node.offset + msg_node.length == msg_header
                        .totalDataSize) {
                    msg_header_store[port].remove(header_id);
                    if (len < msg_header.totalDataSize) {
                        return -2;
                    }
                    System.arraycopy(msg_header.data, 0, buf, 0, msg_header
                            .totalDataSize);
                    return msg_header.totalDataSize;
                }
            }
        }

        InputStream in_stream = mPorts[port].get_input_stream(false);
        if (in_stream == null) return -1;

        DataInputStream dis = new DataInputStream(in_stream);

        while (true) {
            //Read socket until msg recv completes
            byte header_id = 0;
            byte header_flag = 0;
            short payload_size = 0;
            int curr_offset = 0;
            byte payload_data[] = null;

            synchronized (mPorts[port].mBtSocket) {
                try {
                    header_id = dis.readByte();
                    header_flag = dis.readByte();
                    payload_size = dis.readShort();
                    curr_offset = dis.readInt();
                    if (payload_size > 0) {
                        payload_data = new byte[payload_size];
                        dis.readFully(payload_data, 0, payload_size);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    res = -1;
                }
            }

            if (res < 0) return res;

            Log.d("PAY_LOAD", Integer.toHexString((0xFF & (int) header_id)) +
                    " " + Integer.toHexString((0xFF & (int) header_flag)) + "" +
                    " " + Integer.toHexString((int) payload_size) + " " +
                    Integer.toHexString(curr_offset));
            if ((header_flag & 0x80) == 0x80) {
                //If this is file!
                CMFWQueueNode queue_node = new CMFWQueueNode();
                queue_node.headerId = header_id;
                queue_node.headerFlag = header_flag;
                queue_node.buffer = payload_data;
                queue_node.length = payload_size;
                queue_node.offset = curr_offset;

                synchronized (file_recv_queue[port]) {
                    file_recv_queue[port].add(queue_node);
                }
            } else {
                //else this is msg!
                if ((header_flag & 0x40) == 0x00) {
                    //If this is header
                    CMFWMessageHeader msg_header = msg_header_store[port].get
                            (header_id);
                    if (msg_header != null) {
                        Log.d("RECV_MSG", "Header found: error");
                        msg_header_store[port].remove(header_id);
                    }
                    msg_header = new CMFWMessageHeader();
                    ByteBuffer wrapper = ByteBuffer.wrap(payload_data);
                    msg_header.totalDataSize = wrapper.getInt();
                    Log.d("recvMsg", "Total data size: " + Integer.toString
                            (msg_header.totalDataSize));
                    msg_header.data = new byte[msg_header.totalDataSize];

                    msg_header_store[port].put(header_id, msg_header);
                } else {
                    //If this is data
                    CMFWMessageHeader msg_header = msg_header_store[port].get
                            (header_id);
                    if (msg_header == null) {
                        Log.d("RECV_MSG", "Header not found:error");
                        return -1;
                    }

                    System.arraycopy(payload_data, 0, msg_header.data,
                            curr_offset, payload_size);
                    if (curr_offset + payload_size == msg_header
                            .totalDataSize) {
                        msg_header_store[port].remove(header_id);
                        if (len < msg_header.totalDataSize) {
                            return -2;
                        }
                        System.arraycopy(msg_header.data, 0, buf, 0,
                                msg_header.totalDataSize);
                        return msg_header.totalDataSize;
                    }
                }
            }
        }
    }

    // TODO: move to CommChannelService
    public int turnOnWifiDirect(boolean retry) {
        if (!retry) {
            if (mPorts[CMFW_CONTROL_PORT].connect(this.mBluetoothDevice)) {
                byte[] buf = new byte[4096];
                String msg;
                int res = listenRawString(CMFW_CONTROL_PORT, buf, 4096);

                msg = new String(Arrays.copyOfRange(buf, 0, res));
                Log.d("OPEL", "Control Message:" + msg);
                if (msg.compareTo("off") == 0) {
                    mPorts[CMFW_CONTROL_PORT].close();
                    return -1;
                } else {
                    if (this.mWifiDirectBroadcastReceiver.isConnected()) {
                        wfd_in_use++;
                        sendRawString(CMFW_CONTROL_PORT, "on");
                        mPorts[CMFW_CONTROL_PORT].close();
                        return 0;
                    } else {
                        //Not connected --> new Thread to connectChannel wfd
                        sendRawString(CMFW_CONTROL_PORT, "off");
                        mPorts[CMFW_CONTROL_PORT].close();
                    }
                }
            } else {
                Log.d("OPEL", "Control port connection failed");
                return -1;
            }
        }

        Log.d("WFD_ON", "Scan!");

        synchronized (this.mWifiP2pManager) {
            do {
                this.mWifiP2pManager.discoverPeers(this
                        .mWifiP2pManagerChannel, new WifiP2pManager
                        .ActionListener() {
                    @Override
                    public void onSuccess() {
                        Log.d("OPEL", "Scan started");
                    }

                    @Override
                    public void onFailure(int reason) {

                    }
                });
            } while (false);
        }

        //WFD connecting action just started -> now use bt than wfd
        return -1;
    }

    // TODO: move to CommChannelService
    public int turnOffWifiDirect() {
        Log.d("WFD_OFF", "wfd_in_use = " + Integer.toString(wfd_in_use));
        if (--wfd_in_use > 0) {
            Log.d("WifiP2p", "WFD in use");
            return 0;
        } else if (wfd_in_use < 0) wfd_in_use = 0;

        if (!wfd_off_thread.isRunning()) {
            wfd_off_thread = new WfdOffThread();
            wfd_off_thread.start();
        }

        return 0;
    }

    // Communication Framework Payload
    private class CMFWPayloadHeader {
        public byte headerId;
        public byte headerInfo;
        public short payloadSize;
        public int currOffset;

        CMFWPayloadHeader() {
            headerId = 0;
            headerInfo = 0;
            payloadSize = 0;
            currOffset = 0;
        }

        byte[] toByteArray() {
            ByteBuffer bb = ByteBuffer.allocate(8);
            bb.put(headerId);
            bb.put(headerInfo);
            bb.putShort(payloadSize);
            bb.putInt(currOffset);
            return bb.array();
        }
    }

    // Message Header
    private class CMFWMessageHeader {
        public int totalDataSize;
        public byte data[];

        CMFWMessageHeader() {
            totalDataSize = 0;
            data = null;
        }

        byte[] toByteArray() {
            if (totalDataSize == 0) return null;
            return ByteBuffer.allocate(4).putInt(totalDataSize).array();
        }
    }

    // File Header
    private class CMFWFileHeader {
        public int fileSize;
        public char srcFileNameLength;
        public char srcFileName[];
        public char destFileNameLength;
        public char destFileName[];

        CMFWFileHeader() {
            fileSize = 0;
            srcFileNameLength = 0;
            srcFileName = null;
            destFileNameLength = 0;
            destFileName = null;
        }
    }

    // Queue Node
    private class CMFWQueueNode {
        public byte headerId;
        public byte headerFlag;
        public byte buffer[];
        public short length;
        public int offset;

        CMFWQueueNode() {
            headerId = 0;
            headerFlag = 0;
            buffer = null;
            length = 0;
            offset = 0;
        }
    }

    // Wi-fi Direct Off Thread
    private class WfdOffThread extends Thread {
        private boolean running;

        WfdOffThread() {
            running = false;
        }

        public boolean isRunning() {
            return running;
        }

        public void run() {
            running = true;
            int iter = CMFW_WFD_OFF_TIME * 10;
            int prev_iter = 0;
            while (wfd_in_use <= 0 && iter-- > 0) {
                try {
                    sleep(100);
                    if (prev_iter != iter / 10) {
                        prev_iter = iter / 10;
                        Log.d("WFD_OFF", "WFD off after " + Integer.toString
                                (iter / 10) + "seconds");
                    }
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            if (wfd_in_use <= 0) {
                mPorts[CMFW_DEFAULT_PORT].wfd_close();
                mWifiP2pManager.stopPeerDiscovery(mWifiP2pManagerChannel, new
                        WifiP2pManager.ActionListener() {
                            @Override
                            public void onSuccess() {
                                Log.d("WifiP2p", "Stop discovery");
                            }

                            @Override
                            public void onFailure(int reason) {
                            }
                        });

                mWifiDirectBroadcastReceiver.removing = true;
                mWifiP2pManager.removeGroup(mWifiP2pManagerChannel, new
                        WifiP2pManager.ActionListener() {
                            @Override
                            public void onSuccess() {
                                Log.d("WifiP2p", "Removed from group");
                            }

                            @Override
                            public void onFailure(int reason) {
                                Log.d("WifiP2p", "Remove failed");

                            }
                        });
            }
            running = false;
        }
    }

    // Port
    class CommLegacyPort {
        public final String ip_addr = "192.168.49.1";

        private BluetoothSocket mBtSocket;
        private Socket socket;
        private UUID uuid;
        private int tcp_port_num;

        CommLegacyPort(UUID uuid, int port_num) {
            this.uuid = uuid;
            mBtSocket = null;
            socket = null;
            tcp_port_num = port_num;
        }

        public int get_stat() {
            if (mBtSocket == null) return CMFW_STAT_DISCON;

            //Log.d("Stat", Boolean.toString(mBtSocket.isChannelConnected()));
            if (!mBtSocket.isConnected()) return CMFW_STAT_DISCON;

            if (socket == null) return CMFW_STAT_BT_CONNECTED;
            else if (!socket.isConnected())
                return CMFW_STAT_BT_CONNECTED;
            else return CMFW_STAT_WFD_CONNECTED;
        }

        public boolean connect(BluetoothDevice bluetoothDevice) {
            boolean isSucceed = false;
            if (get_stat() != CMFW_STAT_DISCON) {
                isSucceed = true;
                return isSucceed;
            }
            BluetoothSocketWrapper newBtSocket = null;
            // RedCarrottt: Fix Bluetooth connection failure bug
            // (Issue #103)
            try {
                BluetoothSocket rawBtSocket = bluetoothDevice
                        .createRfcommSocketToServiceRecord(uuid);
                newBtSocket = new NativeBluetoothSocket
                        (rawBtSocket);
                newBtSocket.connect();
                isSucceed = true;
            } catch (IOException e) {
                try {
                    newBtSocket = new FallbackBluetoothSocket
                            (newBtSocket.getUnderlyingSocket());
                    Thread.sleep(500);
                    newBtSocket.connect();
                    isSucceed = true;
                } catch (FallbackException e1) {
                    Log.w("Bluetooth", "Could not connectChannel " +
                            "FallbackBluetoothSocket class", e1);
                    isSucceed = false;
                } catch (InterruptedException e1) {
                    Log.w("Bluetooth", e1.getMessage(), e1);
                    isSucceed = false;
                } catch (IOException e1) {
                    Log.w("Bluetooth", "Fallback failed. " +
                            "Cancelling it.", e1);
                    isSucceed = false;
                }
            }

            if (isSucceed == true && newBtSocket
                    .getUnderlyingSocket().isConnected()) {
                Log.d("Bluetooth", "Connected");
            } else {
                Log.d("Bluetooth", "Connection failed");
                newBtSocket = null;
            }

            if (newBtSocket == null) isSucceed = false;
            else this.mBtSocket = newBtSocket.getUnderlyingSocket();

            return isSucceed;
        }

        public void wfd_close() {
            int stat = get_stat();
            if (stat == CMFW_STAT_WFD_CONNECTED) {
                try {
                    socket.close();
                    Log.d("WFD", "socket closed");
                } catch (IOException e) {
                    e.printStackTrace();
                }
                socket = null;
            }

            return;

        }

        public boolean wfd_connect() {
            // TODO
            if (mWifiDirectBroadcastReceiver.isConnected() == false)
                return false;

            Socket tmp_sock = new Socket();

            try {
                tmp_sock.bind(null);
                Log.d("WFDConnect", "Try Socket connectChannel");
                tmp_sock.connect((new InetSocketAddress(ip_addr,
                        tcp_port_num)), 1500);
            } catch (IOException e) {
                e.printStackTrace();
                tmp_sock = null;
            }
            if (tmp_sock == null) return false;
            if (tmp_sock.isConnected() == true) {
                socket = tmp_sock;
                Log.d("WFD_CONNECT", "Connected to device");
                return true;
            } else return false;
        }

        // Remove it
        public OutputStream get_output_stream(boolean is_wfd) {
            OutputStream res = null;
            int stat = get_stat();
            //Log.d("STAT", Integer.toString(stat));
            if (is_wfd == false) if (stat == CMFW_STAT_WFD_CONNECTED)
                stat = CMFW_STAT_BT_CONNECTED;
            switch (stat) {
                case CMFW_STAT_DISCON:
                    res = null;
                    break;
                case CMFW_STAT_BT_CONNECTED: {
                    OutputStream tmp_output_stream = null;

                    try {
                        tmp_output_stream = mBtSocket.getOutputStream();
                    } catch (IOException e) {
                        e.printStackTrace();
                        tmp_output_stream = null;
                    }

                    res = tmp_output_stream;

                    break;
                }
                case CMFW_STAT_WFD_CONNECTED: {
                    OutputStream tmp_output_stream = null;

                    try {
                        tmp_output_stream = socket.getOutputStream();
                    } catch (IOException e) {
                        e.printStackTrace();
                        tmp_output_stream = null;
                    }

                    res = tmp_output_stream;
                    break;
                }
            }

            return res;
        }

        // Remove it
        public InputStream get_input_stream(boolean is_wfd) {
            InputStream res = null;
            int stat = get_stat();
            if (is_wfd == false && stat == CMFW_STAT_WFD_CONNECTED)
                stat = CMFW_STAT_BT_CONNECTED;
            switch (stat) {
                case CMFW_STAT_DISCON:
                    res = null;
                    break;
                case CMFW_STAT_BT_CONNECTED: {
                    InputStream tmp_input_stream = null;

                    try {
                        tmp_input_stream = mBtSocket.getInputStream();
                    } catch (IOException e) {
                        e.printStackTrace();
                        tmp_input_stream = null;
                    }

                    res = tmp_input_stream;

                    break;
                }
                case CMFW_STAT_WFD_CONNECTED: {
                    InputStream tmp_input_stream = null;

                    try {
                        tmp_input_stream = socket.getInputStream();
                    } catch (IOException e) {
                        e.printStackTrace();
                        tmp_input_stream = null;
                    }

                    res = tmp_input_stream;
                    break;
                }
            }

            return res;
        }

        // Remove it
        void close() {
            int stat = get_stat();
            switch (stat) {
                case CMFW_STAT_BT_CONNECTED:
                    try {
                        mBtSocket.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                        Log.d("BTSocket", "Close failed");
                    }
                    mBtSocket = null;
                    break;
                case CMFW_STAT_WFD_CONNECTED:
                    try {
                        socket.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                        Log.d("Socket", "Close failed");
                    }
                    socket = null;
                    break;
            }
        }
    }
}