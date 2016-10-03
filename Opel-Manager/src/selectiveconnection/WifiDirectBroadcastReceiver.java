package selectiveconnection;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.NetworkInfo;
import android.net.wifi.WpsInfo;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pGroup;
import android.net.wifi.p2p.WifiP2pInfo;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.PeerListListener;
import android.util.Log;

import com.example.opel_manager.globalData;

/**
 * Created by eslab on 2016-05-16.
 */
public class WifiDirectBroadcastReceiver extends BroadcastReceiver {
    private WifiP2pManager mManager;
    private WifiP2pManager.Channel mChannel;
    private WifiP2pDevice opelDevice;

    public WifiP2pManager.ConnectionInfoListener mConnectionListener;
    public PeerListListener mPeerListener;

    static public String mMac = null;

    boolean sent_connected = false;
    boolean connection = false;

    public boolean removing = false;

    public WifiDirectBroadcastReceiver(WifiP2pManager manager, WifiP2pManager.Channel channel){
        super();

        this.mManager = manager;
        this.mChannel = channel;
        opelDevice = null;

        mPeerListener = new PeerListListener() {
            @Override
            public void onPeersAvailable(WifiP2pDeviceList peers) {

                WifiP2pConfig config = new WifiP2pConfig();

                for(WifiP2pDevice device : peers.getDeviceList()){
                    if(device.deviceName.equals(OpelCommunicator.getTargetWfdName())) {
                        Log.d("BReceiver", "Found device connecting...");
                        opelDevice = device;
                        if(opelDevice.status != WifiP2pDevice.AVAILABLE)
                            return;

                        if (device.status == WifiP2pDevice.AVAILABLE) {
                            config.deviceAddress = device.deviceAddress;

                            if(device.wpsPbcSupported()) {
                                config.wps.setup = WpsInfo.PBC;
                                Log.d("WPSINFO", "PBC");
                            }
                            else if(device.wpsKeypadSupported()){
                                config.wps.setup = WpsInfo.KEYPAD;
                                config.wps.pin = new String("12345670");
                                Log.d("WPSINFO", "KeyPad");
                            }
                            else{
                                config.wps.setup = WpsInfo.DISPLAY;
                                Log.d("WPSINFO", "Display");
                            }
                            mManager.connect(mChannel, config, new WifiP2pManager.ActionListener() {
                                @Override
                                public void onSuccess() {
                                    Log.d("BReceiver", "Succedded to send connect msg");
                                    sent_connected = true;
                                }

                                @Override
                                public void onFailure(int reason) {
                                    Log.d("BReceiver", "Failed to connect" + Integer.toString(reason));
                                    sent_connected = false;
                                }
                            });
                            break;
                        }
                        else if(device.status==WifiP2pDevice.INVITED){
                            Log.d("Manager", "Invited");

                        }
                    }
                }
            }
        };

        mConnectionListener = new WifiP2pManager.ConnectionInfoListener() {
            @Override
            public void onConnectionInfoAvailable(WifiP2pInfo info) {
            }
        };
    }
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();

        if(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION.equals(action  )){
            // Check to see if Wi-Fi is enabled and notify appropriate activity

            int state = intent.getIntExtra(WifiP2pManager.EXTRA_WIFI_STATE, -1);
            if(state == WifiP2pManager.WIFI_P2P_STATE_ENABLED){
                Log.d("WifiP2p", "State: Enabled");
            }
            else{
                Log.d("WifiP2p", "State: Disabled");
            }
        } else if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.equals(action)) {
            // Call WifiP2pManager.requestPeers() to get a list of current peers
            if (mManager != null) {
                mManager.requestPeers(mChannel, mPeerListener);
            }


        } else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.equals(action)) {
            // Respond to new connection or disconnections

                NetworkInfo networkInfo = (NetworkInfo) intent
                        .getParcelableExtra(WifiP2pManager.EXTRA_NETWORK_INFO);

                if (networkInfo.isConnected()) {
                    WifiP2pGroup p2pGroup = (WifiP2pGroup)intent.getParcelableExtra(WifiP2pManager.EXTRA_WIFI_P2P_GROUP);
                    if(p2pGroup.getOwner().deviceName != null && p2pGroup.getOwner().deviceName.equals(OpelCommunicator.getTargetWfdName())) {
                        Log.d("Breceiver", "Connected:" + Integer.toString(globalData.getInstance().getCommManager().opelCommunicator.wfd_in_use));

                        opelDevice = p2pGroup.getOwner();
                        connection = true;

                        if (isConnected() && globalData.getInstance().getCommManager().opelCommunicator.wfd_in_use == 0)
                            globalData.getInstance().getCommManager().opelCommunicator.cmfw_wfd_off();
                    }
                } else if (networkInfo.isConnectedOrConnecting()) {
                    Log.d("Breceiver", "Connecting");
                } else if (networkInfo.isAvailable()) {
                    if(opelDevice != null) {
                        Log.d("Breceiver", "Available");
                        opelDevice = null;
                        removing = false;
                        connection = false;
                    }
                }

        } else if (WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION.equals(action)) {
            // Respond to this device's wifi state changing
        }
    }

    public boolean isConnected(){
        if(opelDevice == null)
            return false;

        Log.d("BReceiver", Integer.toString(opelDevice.status));

        if(removing == false)
            return connection;
        return false;
    }
}
