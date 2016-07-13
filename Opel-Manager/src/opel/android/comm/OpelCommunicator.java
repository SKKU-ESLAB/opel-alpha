package opel.android.comm;

import android.util.Log;

public class OpelCommunicator {
	static public OpelServer openChannel(String intf_name, OpelCallbacks defCb, OpelCallbacks errCb) {
		OpelServer op_server = new OpelServer(intf_name, OpelSocket.CONN_TYPE_BT, defCb, errCb);
		
		op_server.Start();
		
		return op_server;
	}
	static public OpelClient connectChannel(String intf_name, OpelCallbacks defCb, OpelCallbacks errCb){
		OpelClient op_client = new OpelClient(intf_name, defCb, errCb);
		
		Log.d(OpelUtil.TAG, "Connect");
		
		op_client.Start();
		
		return op_client;
	}
}
