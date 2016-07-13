package opel.android.comm;

import java.util.Iterator;

public class OpelServer extends OpelSCModel {
	OpelServerSocket server;
	AcceptThread act;
	
	public OpelServer(String intf_name, byte conn_type, OpelCallbacks defCb, OpelCallbacks errCb) {
		super(intf_name, defCb, errCb);
		// TODO Auto-generated constructor stub
		server = new OpelServerSocket(intf_name, conn_type);
		act = new AcceptThread();
	}
	
	public void Start(){
		act.start();
		super.Start();
	}
	
	synchronized public void SendMsg(byte[] buff){
		SendMsg(buff, defCb);
	}
	synchronized public void SendMsg(byte[] buff, OpelCallbacks cb){
		OpelMessage op_msg = new OpelMessage();
		
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
	synchronized public void SendFile(String srcPath, String destPath){
		SendFile(srcPath, destPath, defCb);
	}
	synchronized public void SendFile(String srcPath, String destPath, OpelCallbacks cb){
		OpelMessage op_msg = new OpelMessage();
		
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
		
	private class AcceptThread extends Thread {
		public void run(){
			while(true){
				OpelSocket op_sock = server.accept();
				if(null != op_sock){
					generic_read_handler.insertSocket(op_sock);
					generic_read_handler.notify();
				}
			}
		}
	}

}
