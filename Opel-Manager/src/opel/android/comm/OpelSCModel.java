package opel.android.comm;

import android.os.Environment;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class OpelSCModel {
	static final int MAX_DAT_TIMEOUT = 10;
	
	public static final short COMM_ERR_NONE = 0;
	public static final short COMM_ERR_DISCON = -1;
	public static final short COMM_ERR_FAIL = -2;
	public static final short COMM_ERR_FOPEN = -3;
	public static final short COMM_ERR_FREAD = -4;
	public static final short COMM_ERR_FWRIT = -5;
	public static final short COMM_ERR_TOUT = -6;
	
	public static final short QDATA_TYPE_DEF = 0;
	public static final short QDATA_TYPE_ERR = -1;
	
	public static final int COMM_HEADER_SIZE = 128;
	
	public static final int REQ_TIME_INTERVAL = 1;
	
	@SuppressWarnings("unused")
	protected String intf_name = null;
	protected OpelCallbacks defCb = null;
	protected OpelCallbacks statCb = null;

	protected ReadThread generic_read_handler;
	protected MsgWriteThread generic_mwrite_handler;
	protected FileWriteThread generic_fwrite_handler;
	protected AckThread generic_ack_handler;	
	
	int curr_req_id;
		
	public OpelSCModel(String intf_name, OpelCallbacks defCb, OpelCallbacks statChangedCb){
		this.intf_name = intf_name;
		this.defCb = defCb;
		this.statCb = statChangedCb;
		
		generic_read_handler = new ReadThread();
		generic_mwrite_handler = new MsgWriteThread();
		generic_fwrite_handler = new FileWriteThread();
		generic_ack_handler = new AckThread();		
	}
	
	protected void Start(){
		generic_read_handler.start();
		generic_mwrite_handler.start();
		generic_fwrite_handler.start();
		generic_ack_handler.start();
	}
	protected void Cancel(){
		generic_read_handler.destroy();
		generic_mwrite_handler.destroy();
		generic_fwrite_handler.destroy();
		generic_ack_handler.destroy();
		Log.d("OPEL", "CANCLE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1");
	}
	
	public void send(OpelMessage msg, OpelCallbacks cb){
		if(msg.get_req_id() == 0){
			curr_req_id = curr_req_id+1;
			if(curr_req_id <= 0)
				curr_req_id = 1;
			msg.set_req_id(curr_req_id);
		}

		if(msg.is_msg()){
			Log.d(OpelUtil.TAG, "Sending message");
			generic_mwrite_handler.q.enqueue(msg, QDATA_TYPE_DEF, null, 0, cb);

			synchronized(generic_mwrite_handler){
				generic_mwrite_handler.notify();
			}
		}
		else if(msg.is_file()){
			Log.d(OpelUtil.TAG, "Sending file");

			generic_fwrite_handler.q.enqueue(msg, QDATA_TYPE_DEF, null, 0, cb);
			synchronized(generic_fwrite_handler){
				generic_fwrite_handler.notify();
			}
		}
	}
		
	protected class ReadThread extends Thread{
		public LinkedList<OpelSocket> socks;
		private boolean sch; 
		public ReadThread(){
			socks = new LinkedList<OpelSocket>();
			sch = false;
		}
		public void destroy(){
			sch = false;
			Iterator<OpelSocket> it = socks.iterator();
			while(it.hasNext()){
				OpelSocket op_sock = it.next();
				if(op_sock.isConnected()){
					op_sock.Close();
				}
			}
		}
		public void run(){
			Log.d(OpelUtil.TAG, android.os.Process.myTid()+": Read Thread Start");
			sch = true;
			while(sch){
				int num_notAvail = 0;
				int total_size = 0;
				
				if(socks.size() == 0){
					try {
						synchronized(this){
							Log.d(OpelUtil.TAG, android.os.Process.myTid() +": Read Thread Wait");
							wait();
							Log.d(OpelUtil.TAG, this.getId()+": Read Thread Notified");
						}
					} catch (InterruptedException e1) {
						// TODO Auto-generated catch block
						e1.printStackTrace();
					}
				}
				Iterator<OpelSocket> it = socks.iterator();
		
				while(it.hasNext()){
					OpelSocket op_sock = it.next();
					if(op_sock.isConnected()){	//For Asynchronous Operation
						if(op_sock.getStat() != OpelSocket.STAT_CONNECTED)
							statCb.OpelCb(new OpelMessage(), (short)OpelSocket.STAT_CONNECTED);
						Log.d(OpelUtil.TAG, "Socket Check:" +op_sock.getPrivate());
						if(op_sock.isAvailable()){ //For Asynchronous read and if it's available, then read handler handles it
							byte[] buff = new byte[COMM_HEADER_SIZE + op_sock.max_data_len()];
							op_sock.read(buff);
							
							readProcessing(op_sock, buff);
						}
						else
							num_notAvail++;
					}
					else{
						if(op_sock.getStat() == OpelSocket.STAT_CONNECTED)
							statCb.OpelCb(new OpelMessage(), (short)OpelSocket.STAT_CONNECTED);
						synchronized (socks){
							it.remove();
						}
					}
				}
					
				total_size = socks.size();
				if(num_notAvail == total_size){	//No socket has data --> sleep for a while to avoid busy waiting
					try {
						Log.d(OpelUtil.TAG, "No Request : Sleep for " + REQ_TIME_INTERVAL + "Seconds");
						Thread.sleep(1000 * REQ_TIME_INTERVAL,0);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
		}
		synchronized public void insertSocket(OpelSocket op_sock){
			synchronized (socks){
				if(null != op_sock)
					socks.add(op_sock);
			}
		}
		private void readProcessing(OpelSocket op_sock, byte[] buff){
			Log.d(OpelUtil.TAG, "Process new read");
			OpelMessage op_msg = new OpelMessage();
			
			op_msg.init_from_buff(buff);
			byte[] data = new byte[op_msg.get_data_len()];
			int i;
			for(i=0; i<op_msg.get_data_len(); i++)
				data[i] = buff[i+COMM_HEADER_SIZE];
			
			op_msg.set_data(data, op_msg.get_data_len());
			op_msg.set_op_sock(op_sock);
			
			if(op_msg.is_file() && op_msg.is_special() == false){
				
				File file = null;
				FileOutputStream fos = null;
				try {
					file = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS), op_msg.get_file_name());
					fos = new FileOutputStream(file);
					fos.write(data, op_msg.get_file_offset(), op_msg.get_data_len());
					
					fos.flush();
					fos.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
				if(file == null || fos == null)
					op_msg.set_err(COMM_ERR_FWRIT);
			}
			if(op_msg.is_ack()){
				synchronized(generic_ack_handler){
					generic_ack_handler.notify();
				}
				generic_ack_handler.rq.signal(op_msg);
				
			}
			else{
				defCb.OpelCb(op_msg, op_msg.get_err());
			}
		}
	}
	
	protected class MsgWriteThread extends Thread{
		private OpelCommQueue q;
		private boolean sch;
		public MsgWriteThread(){
			q = new OpelCommQueue();
			sch = false;
		}
		public void destroy(){
			sch = false;
		}
		public void run(){
			Log.d(OpelUtil.TAG, this.getId()+"MWrite Thread Start");
			sch = true;
			while(sch){
				if(q.isEmpty()){
					try {
						synchronized(this){
							Log.d(OpelUtil.TAG, this.getId()+": MWrite Thread Wait");
							this.wait();
							Log.d(OpelUtil.TAG, this.getId()+": MWrite Thread Notified");
						}
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
				else{
					OpelQueueData qd = q.dequeue();
					OpelMessage msg = qd.getMsg();
					OpelSocket op_sock = msg.get_op_sock();
					OpelCallbacks cb = qd.getCb();
					
					if(msg.get_data_len() > op_sock.max_data_len() ){
						Log.e(OpelUtil.TAG, "Not supported yet");
					}
					else{
						byte[] buff = new byte[COMM_HEADER_SIZE + msg.get_data_len()];
						msg.init_to_buff(buff);
						
						for(int i=0; i<msg.get_data_len(); i++){
							buff[i+COMM_HEADER_SIZE] = msg.get_data()[i];
						}
						
						op_sock.write(buff);
						
						if(cb != null){
							generic_ack_handler.rq.insert(msg, MAX_DAT_TIMEOUT, cb);
							synchronized(generic_ack_handler){
								generic_ack_handler.notify();
							}
						}
					}
				}
			}
		}
	}
	protected class FileWriteThread extends Thread{
		private OpelCommQueue q;
		private boolean sch;
		
		public FileWriteThread(){
			q = new OpelCommQueue();
			sch = false;
		}
		
		public void destroy(){
			sch = false;
		}
		public void run(){
			Log.d(OpelUtil.TAG, this.getId()+": FWrite Thread Start");
			sch = true;
			while(sch){
				if(q.isEmpty()){
					try {
						synchronized(this){
							Log.d(OpelUtil.TAG, this.getId()+": FWrite Thread Wait");
							this.wait();
							Log.d(OpelUtil.TAG, this.getId()+": FWrite Thread Notified");
						}
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
				else{
					OpelQueueData qd = q.dequeue();
					OpelMessage msg = qd.getMsg();
					OpelSocket op_sock = msg.get_op_sock();
					OpelCallbacks cb = qd.getCb();
					
					
					String fname = msg.get_file_name();
					//Android Specific
					
					try {
						File fd = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS), fname);
						BufferedInputStream tmpIn = new BufferedInputStream(new FileInputStream(fd));
						int fsize = (int)fd.length();
						int offset = 0;
						if(fsize < 0)
							Log.e(OpelUtil.TAG, "Not supported file transfer over 2GB size");
						else{
							Log.d(OpelUtil.TAG, "File:"+fname+"("+fsize+")");
							if(cb != null){
								generic_ack_handler.rq.insert(msg, MAX_DAT_TIMEOUT, cb);
								synchronized(generic_ack_handler){
									generic_ack_handler.notify();	
								}
							}
							
							while(offset < fsize){
								
								int len = (fsize - offset) > op_sock.max_data_len() ? op_sock.max_data_len():fsize-offset;							
								
								byte[] data = new byte[len];
								
								int bytes = tmpIn.read(data);
								if(bytes == -1)
									Log.d(OpelUtil.TAG, "File Read error");
								else{
									msg.set_file(fname, offset, fsize);
									msg.set_data(null, bytes);
									
									byte[] buff = new byte[COMM_HEADER_SIZE + bytes];
									Log.d(OpelUtil.TAG, "init?error?"+data.length+"/"+buff.length);
									msg.init_to_buff(buff);
									for(int i=0; i<bytes; i++){
										buff[COMM_HEADER_SIZE + i] = data[i];
									}
									
									op_sock.write(buff);
									offset += bytes;
								}
							}
							tmpIn.close();
							
							//Last Message gogo
							msg.set_file(fname, offset, fsize);
							msg.set_special();
							msg.set_data(null, 0);
							byte[] buff = new byte[COMM_HEADER_SIZE];
							msg.init_to_buff(buff);
							op_sock.write(buff);
						}
					} catch (Exception e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
						Log.e(OpelUtil.TAG, "Exception: File Read", e);
					}
				}
			}
		}
	}
	protected class AckThread extends Thread{
		public OpelCommQueue aq;
		public ReqQueue rq;
		public boolean sch;
		
		public AckThread(){
			aq = new OpelCommQueue();
			rq = new ReqQueue(aq);
			sch = false;;
		}
		public void destroy(){
			sch = false;
		}
		public void run(){
			sch = true;
			Log.d(OpelUtil.TAG, this.getId()+"Ack Thread Start");
			while(sch){
				if(rq.isEmpty() && aq.isEmpty()){
					try {
						synchronized(this){
							Log.d(OpelUtil.TAG, this.getId()+": Ack Thread Wait");
							wait();
							Log.d(OpelUtil.TAG, this.getId()+": Ack Thread Notified");
						}
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
				else{
					if(rq.isEmpty() == false){
						rq.doWait(REQ_TIME_INTERVAL);
					}
					while(aq.isEmpty() == false){
						OpelQueueData qd = aq.dequeue();
						OpelMessage msg = qd.getMsg();
						OpelCallbacks cb = qd.getCb();
						
						/* Someday it should be distinguished */
						/*
						if(qd.getType() == QDATA_TYPE_ERR){
						}
						else
						*/
						if(qd.getType() == QDATA_TYPE_ERR || msg.get_err() != COMM_ERR_NONE){
							Log.d(OpelUtil.TAG, "Invoke error callback");
							defCb.OpelCb(msg, msg.get_err());
						}
						else if(cb == null){
							Log.d(OpelUtil.TAG, "Invoke default callback");
							defCb.OpelCb(msg, msg.get_err());
						}
						else{
							Log.d(OpelUtil.TAG, "Invoke registered callback");
							cb.OpelCb(msg, msg.get_err());
						}
					}
				}
			}
		}
	}
	private class OpelQueueData {
		static final int QD_TYPE_DEF = 0;
		static final int QD_TYPE_ERR = 1;
		
		private OpelMessage op_msg;
		private byte[] buff;
		private int buff_len;
		private OpelCallbacks op_cb;
		private int type;
		
		public OpelQueueData(){
			op_msg = new OpelMessage();
			setBuff(null, 0);
			setCb(null);
			type = QD_TYPE_DEF;
		}
		public OpelQueueData(OpelQueueData qdt){
			this.op_msg = new OpelMessage(qdt.op_msg);
			setBuff(qdt.buff, qdt.buff_len);
			setCb(qdt.op_cb);
			this.type = qdt.type;
		}

		public byte[] getBuff() {
			return buff;
		}
		public int getBuffLen(){
			return buff_len;		
		}
		public void setBuff(byte[] buff, int len) {
			this.buff_len = len;
			if(buff != null){
				
				this.buff = buff.clone();
			}
		}

		public OpelCallbacks getCb() {
			return op_cb;
		}

		public void setCb(OpelCallbacks op_cb) {
			this.op_cb = op_cb;
		}

		public OpelMessage getMsg() {
			return op_msg;
		}

		public void setMsg(OpelMessage op_msg) {
			this.op_msg = new OpelMessage(op_msg);
		}
		
		public void setType(int type){
			this.type = type;
		}
		
		public int getType(){
			return type;
		}
	}
	
	private class OpelCommQueue {
		LinkedList<OpelQueueData> q;
		public OpelCommQueue(){
			q = new LinkedList<OpelQueueData>();
		}
		synchronized public void enqueue(OpelMessage msg, int type, byte[] buff, int buff_len, OpelCallbacks cb){
			synchronized (q){
				OpelQueueData qdt = new OpelQueueData();
				qdt.setMsg(msg);
				qdt.setType(type);
				if(buff != null){
					qdt.setBuff(buff, buff_len);
				}
				qdt.setCb(cb);
				q.add(qdt);
			}
		}
		synchronized public OpelQueueData dequeue(){
			OpelQueueData qdt;
			synchronized (q){
				qdt = q.poll();
			}
			return qdt;
		}
		synchronized public int length(){
			int res;
			synchronized (q){
			res = q.size();
			}			
			return res;
		}
		synchronized public boolean isEmpty(){
			boolean res;
			synchronized (q){
				res = (q.size() == 0);
			}
			return res;
		}
	}
	
	private class ReqQueue{
		LinkedList<ReqQueueData> reqList;
		private Lock lock;
		Condition awaken;
		OpelCommQueue ackQueue;
		
		private class ReqQueueData{
			public OpelMessage op_msg;
			public int timeout;
			public int init_tout;
			public OpelCallbacks cb;
			
			public ReqQueueData(){
				op_msg = null;
				cb = null;
				timeout = 0;
				init_tout = 0;
			}
		}
		
		public ReqQueue(OpelCommQueue ack_queue){
			reqList = new LinkedList<ReqQueueData>();
			lock = new ReentrantLock();
			awaken = lock.newCondition();
			ackQueue = ack_queue;
		}
		
		synchronized public void doWait(int timeout){
				try {
					lock.lock();
					if(awaken.await(timeout, TimeUnit.SECONDS) == false){
						Iterator<ReqQueueData> it = reqList.iterator();
						while(it.hasNext()){
							ReqQueueData rqd = it.next();
							rqd.timeout -= timeout;
							if(rqd.timeout <= 0){
								OpelMessage op_msg = rqd.op_msg;				
								op_msg.set_err((short)COMM_ERR_TOUT);
								ackQueue.enqueue(op_msg, QDATA_TYPE_ERR, null, 0, rqd.cb);
								it.remove();
							}
						}
					}
					lock.unlock();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}		
		}
		
		synchronized public void insert(OpelMessage op_msg, int timeout, OpelCallbacks cb){
			ReqQueueData rqd = new ReqQueueData();
			rqd.op_msg = op_msg;
			rqd.timeout = timeout;
			rqd.init_tout = timeout;
			rqd.cb = cb;
			synchronized (reqList){
				reqList.add(rqd);
			}
		}
		
		//Refresh the request timeout and enqueue
		synchronized public void signal(OpelMessage msg){
			synchronized(reqList){
				Iterator<ReqQueueData> it = reqList.iterator();
				while(it.hasNext()){
					ReqQueueData rqd = it.next();
					if(rqd.op_msg.get_req_id() == msg.get_req_id()){
						rqd.timeout = rqd.init_tout;
						ackQueue.enqueue(msg, QDATA_TYPE_DEF, null, 0, rqd.cb);
						break;
					}
				}
			}
		}
				
		//Signal and enqueue to ack queue with proper message
		synchronized public void signal(OpelMessage msg, byte[] buff, int buff_len){
			synchronized (reqList) {
				Iterator<ReqQueueData> it = reqList.iterator();
				while(it.hasNext()){
					ReqQueueData rqd = it.next();
					if(rqd.op_msg.get_req_id() == msg.get_req_id()){
						ackQueue.enqueue(msg, QDATA_TYPE_DEF, buff, buff_len, rqd.cb);
						it.remove();
						lock.lock();
						awaken.signal();
						lock.unlock();
						break;
					}
				}
			}
		}
		
		synchronized public boolean isEmpty(){
			return reqList.size() == 0;
		}
	}
}
