package opel.android.comm;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.Set;
import java.util.UUID;

public class OpelSocket {
	static final byte CONN_TYPE_BT = 1;
	static final byte CONN_TYPE_WD = 2;
	static final byte CONN_TYPE_WF = 4;
	
	public static final short STAT_DISCON = 101;
	public static final short STAT_CONNECTING = 102;
	public static final short STAT_CONNECTED = 103;
	
	static final int BT_MAX_DAT_LEN = 880;
	
	private byte conn_type;
	
	private InputStream mmInStream;
	private OutputStream mmOutStream;
	
	private BluetoothSocket bt_client_sock;
	
	String intf_name;
	
	private short stat;
	
	public OpelSocket(String intf_name, byte conn_type, Object sock){
		this.conn_type = conn_type;
		this.intf_name = new String(intf_name);
		
		bt_client_sock = null;
		stat = STAT_DISCON;
		
		switch(conn_type){
			case CONN_TYPE_BT:
				if(null != sock){
					bt_client_sock = (BluetoothSocket) sock;
					if(bt_client_sock.isConnected())
						stat = STAT_CONNECTED;
					else
						stat = STAT_DISCON;
					InputStream tmpIn = null;
					OutputStream tmpOut = null;
					try{
						tmpIn = bt_client_sock.getInputStream();
						tmpOut = bt_client_sock.getOutputStream();
					} catch (IOException e){
						Log.e(OpelUtil.TAG, "Sockets not created", e);
					}
					
					mmInStream = tmpIn;
					mmOutStream = tmpOut;			
				}
				break;
			case CONN_TYPE_WD:
			case CONN_TYPE_WF:
			default:
				break;				
		}
	}
	
	synchronized public void write(byte[] buffer){
		try{
			mmOutStream.write(buffer);			
		} catch (IOException e){
			Log.e(OpelUtil.TAG, "Exception during write", e);
		}
	}
	synchronized public int read(byte[] buffer){
		int bytes = 0;
		try{			
			bytes = mmInStream.read(buffer);
		} catch (IOException e){
			Log.e(OpelUtil.TAG, "Exception during read", e);
		}
		return bytes;
	}
	public int max_data_len(){
		int res = 0;
		switch(conn_type){
			case CONN_TYPE_BT:
				res = BT_MAX_DAT_LEN;
				break;
			case CONN_TYPE_WD:
			case CONN_TYPE_WF:
			default:
				break;
		}
		return res;
	}
	
	public boolean isAvailable(){
		boolean res = false;
		if(null != mmInStream){
			try {
				int avail = mmInStream.available();
				Log.d(OpelUtil.TAG, "Available:"+avail);
				res = (avail > 0);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
		return res;
	}
	
	public boolean isConnected(){
		boolean res=false;
		
		try {
			res = bt_client_sock.isConnected();
		} catch(NullPointerException e){
			e.printStackTrace();
			Log.e(OpelUtil.TAG, "bt_client_sock is not initialized", e);
		}
		
		return res;
	}
	
	public short getStat(){
		return stat;
	}
	
	/*
	 * getPrivate()
	 * Desc: get useful information depending on the connection type. 
	 * e.g. Remote device name if bt connection 
	 */
	public String getPrivate(){
		String res = null;
		
		switch(conn_type){
		case CONN_TYPE_BT:
			res = new String(bt_client_sock.getRemoteDevice().getName());
			break;
		case CONN_TYPE_WD:
		case CONN_TYPE_WF:
		default:
			break;
		}
		
		return res;
	}
	
	synchronized public short connect(){
		short res = OpelSCModel.COMM_ERR_FAIL;
		
		switch(conn_type){
		case CONN_TYPE_BT:
			UUID uuid = OpelUtil.name2uuid(intf_name);
			Log.d(OpelUtil.TAG, intf_name+uuid.toString());
			BluetoothSocketWrapper tmpSock = null;
			Set<BluetoothDevice> pairedDevices = BluetoothAdapter.getDefaultAdapter().getBondedDevices();

			if(pairedDevices.size() > 0){
				for(BluetoothDevice tmpDevice : pairedDevices){

				}
				Log.d(OpelUtil.TAG, "Connected or Disconnected");
			}

			break;
		case CONN_TYPE_WD:
		case CONN_TYPE_WF:
		default:
			break;
		}
		
		return res;
	}
	
	public void Close()
	{
		try {
			bt_client_sock.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	// RedCarrottt: Fix Bluetooth connection failure bug (Issue #103, #112)
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
}
