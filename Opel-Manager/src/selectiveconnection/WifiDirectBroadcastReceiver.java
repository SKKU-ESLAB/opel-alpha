package selectiveconnection;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.NetworkInfo;
import android.net.wifi.WpsInfo;
import android.net.wifi.p2p.WifiP2pConfig;
import android.net.wifi.p2p.WifiP2pDevice;
import android.net.wifi.p2p.WifiP2pDeviceList;
import android.net.wifi.p2p.WifiP2pInfo;
import android.net.wifi.p2p.WifiP2pManager;
import android.net.wifi.p2p.WifiP2pManager.PeerListListener;
import android.os.Handler;
import android.util.Log;

import java.util.Collection;
import java.util.Iterator;

import selectiveconnection.OpelCommunicator;

/**
 * Created by eslab on 2016-05-16.
 */
public class WifiDirectBroadcastReceiver extends BroadcastReceiver {
    private WifiP2pManager mManager;
    private WifiP2pManager.Channel mChannel;

    public WifiP2pManager.ConnectionInfoListener mConnectionListener;
    public PeerListListener mPeerListener;

    static public String mMac = null;

    boolean sent_connected = false;

    boolean connected = false;

    public WifiDirectBroadcastReceiver(WifiP2pManager manager, WifiP2pManager.Channel channel){
        super();

        this.mManager = manager;
        this.mChannel = channel;

        mPeerListener = new PeerListListener() {
            @Override
            public void onPeersAvailable(WifiP2pDeviceList peers) {

                WifiP2pConfig config = new WifiP2pConfig();
                for(WifiP2pDevice device : peers.getDeviceList()){
                    if(device.deviceName.equals(OpelCommunicator.CMFW_WFD_NAME)) {
                        Log.d("BReceiver", "Found device connecting...");
                        if (device.status == WifiP2pDevice.AVAILABLE) {
                            config.deviceAddress = device.deviceAddress;

                            if(device.wpsPbcSupported()) {
                                config.wps.setup = WpsInfo.PBC;
                                Log.d("WPSINFO", "PBC");
                            }
                            else if(device.wpsKeypadSupported()){
                                config.wps.setup = WpsInfo.KEYPAD;
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
                                    mManager.stopPeerDiscovery(mChannel, null);
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
        } else if (WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION.equals(action)) {
            // Call WifiP2pManager.requestPeers() to get a list of current peers

        } else if (WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION.equals(action)) {
            // Respond to new connection or disconnections

        } else if (WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION.equals(action)) {
            // Respond to this device's wifi state changing
            Log.d("BReceiver", "Changed");
        }
    }

    public boolean isConnected(){
        return connected;
    }
}
