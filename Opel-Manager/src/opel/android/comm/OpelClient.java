package opel.android.comm;

import android.util.Log;

import java.util.Iterator;

public class OpelClient extends OpelSCModel {
	public static final int COMM_DISCON = 0;
	public static final int COMM_CONNECTING = 1;
	public static final int COMM_CONNECTED = 2;
	ConnectThread connThread;
	private int status;
	public OpelClient(String intf_name, OpelCallbacks defCb, OpelCallbacks statCb) {
		super(intf_name, defCb, statCb);
		// TODO Auto-generated constructor stub
		status = COMM_DISCON;
		connThread = new ConnectThread();
	}
	
	public void Start(){
		connThread.start();
	}
	public void Cancel(){
		super.Cancel();
		
	}
	
	public synchronized void connected(){
		if(status != COMM_CONNECTING)
			return;
		status = COMM_CONNECTED;
		super.Start();
		statCb.OpelCb(new OpelMessage(), (short)OpelSocket.STAT_CONNECTED);			
	}
	
	public int getStat(){
		return status;
	}
	
	synchronized public void SendMsg(String message){
		SendMsg(message, defCb);
	}
	synchronized public void SendMsg(String message, OpelCallbacks cb){
		byte[] buff = new byte[message.length() + 1];
		int i;
		for(i=0; i<message.length(); i++){
			buff[i] = (byte)message.charAt(i);
		}
		
		buff[i] = 0;
		
		SendMsg(buff, cb);
	}
	synchronized public void SendMsg(byte[] buff){
		SendMsg(buff, defCb);
	}
	synchronized public void SendMsg(byte[] buff, OpelCallbacks cb){
		OpelMessage op_msg = new OpelMessage();
		
		op_msg.set_msg();
		op_msg.set_data(buff, buff.length);
		Log.d(OpelUtil.TAG, "Sending Message1 " + new String(buff));
		
		Iterator<OpelSocket> it = generic_read_handler.socks.iterator();
		
		synchronized(generic_read_handler.socks){
			while(it.hasNext()){
				Log.d(OpelUtil.TAG, "Sending Message2 " + new String(buff));
				OpelSocket ops = it.next();
				
				op_msg.set_op_sock(ops);
				super.send(op_msg, cb);
			}
		}
	}
	synchronized public void SendFile(String srcPath, String destPath){
		SendFile(srcPath, destPath, defCb);
	}
	synchronized public void SendFile(String srcPath, String destPath, OpelCallbacks cb){
		OpelMessage op_msg = new OpelMessage();
		
		op_msg.set_file(srcPath, destPath, 0, -1);
		op_msg.complete_header();
		
		Iterator<OpelSocket> it = generic_read_handler.socks.iterator();
		
		synchronized(generic_read_handler.socks){
			Log.d(OpelUtil.TAG, "Sending File");
			while(it.hasNext()){
				OpelSocket ops = it.next();
				
				Log.d(OpelUtil.TAG, "Sending File1");

				op_msg.set_op_sock(ops);
				super.send(op_msg, cb);
			}
		}
	}
	
	synchronized public void MsgRespond(OpelMessage msg, byte[] buff){
		MsgRespond(msg, buff);
	}
	synchronized public void MsgRespond(OpelMessage msg, byte[] buff, OpelCallbacks cb){
		OpelMessage op_msg = new OpelMessage();
		op_msg.set_op_sock(msg.get_op_sock());
		op_msg.set_req_id(msg.get_req_id());
		op_msg.set_msg();
		op_msg.set_data(buff, buff.length);
		
		Iterator<OpelSocket> it = generic_read_handler.socks.iterator();
		
		synchronized(generic_read_handler.socks){
			while(it.hasNext()){
				OpelSocket ops = it.next();
				
				op_msg.set_op_sock(ops);
				super.send(op_msg, cb);
			}
		}	
	}
	
	synchronized public void FileRespond(OpelMessage msg, String srcPath, String destPath){
		FileRespond(msg, srcPath, destPath, defCb);
	}
	synchronized public void FileRespond(OpelMessage msg, String srcPath, String destPath, OpelCallbacks cb){
		OpelMessage op_msg = new OpelMessage();
		op_msg.set_op_sock(msg.get_op_sock());
		op_msg.set_req_id(msg.get_req_id());
		op_msg.set_file(srcPath, destPath, 0, -1);
		
		Iterator<OpelSocket> it = generic_read_handler.socks.iterator();
		
		synchronized(generic_read_handler.socks){
			while(it.hasNext()){
				OpelSocket ops = it.next();
				
				op_msg.set_op_sock(ops);
				super.send(op_msg, cb);
			}
		}
	}
	
	private class ConnectThread extends Thread {
		public void run(){
					
			if(status == COMM_CONNECTING || status == COMM_CONNECTED)
				return;
			
			status = COMM_CONNECTING;
			
			Log.d("hi", "go connect");
			OpelSocket op_sock = new OpelSocket(intf_name, OpelSocket.CONN_TYPE_BT, null);
			if(op_sock.connect() == OpelSCModel.COMM_ERR_NONE){
				synchronized(generic_read_handler){
					generic_read_handler.insertSocket(op_sock);
					generic_read_handler.notify();
				}
				connected();
			}
			else{
				status = COMM_DISCON;
				defCb.OpelCb(null, OpelSCModel.COMM_ERR_DISCON);
			}
			Log.d("hi", "done connect");
		}
	}

}
