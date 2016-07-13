package opel.android.comm;

import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;

public class TCPStreaming extends Thread {
	private Socket tcpSocket;
	private SocketAddress sock_addr;
	private InputStream mInStream;
	String ip;
	int port;
	private short stat;
	static public short STAT_DISCON = 0;
	static public short STAT_CONNECTING = 1;
	static public short STAT_CONNECTED = 2;
	
	public TCPStreaming(String ip, int port){
		this.ip = new String(ip);
		this.port = port;
		stat = STAT_DISCON;
		Socket tmpSock = null;
		sock_addr = null;
		try {
			tmpSock = new Socket(ip, port);
			sock_addr = tmpSock.getRemoteSocketAddress();
			if(tmpSock.isConnected()){
				stat = STAT_CONNECTED;
				mInStream = tmpSock.getInputStream();
				onStatChanged(stat);
			}
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();			
		}
		tcpSocket = tmpSock;
	}
	
	public void connect(String ip, int port){
		if(stat != STAT_DISCON){
			Log.d("TCPStreaming", "Already connecting or connected");
			return;
		}
		if(tcpSocket == null){
			Socket tmpSock = null;
			try {
				stat = STAT_CONNECTING;
				tmpSock = new Socket(ip, port);
				sock_addr = tmpSock.getRemoteSocketAddress();
				if(tmpSock.isConnected()){
					stat = STAT_CONNECTED;
					mInStream = tmpSock.getInputStream();
					onStatChanged(stat);
				}
			} catch (UnknownHostException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();			
			}
			tcpSocket = tmpSock;
		}
		else{
			stat = STAT_CONNECTING;
			try {				
				tcpSocket.connect(sock_addr);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			if(tcpSocket.isConnected()){
				stat = STAT_CONNECTED;
				try {
					mInStream = tcpSocket.getInputStream();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				onStatChanged(stat);
			}
		}
		
	}
	public void onStatChanged(short stat){
		/*
		 * Should implement this
		 */
	}
	public void onReceived(byte[] frame){
		/*
		 * Should implement this
		 */
	}
	
	public void run(){
		short prev_stat = stat;
		while(true){
			if(prev_stat != stat){
				prev_stat = stat;
				onStatChanged(stat);
			}
			while(stat == STAT_CONNECTED){
				if(tcpSocket.isConnected() == false){
					stat = STAT_DISCON;
					break;					
				}
				byte[] frame = null;
				byte[] buff = new byte[1460];
				
				try {
					mInStream.read(buff);
					
					byte[] lengthBuf = Arrays.copyOfRange(buff, 0, 4);
					byte[] offsetBuf = Arrays.copyOfRange(buff, 4, 8); 
					ByteOrder order = ByteOrder.BIG_ENDIAN;
					ByteBuffer buf = ByteBuffer.allocate(4);
					buf.order(order);
									
					buf.put(lengthBuf);
					buf.flip();
					
					int totalLen = buf.getInt();
					
					buf.put(offsetBuf);
					buf.flip();
					
					int offset = buf.getInt();
					
					if(frame == null){
						if(offset != 0)
							Log.d("TCPStreaming", offset+" ???!!");
						frame = new byte[totalLen];
					}
					int len = (offset+1452 <= totalLen)? 1452:totalLen-offset; 
					
					for(int i=0; i<len; i++){
						frame[offset+i] = buff[8+i];
					}
					
					if(totalLen <= offset+1452){
						onReceived(frame);
						frame = null;
					}
					
					
					
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
			}
			
			try {
				sleep(3000, 0);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
}
