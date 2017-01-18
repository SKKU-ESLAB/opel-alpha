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

package selectiveconnection;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.Channel;
import android.util.Log;

import com.example.opel_manager.globalData;

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
import java.lang.reflect.Method;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.UUID;

public class OpelCommunicator {
    // Status of Communication Framework
    public static final int CMFW_STAT_DISCON = 0;
    public static final int CMFW_STAT_BT_CONNECTED = 1;
    public static final int CMFW_STAT_WFD_CONNECTED = 2;

    // Constants on Target Device Name
    private static final String CMFW_BT_NAME_RPI2 = "pi";
    private static final String CMFW_WFD_NAME_RPI2 = "OPEL";
    private static final String CMFW_BT_NAME_TX1 = "tegra";
    private static final String CMFW_WFD_NAME_TX1 = "OPEL-Tegra";
    private static String sTargetBtName = CMFW_BT_NAME_TX1;
    private static String sTargetWfdName = CMFW_WFD_NAME_TX1;

    public static String getTargetBtName() {
        Log.d("OPEL", "bt: " + sTargetBtName);
        return sTargetBtName;
    }

    public static String getTargetWfdName() {
        Log.d("OPEL", "wfd: " + sTargetWfdName);
        return sTargetWfdName;
    }

    public static void setTargetRPi2() {
        sTargetBtName = CMFW_BT_NAME_RPI2;
        sTargetWfdName = CMFW_WFD_NAME_RPI2;
        Log.d("OPEL", "bt: " + sTargetBtName);
        Log.d("OPEL", "wfd: " + sTargetWfdName);
    }

    public static void setTargetTX1() {
        sTargetBtName = CMFW_BT_NAME_TX1;
        sTargetWfdName = CMFW_WFD_NAME_TX1;
        Log.d("OPEL", "bt: " + sTargetBtName);
        Log.d("OPEL", "wfd: " + sTargetWfdName);
    }

    // Communication Framework Payload
    private class cmfw_payload_header_c {
        public byte header_id;
        public byte header_info;
        public short payload_size;
        public int curr_offset;

        cmfw_payload_header_c() {
            header_id = 0;
            header_info = 0;
            payload_size = 0;
            curr_offset = 0;
        }

        byte[] to_byte() {
            ByteBuffer bb = ByteBuffer.allocate(8);
            bb.put(header_id);
            bb.put(header_info);
            bb.putShort(payload_size);
            bb.putInt(curr_offset);
            return bb.array();
        }
    }

    // Message Header
    private class cmfw_msg_header_c {
        public int total_data_size;
        public byte data[];

        cmfw_msg_header_c() {
            total_data_size = 0;
            data = null;
        }

        byte[] to_byte() {
            if (total_data_size == 0) return null;
            return ByteBuffer.allocate(4).putInt(total_data_size).array();
        }
    }

    // File Header
    private class cmfw_file_header_c {
        public int file_size;
        public char src_file_name_len;
        public char src_file_name[];
        public char dest_file_name_len;
        public char dest_file_name[];

        cmfw_file_header_c() {
            file_size = 0;
            src_file_name_len = 0;
            src_file_name = null;
            dest_file_name_len = 0;
            dest_file_name = null;
        }
    }

    // Queue Node
    private class cmfw_queue_node_c {
        public byte header_id;
        public byte header_flag;
        public byte buf[];
        public short len;
        public int offset;

        cmfw_queue_node_c() {
            header_id = 0;
            header_flag = 0;
            buf = null;
            len = 0;
            offset = 0;
        }
    }

    // RedCarrottt: Fix Bluetooth connection failure bug (Issue #103)
    public static class NativeBluetoothSocket implements
            BluetoothSocketWrapper {
        private BluetoothSocket socket;

        public NativeBluetoothSocket(BluetoothSocket tmp) {
            this.socket = tmp;
        }

        @Override
        public InputStream getInputStream() throws IOException {
            return socket.getInputStream();
        }

        @Override
        public OutputStream getOutputStream() throws IOException {
            return socket.getOutputStream();
        }

        @Override
        public String getRemoteDeviceName() {
            return socket.getRemoteDevice().getName();
        }

        @Override
        public void connect() throws IOException {
            socket.connect();
        }

        @Override
        public String getRemoteDeviceAddress() {
            return socket.getRemoteDevice().getAddress();
        }

        @Override
        public void close() throws IOException {
            socket.close();
        }

        @Override
        public BluetoothSocket getUnderlyingSocket() {
            return socket;
        }
    }

    public class FallbackBluetoothSocket extends NativeBluetoothSocket {
        private BluetoothSocket fallbackSocket;

        public FallbackBluetoothSocket(BluetoothSocket tmp) throws
                FallbackException {
            super(tmp);
            try {
                Class<?> clazz = tmp.getRemoteDevice().getClass();
                Class<?>[] paramTypes = new Class<?>[]{Integer.TYPE};
                Method m = clazz.getMethod("createRfcommSocket", paramTypes);
                Object[] params = new Object[]{Integer.valueOf(1)};
                fallbackSocket = (BluetoothSocket) m.invoke(tmp
                        .getRemoteDevice(), params);
            } catch (Exception e) {
                throw new FallbackException(e);
            }
        }

        @Override
        public InputStream getInputStream() throws IOException {
            return fallbackSocket.getInputStream();
        }

        @Override
        public OutputStream getOutputStream() throws IOException {
            return fallbackSocket.getOutputStream();
        }

        @Override
        public void connect() throws IOException {
            fallbackSocket.connect();
        }

        @Override
        public void close() throws IOException {
            fallbackSocket.close();
        }
    }

    public static class FallbackException extends Exception {
        private static final long serialVersionUID = 1L;

        public FallbackException(Exception e) {
            super(e);
        }
    }

    public static interface BluetoothSocketWrapper {
        InputStream getInputStream() throws IOException;

        OutputStream getOutputStream() throws IOException;

        String getRemoteDeviceName();

        String getRemoteDeviceAddress();

        BluetoothSocket getUnderlyingSocket();

        void connect() throws IOException;

        void close() throws IOException;
    }

    // Port
    private class cmfw_port_c {
        public final String ip_addr = "192.168.49.1";

        private BluetoothSocket mBtSocket;
        private Socket socket;
        private UUID uuid;
        private int tcp_port_num;

        cmfw_port_c(UUID uuid, int port_num) {
            this.uuid = uuid;
            mBtSocket = null;
            socket = null;
            tcp_port_num = port_num;
        }

        public int get_stat() {
            if (mBtSocket == null) return CMFW_STAT_DISCON;

            //Log.d("Stat", Boolean.toString(mBtSocket.isConnected()));
            if (mBtSocket.isConnected() == false) return CMFW_STAT_DISCON;

            if (socket == null) return CMFW_STAT_BT_CONNECTED;
            else if (socket.isConnected() == false)
                return CMFW_STAT_BT_CONNECTED;
            else return CMFW_STAT_WFD_CONNECTED;
        }


        public boolean connect() {
            boolean isSucceed = false;
            if (get_stat() != CMFW_STAT_DISCON) {
                isSucceed = true;
                return isSucceed;
            }

            Set<BluetoothDevice> paired_devices = BluetoothAdapter
                    .getDefaultAdapter().getBondedDevices();
            BluetoothSocketWrapper newBtSocket = null;

            if (paired_devices.size() > 0) {
                for (BluetoothDevice btDevice : paired_devices) {
                    if (btDevice.getName().contains(OpelCommunicator
                            .getTargetBtName())) {
                        // RedCarrottt: Fix Bluetooth connection failure bug
                        // (Issue #103)
                        try {
                            BluetoothSocket rawBtSocket = btDevice
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
                                Log.w("Bluetooth", "Could not initialize " +
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
                    }
                }
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
            if (globalData.getInstance().getWifiReceiver().isConnected() ==
                    false)
                return false;

            Socket tmp_sock = new Socket();

            try {
                tmp_sock.bind(null);
                Log.d("WFDConnect", "Try Socket connect");
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

    private cmfw_port_c ports[];

    public static final int CMFW_WFD_OFF_TIME = 10;
    public static final int CMFW_CONTROL_PORT = 0;
    public static final int CMFW_DEFAULT_PORT = 1;
    public static final int CMFW_RFS_PORT = 2;
    public static final int CMFW_STR_PORT = 3;
    public static final int CMFW_MAR_PORT = 4;

    public static final short CMFW_PACKET_SIZE = 1008;
    public static final short CMFW_PACKET_HEADER_SIZE = 8;

    private final int CMFW_CMD_WFD_ON = 1;
    private final int CMFW_CMD_WFD_ON_ACK = 2;
    private final int CMFW_CMD_WFD_OFF = 3;
    private final int CMFW_CMD_WFD_OFF_ACK = 4;

    private final int DEFINED_PORT_NUM = 5;
    private final String ports_uuid[] =
            {"0a1b2c3d-4e5f-6a1c-2d0e-1f2a3b4c5d6d",
                    "0a1b2c3d-4e5f-6a1c-2d0e-1f2a3b4c5d6e",
                    "0a1b2c3d-4e5f-6a1c-2d0e-1f2a3b4c5d6f",
                    "0a1b2c3d-4e5f-6a1c-2d0e-1f2a3b4c5d6a",
                    "0a1b2c3d-4e5f-6a1c-2d0e-1f2a3b4c5d6b"};
    private final int ports_port_num[] = {10001, 10002, 10003, 10004, 10005};

    public static byte header_id[];

    private HashMap<Byte, cmfw_msg_header_c> msg_header_store[];
    private HashMap<Byte, cmfw_file_header_c> file_header_store[];

    private LinkedList<cmfw_queue_node_c> msg_recv_queue[];
    private LinkedList<cmfw_queue_node_c> file_recv_queue[];

    private WifiP2pManager mManager;
    private Channel mChannel;

    public int wfd_in_use;

    private List<WifiP2pDevice> peers;
    private String mMac;

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
                ports[CMFW_DEFAULT_PORT].wfd_close();
                mManager.stopPeerDiscovery(mChannel, new WifiP2pManager
                        .ActionListener() {
                    @Override
                    public void onSuccess() {
                        Log.d("WifiP2p", "Stop discovery");
                    }

                    @Override
                    public void onFailure(int reason) {
                    }
                });
                //mManager.cancelConnect(mChannel, );
                globalData.getInstance().getWifiReceiver().removing = true;
                mManager.removeGroup(mChannel, new WifiP2pManager
                        .ActionListener() {
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

    private WfdOffThread wfd_off_thread;

    public static String getMacAddr() {
        try {
            List<NetworkInterface> all = Collections.list(NetworkInterface
                    .getNetworkInterfaces());
            for (NetworkInterface nif : all) {
                if (!nif.getName().equalsIgnoreCase("wlan0")) continue;

                byte[] macBytes = nif.getHardwareAddress();
                if (macBytes == null) {
                    return "";
                }

                StringBuilder res1 = new StringBuilder();
                for (byte b : macBytes) {
                    res1.append(Integer.toHexString(b & 0xFF) + ":");
                }

                if (res1.length() > 0) {
                    res1.deleteCharAt(res1.length() - 1);
                }
                return res1.toString();
            }
        } catch (Exception ex) {
        }
        return "02:00:00:00:00:00";
    }

    public OpelCommunicator(WifiP2pManager wifiManager, Channel wifiChannel) {
        mManager = wifiManager;
        mChannel = wifiChannel;

        ports = new cmfw_port_c[DEFINED_PORT_NUM];

        header_id = new byte[DEFINED_PORT_NUM];
        msg_header_store = new HashMap[DEFINED_PORT_NUM];
        file_header_store = new HashMap[DEFINED_PORT_NUM];

        msg_recv_queue = new LinkedList[DEFINED_PORT_NUM];
        file_recv_queue = new LinkedList[DEFINED_PORT_NUM];

        for (int i = 0; i < DEFINED_PORT_NUM; i++) {
            header_id[i] = 1;

            ports[i] = new cmfw_port_c(UUID.fromString(ports_uuid[i]),
                    ports_port_num[i]);

            msg_header_store[i] = new HashMap<Byte, cmfw_msg_header_c>();
            file_header_store[i] = new HashMap<Byte, cmfw_file_header_c>();

            msg_recv_queue[i] = new LinkedList<cmfw_queue_node_c>();
            file_recv_queue[i] = new LinkedList<cmfw_queue_node_c>();
        }

        wfd_in_use = 0;

        wfd_off_thread = new WfdOffThread();
    }

    public void close(int port) {
        ports[port].close();
    }

    public boolean connect(int port) {
        if (port >= DEFINED_PORT_NUM) {
            return false;
        }
        return ports[port].connect();
    }

    public int cmfw_send_msg(int port, byte[] buf, int len) {
        int res = 0;


        OutputStream out_stream = ports[port].get_output_stream(false);

        if (out_stream == null) return -1;

        DataOutputStream dos = new DataOutputStream(out_stream);

        byte to_write[] = null;
        byte payload_header_data[] = null;

        // Header's header
        cmfw_payload_header_c payload_header = new cmfw_payload_header_c();
        payload_header.header_id = header_id[port]++;
        //0x80 : file, 0x40 : data, 0x20 : end data
        payload_header.header_info = 0x00 | 0x00 | 0x00;
        payload_header.payload_size = 4;
        payload_header.curr_offset = 0;

        to_write = new byte[CMFW_PACKET_HEADER_SIZE + payload_header
                .payload_size];
        payload_header_data = payload_header.to_byte();

        if (res == -1) return res;

        System.arraycopy(payload_header_data, 0, to_write, 0,
                CMFW_PACKET_HEADER_SIZE);

        // Header
        cmfw_msg_header_c msg_header = new cmfw_msg_header_c();
        msg_header.total_data_size = len;

        byte header_data[] = msg_header.to_byte();
        System.arraycopy(header_data, 0, to_write, CMFW_PACKET_HEADER_SIZE,
                payload_header.payload_size);


        try {
            // synchronized (ports[port]) {
            dos.write(to_write, 0, CMFW_PACKET_HEADER_SIZE + payload_header
                    .payload_size);
            // }

        } catch (Exception e) {
            e.printStackTrace();
            Log.d("cmfw_send_msg", "Header write failed");
            close(port);
            res = -1;
        }
        if (res < 0) return res;

        // Data
        short max_payload_size = CMFW_PACKET_SIZE - CMFW_PACKET_HEADER_SIZE;

        int bytes = 0;
        while (bytes < len) {
            if (max_payload_size < len - bytes) {
                payload_header.payload_size = max_payload_size;
                payload_header.header_info = 0x00 | 0x40 | 0x20;
            } else {
                payload_header.payload_size = (short) (len - bytes);
                payload_header.header_info = 0x00 | 0x40 | 0x20;
            }
            Log.d("cmfw_send_msg", "payload_size = " + Integer.toString
                    (payload_header.payload_size));

            payload_header.curr_offset = bytes;
            to_write = new byte[CMFW_PACKET_HEADER_SIZE + payload_header
                    .payload_size];
            payload_header_data = payload_header.to_byte();

            System.arraycopy(payload_header_data, 0, to_write, 0,
                    CMFW_PACKET_HEADER_SIZE);
            System.arraycopy(buf, bytes, to_write, CMFW_PACKET_HEADER_SIZE,
                    payload_header.payload_size);
            try {
                // synchronized (ports[port]) {
                dos.write(to_write, 0, CMFW_PACKET_HEADER_SIZE +
                        payload_header.payload_size);
                // }
            } catch (Exception e) {
                e.printStackTrace();
                Log.d("cmfw_send_msg", "Header write failed");
                res = -1;
                close(port);
            }
            if (res < 0) break;
            bytes += payload_header.payload_size;
        }

        return res;
    }


    public int cmfw_send_msg(int port, String buf) {
        byte buf_bytes[] = new byte[buf.length() + 1];
        byte buf_bytes_str[] = buf.getBytes();
        System.arraycopy(buf_bytes_str, 0, buf_bytes, 0, buf_bytes_str.length);
        buf_bytes[buf_bytes_str.length] = 0;

        return cmfw_send_msg(port, buf_bytes, buf_bytes.length);
    }

    public int cmfw_recv_msg(int port, byte[] buf, int len) {
        Log.d("RECV_MSG", "REceiving");
        int res = 0;
        while (true) {
            //Check msg queue nodes first
            byte header_id;
            byte header_flag;
            byte payload_size;
            byte offset;

            if (msg_recv_queue[port].isEmpty()) break;

            cmfw_queue_node_c msg_node;
            synchronized (msg_recv_queue[port]) {
                msg_node = msg_recv_queue[port].pollFirst();
            }
            if (msg_node == null) return -1;

            header_id = msg_node.header_id;
            if ((msg_node.header_flag & 0x40) == 0x00) {
                //If this is header
                cmfw_msg_header_c msg_header = msg_header_store[port].get
                        (header_id);
                if (msg_header != null) {
                    Log.d("RECV_MSG", "Header found: error");
                    msg_header_store[port].remove(header_id);
                }
                msg_header = new cmfw_msg_header_c();
                ByteBuffer wrapper = ByteBuffer.wrap(msg_node.buf);
                msg_header.total_data_size = wrapper.getInt();
                Log.d("recvMsg", "Total data size: " + Integer.toString
                        (msg_header.total_data_size));
                msg_header.data = new byte[msg_header.total_data_size];

                msg_header_store[port].put(header_id, msg_header);
            } else {
                //If this is data
                cmfw_msg_header_c msg_header = msg_header_store[port].get
                        (header_id);
                if (msg_header == null) {
                    Log.d("RECV_MSG", "Header not found:error");
                    return -1;
                }

                System.arraycopy(msg_node.buf, 0, msg_header.data, msg_node
                        .offset, msg_node.len);
                if (msg_node.offset + msg_node.len == msg_header
                        .total_data_size) {
                    msg_header_store[port].remove(header_id);
                    if (len < msg_header.total_data_size) {
                        return -2;
                    }
                    System.arraycopy(msg_header.data, 0, buf, 0, msg_header
                            .total_data_size);
                    return msg_header.total_data_size;
                }
            }
        }

        InputStream in_stream = ports[port].get_input_stream(false);
        if (in_stream == null) return -1;

        DataInputStream dis = new DataInputStream(in_stream);

        while (true) {
            //Read socket until msg recv completes
            byte header_id = 0;
            byte header_flag = 0;
            short payload_size = 0;
            int curr_offset = 0;
            byte payload_data[] = null;

            synchronized (ports[port].mBtSocket) {
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
                cmfw_queue_node_c queue_node = new cmfw_queue_node_c();
                queue_node.header_id = header_id;
                queue_node.header_flag = header_flag;
                queue_node.buf = payload_data;
                queue_node.len = payload_size;
                queue_node.offset = curr_offset;

                synchronized (file_recv_queue[port]) {
                    file_recv_queue[port].add(queue_node);
                }
            } else {
                //else this is msg!
                if ((header_flag & 0x40) == 0x00) {
                    //If this is header
                    cmfw_msg_header_c msg_header = msg_header_store[port].get
                            (header_id);
                    if (msg_header != null) {
                        Log.d("RECV_MSG", "Header found: error");
                        msg_header_store[port].remove(header_id);
                    }
                    msg_header = new cmfw_msg_header_c();
                    ByteBuffer wrapper = ByteBuffer.wrap(payload_data);
                    msg_header.total_data_size = wrapper.getInt();
                    Log.d("recvMsg", "Total data size: " + Integer.toString
                            (msg_header.total_data_size));
                    msg_header.data = new byte[msg_header.total_data_size];

                    msg_header_store[port].put(header_id, msg_header);
                } else {
                    //If this is data
                    cmfw_msg_header_c msg_header = msg_header_store[port].get
                            (header_id);
                    if (msg_header == null) {
                        Log.d("RECV_MSG", "Header not found:error");
                        return -1;
                    }

                    System.arraycopy(payload_data, 0, msg_header.data,
                            curr_offset, payload_size);
                    if (curr_offset + payload_size == msg_header
                            .total_data_size) {
                        msg_header_store[port].remove(header_id);
                        if (len < msg_header.total_data_size) {
                            return -2;
                        }
                        System.arraycopy(msg_header.data, 0, buf, 0,
                                msg_header.total_data_size);
                        return msg_header.total_data_size;
                    }
                }
            }
        }
    }

    public int cmfw_wfd_on(boolean retry) {
        if (!retry) {
            if (ports[CMFW_CONTROL_PORT].connect()) {
                byte[] buf = new byte[4096];
                String msg;
                int res = cmfw_recv_msg(CMFW_CONTROL_PORT, buf, 4096);

                msg = new String(Arrays.copyOfRange(buf, 0, res));
                Log.d("OPEL", "Control Message:" + msg);
                if (msg.equals("off")) {
                    ports[CMFW_CONTROL_PORT].close();
                    return -1;
                } else {
                    if (globalData.getInstance().getWifiReceiver()
                            .isConnected()) {
                        wfd_in_use++;
                        cmfw_send_msg(CMFW_CONTROL_PORT, "on");
                        ports[CMFW_CONTROL_PORT].close();
                        return 0;
                    } else {
                        //Not connected --> new Thread to connect wfd
                        cmfw_send_msg(CMFW_CONTROL_PORT, "off");
                        ports[CMFW_CONTROL_PORT].close();
                    }
                }
            } else {
                Log.d("OPEL", "Control port connection failed");
                return -1;
            }
        }


        Log.d("WFD_ON", "Scan!");

        synchronized (mManager) {
            do {
                mManager.discoverPeers(mChannel, new WifiP2pManager
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

    public int cmfw_wfd_off() {
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

    public int cmfw_send_file(int port, File fd) {

        int res = 0;
        int iter = 0;
        boolean by_wfd;

        if (cmfw_wfd_on(false) < 0) by_wfd = false;
        else by_wfd = true;

        if (by_wfd && false == ports[port].wfd_connect()) {
            by_wfd = false;
        }
        if (!by_wfd) {
            ports[CMFW_RFS_PORT].connect();
            Log.d("WifiDirect", "Connected");
        }

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
            bis = new BufferedInputStream(new FileInputStream(fd));
        } catch (IOException e) {
            e.printStackTrace();
            res = -1;
        }
        if (res == -1) return -1;
        try {
            byte buff[] = new byte[CMFW_PACKET_SIZE];
            int read_size;
            Log.d("File", "Name Len:" + Integer.toString((int) ((byte) fd
                    .getName().length())));
            dos.writeByte((byte) fd.getName().length());
            dos.write(fd.getName().getBytes(), 0, fd.getName().length());
            Log.d("File", "Name:" + fd.getName());
            dos.writeInt((int) fd.length());
            Log.d("File", "File Size:" + Integer.toString((int) fd.length()));
            while ((read_size = bis.read(buff, 0, CMFW_PACKET_SIZE)) != -1) {
                dos.write(buff, 0, read_size);
                bytes += read_size;
            }
        } catch (IOException e) {
            e.printStackTrace();
            if (bytes < fd.length()) res = -1;
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

    public int cmfw_recv_file(int port, File f) {

        int res = 0;
        int fnamelen = 0, flen = 0;
        byte fname[] = null, buf[] = null;
        String str_fname = null;

        int iter = 0;
        boolean by_wfd;

        if (cmfw_wfd_on(false) < 0) by_wfd = false;
        else by_wfd = true;


        if (by_wfd && false == ports[port].wfd_connect()) {
            by_wfd = false;
        }
        if (!by_wfd) {
            ports[CMFW_RFS_PORT].connect();
        }
        Log.d("WifiDirect", "Connected");
        InputStream is;
        OutputStream os;

        if (by_wfd) {
            is = ports[port].get_input_stream(true);
            os = ports[port].get_output_stream(true);
        } else {
            is = ports[CMFW_RFS_PORT].get_input_stream(false);
            os = ports[CMFW_RFS_PORT].get_output_stream(false);
        }
        if (is == null || os == null) return -1;

        DataInputStream dis = new DataInputStream(is);


        try {
            fnamelen = dis.readInt();
            fname = new byte[fnamelen];
            dis.readFully(fname, 0, fnamelen);
            str_fname = new String(fname);
            flen = dis.readInt();
        } catch (IOException e) {
            e.printStackTrace();
            res = -1;
        }

        Log.d("FileInfo", str_fname + "(" + Integer.toString(flen) + ")");

        if (res == -1) {
            ports[port].wfd_close();
            return res;
        }
        int bytes = 0;
        BufferedOutputStream bos = null;
        try {
            buf = new byte[CMFW_PACKET_SIZE];
            int read_size;
            bos = new BufferedOutputStream(new FileOutputStream(f));
            int curr_progress, prev_progress = 0;
            while (bytes < flen) {

                if ((curr_progress = (bytes * 10) / flen) != prev_progress) {
                    Log.d("Progress", Integer.toString(curr_progress * 10));
                    prev_progress = curr_progress;
                }
                read_size = dis.read(buf, 0, CMFW_PACKET_SIZE);
                bos.write(buf, 0, read_size);
                bytes += read_size;
            }
            Log.d("BYTE?SIZE", Integer.toString(bytes) + "/" + Integer
                    .toString(flen));
            //os.write(1023);
        } catch (IOException e) {
            e.printStackTrace();
            res = -1;
        }

        if (res == 0) {
            try {

                is.close();
                dis.close();
                bos.flush();
                bos.close();

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        byte[] ha = new byte[256];
        ha[0] = '1';
        try {
            os.write(ha, 0, 1);
        } catch (IOException e) {
            e.printStackTrace();
        }
        if (!by_wfd) ports[CMFW_RFS_PORT].close();
        else cmfw_wfd_off();

        return res;
    }
}