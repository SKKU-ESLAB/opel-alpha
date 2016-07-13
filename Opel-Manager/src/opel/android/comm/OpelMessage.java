package opel.android.comm;

import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;

public class OpelMessage {
	static final int PACKET_TYPE_MSG = 1;
	static final int PACKET_TYPE_FILE = 2;
	static final int PACKET_TYPE_ACK = 4;
	static final int PACKET_TYPE_SPE = 8;
	
	private OPEL_Header op_header;
	private OpelSocket op_sock;
	byte[] data;
	
	private class OPEL_Header {
		public int req_id;
		public int data_len;
		public short type;
		public short err;
		
		//Only if type &= PACKET_TYPE_FILE
		public String fname;
		public int fsize;
		public int offset;
		public String destFname;
		
		public boolean initialized;
		
		OPEL_Header(){
			req_id = 0;
			data_len = 0;
			type = 0;
			fname = null;
			fsize = 0;
			offset = 0;
			initialized = false;
			destFname = null;
		}
		OPEL_Header(OPEL_Header arg){
			req_id = arg.req_id;
			data_len = arg.data_len;
			type = arg.type;
			if(arg.fname == null)
				fname = null;
			else
				fname = new String(arg.fname);
			fsize = arg.fsize;
			offset = arg.offset;
			initialized = arg.initialized;
			if(arg.destFname == null)
				destFname = null;
			else
				destFname = new String(arg.destFname);
		}
		
		private byte[] stob(short s){
			ByteOrder order = ByteOrder.LITTLE_ENDIAN;
			ByteBuffer buff = ByteBuffer.allocate(2);
			buff.order(order);
			
			buff.putShort(s);
			
			return buff.array();
		}
		private byte[] itob(int integer){
			ByteOrder order = ByteOrder.LITTLE_ENDIAN;
			ByteBuffer buff = ByteBuffer.allocate(4);
			buff.order(order);
	 
			buff.putInt(integer);
	
			return buff.array();
		}
		private short btos(byte[] bytes){
			ByteOrder order = ByteOrder.LITTLE_ENDIAN;
			ByteBuffer buff = ByteBuffer.allocate(2);
			buff.order(order);
			
			buff.put(bytes);
			buff.flip();
			
			return buff.getShort();
		}		
		private int btoi(byte[] bytes) {
			ByteOrder order = ByteOrder.LITTLE_ENDIAN;			 
			ByteBuffer buff = ByteBuffer.allocate(4);
			buff.order(order);
	 
			buff.put(bytes);
			buff.flip();
			
			return buff.getInt(); 
		}
		public boolean init_from_buff(byte[] buff){
			if(null == buff)
				return false;
			
			req_id = btoi(Arrays.copyOfRange(buff, 0, 4));
			data_len = btoi(Arrays.copyOfRange(buff, 4, 8));
			type = btos(Arrays.copyOfRange(buff, 8, 10));
			err = btos(Arrays.copyOfRange(buff, 10, 12));
			if(0 != (PACKET_TYPE_FILE & type)){
				fname = new String(Arrays.copyOfRange(buff, 14, 38));
				fsize = btoi(Arrays.copyOfRange(buff, 38, 42));
				offset = btoi(Arrays.copyOfRange(buff, 42, 46));
				destFname = new String(Arrays.copyOfRange(buff, 46, 70));
			}
			
			initialized = true;
			
			return true;
		}
		public boolean init_to_buff(byte[] buff){
			byte[] tmp_buff;
			
			Log.d(OpelUtil.TAG, "Bufflen:"+buff.length);
			tmp_buff = itob(req_id);
			try{
				for(int i=0; i<4; i++){
					buff[i] = tmp_buff[i];
				}
				tmp_buff = itob(data_len);
				for(int i=0; i<4; i++){
					buff[i+4] = tmp_buff[i];
				}
				tmp_buff = stob(type);
				for(int i=0; i<2; i++){
					buff[i+8] = tmp_buff[i];
				}
				tmp_buff = stob(err);
				for(int i=0; i<2; i++){
					buff[i+10] = tmp_buff[i];
				}

				if(0 != (PACKET_TYPE_FILE & type)){
					tmp_buff = fname.getBytes();
					for(int i=0; i<tmp_buff.length; i++){
						buff[i+14] = tmp_buff[i];
					}
					tmp_buff = itob(fsize);
					for(int i=0; i<4; i++){
						buff[i+38] = tmp_buff[i];
					}
					tmp_buff = itob(offset);
					for(int i=0; i<4; i++){
						buff[i+42] = tmp_buff[i];
					}
					tmp_buff = destFname.getBytes();
					for(int i=0; i<tmp_buff.length; i++){
						buff[i+46] = tmp_buff[i];
					}
				}
				
			} catch(IndexOutOfBoundsException e){
				e.printStackTrace();
				Log.e("OPEL_MSG", "init_to_buff: buff is small error",e);
			}
			
			return true;
		}
		public boolean isInitialized(){
			return initialized;
		}
		public void complete(){
			initialized = true;
		}
	}
	
	public OpelMessage(){
		op_header = new OPEL_Header();
		data = null;
		op_sock = null;
	}
	public OpelMessage(OpelMessage arg){
		Log.d(OpelUtil.TAG, "Sending Message3.2.0 ");
		op_header = new OPEL_Header(arg.op_header);
		Log.d(OpelUtil.TAG, "Sending Message3.2.1 ");
		if(null == arg.data)
			data = null;
		else{
			data = arg.data.clone();
		}
		op_sock = arg.op_sock;
		Log.d(OpelUtil.TAG, "Sending Message3.2.2 ");
	}
	
	public boolean init_from_buff(byte[] buff){
		return op_header.init_from_buff(buff);
	}
	public boolean init_to_buff(byte[] buff){
		return op_header.init_to_buff(buff);
	}
	
	public byte[] get_data(){
		return data;
	}
	
	public short get_err(){
		return op_header.err;
	}
	
	public void set_data(byte[] data, int data_len){
		if(data != null)
			this.data = data.clone();
		op_header.data_len = data_len;
		op_header.complete();
	}
	
	public int get_req_id(){
		if(!op_header.isInitialized())
			return -1;
		return op_header.req_id;
	}
	
	public int get_data_len(){
		if(!op_header.isInitialized())
			return -1;
		return op_header.data_len;
	}
	
	public boolean is_file(){
		if(!op_header.isInitialized())
			return false;
		if((op_header.type & PACKET_TYPE_FILE) != 0)
			return true;
		else 
			return false;
	}
	
	public boolean is_msg(){
		if(!op_header.isInitialized())
			return false;
		if(((op_header.type) & PACKET_TYPE_MSG) != 0)
			return true;
		else
			return false;
	}
	
	public boolean is_ack(){
		if(!op_header.isInitialized())
			return false;
		if(((op_header.type) & PACKET_TYPE_ACK) != 0)
			return true;
		else
			return false;
	}
	
	public boolean is_special(){
		if(!op_header.isInitialized())
			return false;
		if(((op_header.type) & PACKET_TYPE_SPE) != 0)
			return true;
		else
			return false;
	}
	
	public void set_special(){
		op_header.type |= PACKET_TYPE_SPE;
	}
	
	public int get_file_offset(){
		if(!is_file())
			return -1;
		else
			return op_header.offset;
	}
	
	public int get_file_size(){
		if(!is_file())
			return -1;
		else
			return op_header.fsize;
	}
	public String get_file_name(){
		if(!is_file())
			return null;
		else
			return op_header.fname;
	}
	
	public void set_req_id(int req){
		op_header.req_id = req;
	}
	
	public void set_file(String fname, int offset, int size){
		if(null != fname){
			op_header.fname = new String(fname);
		}
		
		this.op_header.fsize = size;
		this.op_header.offset = offset;
		
		op_header.type &= PACKET_TYPE_ACK;
		op_header.type |= PACKET_TYPE_FILE;
	}
	
	public void set_file(String srcFname, String destFname, int offset, int size){
		if(null != destFname){
			op_header.destFname = new String(destFname);
		}
		
		set_file(srcFname, offset, size);
	}
	
	public void set_msg(){
		op_header.type &= PACKET_TYPE_ACK;
		op_header.type |= PACKET_TYPE_MSG;
	}
	
	public void set_ack(){
		op_header.type |= PACKET_TYPE_ACK;
	}
	
	public void set_err(short err){
		op_header.err = err;
	}
	
	public void set_op_sock(OpelSocket op_sock){
		this.op_sock = op_sock;
	}
	
	public OpelSocket get_op_sock(){
		return op_sock;
	}
	
	public void complete_header(){
		op_header.complete();
	}

}
