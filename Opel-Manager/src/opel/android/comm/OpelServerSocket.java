package opel.android.comm;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.IOException;

public class OpelServerSocket {
	static final byte CONN_TYPE_BT = 1;
	static final byte CONN_TYPE_WD = 2;
	static final byte CONN_TYPE_WF = 4;
	
	static final int STAT_LISTEN = 0;
	
	private byte conn_type;
	
	String intf_name;
	
	BluetoothServerSocket bt_server;
	
	
	public OpelServerSocket(String intf_name, byte conn_type){
		this.intf_name = new String(intf_name);
		this.conn_type = conn_type;
		
		switch(conn_type){
		case CONN_TYPE_BT:
			BluetoothServerSocket tmp = null;
			BluetoothAdapter tmpAdapter = BluetoothAdapter.getDefaultAdapter();
			try {
				tmp = tmpAdapter.listenUsingInsecureRfcommWithServiceRecord(intf_name, OpelUtil.name2uuid(intf_name));
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			bt_server = tmp;
			
			break;
		case CONN_TYPE_WD:
		case CONN_TYPE_WF:
		default:
			break;
		}	
	}
	
	public OpelSocket accept(){
		OpelSocket op_sock = null;
		switch(conn_type){
		case CONN_TYPE_BT:
			if(bt_server == null)
				Log.d(OpelUtil.TAG, "Not listening");
			else{
				BluetoothSocket tmp_cli = null;
				
				try{
					tmp_cli = bt_server.accept();
				} catch(IOException e){
					e.printStackTrace();
					Log.e(OpelUtil.TAG, "accpet failed", e);
				}
				
				if(tmp_cli != null){
					op_sock = new OpelSocket(intf_name, conn_type, tmp_cli);
				}				
			}				
			break;
		case CONN_TYPE_WD:
		case CONN_TYPE_WF:
		default:
			break;
		}
		
		return op_sock;
	}
}
