package com.opel.opel_manager.view;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.NotificationCompat;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.opel.cmfw.glue.CommBroadcastReceiver;
import com.opel.cmfw.glue.CommChannelEventListener;
import com.opel.cmfw.service.CommChannelService;
import com.opel.opel_manager.R;
import com.opel.opel_manager.controller.LegacyAppCoreStub;
import com.opel.opel_manager.controller.LegacyJSONParser;
import com.opel.opel_manager.controller.OPELContext;
import com.opel.opel_manager.model.OPELAppList;
import com.opel.opel_manager.model.OPELApplication;

import java.util.ArrayList;

import static com.opel.opel_manager.controller.OPELContext.getAppList;

public class MainActivity extends Activity implements CommChannelEventListener {
    private static final String TAG = "MainActivity";
    // RPC on CommChannelService
    private CommChannelService mCommChannelService = null;
    private CommBroadcastReceiver mCommBroadcastReceiver;
    private final MainActivity self = this;

    // IconGridView
    private GridView mIconGridView;
    private IconListAdapter mIconListAdapter;

    // Wi-fi Direct
    private boolean mIsWaitingWifiDirectOnForCamera = false;

    private PortIndicator mDefaultPortIndicator;
    private PortIndicator mLargeDataPortIndicator;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Initialize UI Layout
        setContentView(R.layout.activity_main);

        // Check Permission
        this.checkStoragePermission();

        // Initialize OPELContext "only once on launch"
        OPELContext.initialize(this, this.mHandler);

        // Initialize UI Contents
        this.initializeUIContents();

        // Initialize Communication
        this.initializeCommunication();
    }

    protected void onRestart() {
        super.onRestart();
        this.updateUI();
    }

    protected void onPause() {
        super.onPause();
        this.updateUI();
    }

    protected void onResume() {
        super.onResume();
        this.updateUI();
    }

    protected void onDestroy() {
        super.onDestroy();
        unbindService(this.mCommServiceConnection);
        OPELContext.finish();
    }

    private void checkStoragePermission() {
        // Check storage read/write permission
        if (this.checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager
                .PERMISSION_GRANTED) {
            Log.d("OPEL", "Requesting Permission");

            if (ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission
                    .READ_EXTERNAL_STORAGE)) {
                Toast.makeText(this, "Storage permission is required to start " + "OPEL Manager"
                        + ".", Toast.LENGTH_SHORT).show();
            }
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission
                    .READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
        } else {
            Log.d("OPEL", "Permission granted");
        }
    }

    public void onRequestPermissionsResult(int requestCode, String permissions[], int[]
            grantResults) {
        if (grantResults.length <= 0 || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
            Toast.makeText(this, "Failed to be granted", Toast.LENGTH_LONG).show();
            this.finish();
        }
    }

    private void initializeUIContents() {
        //Create IconListAdapter
        this.mIconGridView = (GridView) findViewById(R.id.iconGridView);
        OPELAppList list = getAppList();
        this.mIconListAdapter = new IconListAdapter(this, R.layout.template_gridview_icon_main,
                list.getList());
        this.mIconGridView.setAdapter(mIconListAdapter);
        this.mIconGridView.setOnItemClickListener(mItemClickListener);
        this.mIconGridView.setOnItemLongClickListener(mItemLongClickListener);

        this.mDefaultPortIndicator = new PortIndicator(R.id.indicatorBT, R.drawable
                .bluetooth_disabled, R.drawable.bluetooth);
        this.mLargeDataPortIndicator = new PortIndicator(R.id.indicatorWFD, R.drawable
                .wifidirect_disabled, R.drawable.wifidirect);
    }

    private void initializeCommunication() {
        // Launch CommChannelService for setting connection with target OPEL device.
        Intent serviceIntent = new Intent(this, CommChannelService.class);
        this.bindService(serviceIntent, this.mCommServiceConnection, Context.BIND_AUTO_CREATE);
    }

    private ServiceConnection mCommServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder inputBinder) {
            CommChannelService.CommBinder serviceBinder = (CommChannelService.CommBinder)
                    inputBinder;
            mCommChannelService = serviceBinder.getService();

            // Set CommBroadcastReceiver and CommChannelEventListener
            IntentFilter broadcastIntentFilter = new IntentFilter();
            broadcastIntentFilter.addAction(CommBroadcastReceiver.ACTION);
            mCommBroadcastReceiver = new CommBroadcastReceiver(self);
            registerReceiver(mCommBroadcastReceiver, broadcastIntentFilter);

            // Request to connect channel
            OPELContext.getAppCore().setCommService(mCommChannelService);
            mCommChannelService.connectChannel(); // RPC to CommChannelService
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            unregisterReceiver(mCommBroadcastReceiver);
            mCommChannelService = null;
        }
    };

    private GridView.OnItemClickListener mItemClickListener = new GridView.OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> arg0, View v, int position, long arg3) {
            AlertDialog.Builder alertDlg = new AlertDialog.Builder(MainActivity.this);
            OPELAppList appList = getAppList();

            // Native Menu Handling
            int appType = appList.getList().get(position).getType();
            String appName = appList.getList().get(position).getTitle();
            if (appType == -1) {
                launchDefaultApp(appName);
            } else if (appType == 0) {
                //Run OPELApplication if it is not running
                String appId = "" + appList.getList().get(position).getAppId();
                launchUserApp(appId, appName);
            } else if (appType == 1) {
                Toast.makeText(getApplicationContext(), appList.getList().get(position).getTitle
                        () + " is " + "OPEN", Toast.LENGTH_SHORT).show();
                OPELApplication app = appList.getList().get(position);
                showRemoteConfigUI(app);
            }
        }
    };

    private GridView.OnItemLongClickListener mItemLongClickListener = new GridView
            .OnItemLongClickListener() {
        public boolean onItemLongClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
            final OPELAppList appList = getAppList();
            OPELApplication app = OPELContext.getAppList().getList().get(position);
            int appType = app.getType();
            if (appType == -1) {
                // Default Apps
                Toast.makeText(getApplicationContext(), "[Native app]" + appList.getList().get
                        (position).getTitle(), Toast.LENGTH_SHORT).show();
            } else if (appType == 0) {
                // Apps Not Running
                Toast.makeText(getApplicationContext(), "[Installed]" + appList.getList().get
                        (position).getTitle(), Toast.LENGTH_SHORT).show();
            } else if (appType == 1) {
                terminateUserApp(app);
            }
            return true;
        }
    };

    class PortIndicator {
        public static final int STATE_DISCONNECTED = 0;
        public static final int STATE_CONNECTING = 1;
        public static final int STATE_CONNECTED = 2;

        private ImageView mIndicatorImageView;
        private ConnectingAnimationThread mConnectingAnimationThread;
        private int mState;
        private int mDisconnectedResourceId;
        private int mConnectedResourceId;

        public PortIndicator(int indicatorImageViewId, int disconnectedResourceId, int
                connectedResourceId) {
            this.mIndicatorImageView = (ImageView) findViewById(indicatorImageViewId);
            this.mState = STATE_DISCONNECTED;
            this.mDisconnectedResourceId = disconnectedResourceId;
            this.mConnectedResourceId = connectedResourceId;
            this.mConnectingAnimationThread = new ConnectingAnimationThread();
        }

        public int getState() {
            return this.mState;
        }

        public void setDisconnected() {
            this.mState = STATE_DISCONNECTED;
            this.mConnectingAnimationThread.disable();
            this.mIndicatorImageView.setImageResource(mDisconnectedResourceId);
        }

        public void setConnecting() {
            this.mState = STATE_CONNECTING;
            this.mConnectingAnimationThread.start();
        }

        public void setConnected() {
            this.mState = STATE_CONNECTED;
            this.mConnectingAnimationThread.disable();
            mIndicatorImageView.setImageResource(mConnectedResourceId);
        }

        class ConnectingAnimationThread extends Thread {
            private static final int SLEEP_MILLISECS = 300;
            private boolean mIsEnabled = false;

            public void disable() {
                this.mIsEnabled = false;
            }

            @Override
            public void run() {
                boolean imageViewConnected = false;
                this.mIsEnabled = true;
                while (this.mIsEnabled) {
                    try {
                        final boolean fImageViewConnected = imageViewConnected;
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                if (fImageViewConnected) {
                                    mIndicatorImageView.setImageResource(mConnectedResourceId);
                                } else {
                                    mIndicatorImageView.setImageResource(mDisconnectedResourceId);
                                }
                            }
                        });

                        imageViewConnected = !imageViewConnected;

                        Thread.sleep(SLEEP_MILLISECS);
                    } catch (InterruptedException e) {
                    }
                }

                final int fState = mState;
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (fState == STATE_CONNECTED) {
                            mIndicatorImageView.setImageResource(mConnectedResourceId);
                        } else if (fState == STATE_DISCONNECTED) {
                            mIndicatorImageView.setImageResource(mDisconnectedResourceId);
                        }
                    }
                });
            }
        }
    }

    // Input Json Format
    /*{
     * 		"appTitle":"collision detector",
     * 		appID: "2",
    		time: "2015-08-02. 15:02",
    		description: "collision detection!!!",
    		text: "collision is detected!!",
    		img: 234234
      }
    */
    public void makeNotification(String JsonData) {

        NotificationManager nm = (NotificationManager) getSystemService(Context
                .NOTIFICATION_SERVICE);
        Resources res = getResources();

        Intent notificationIntent = new Intent(this, RemoteNotiUIActivity.class);
        Bundle extras = new Bundle();
        extras.putString("jsonData", JsonData);
        extras.putString("checkNoti", "1");
        notificationIntent.putExtras(extras);

        PendingIntent contentIntent = PendingIntent.getActivity(this, 1, notificationIntent,
                PendingIntent.FLAG_UPDATE_CURRENT);

        LegacyJSONParser jp = new LegacyJSONParser(JsonData);
        String appId = jp.getValueByKey("appID");
        OPELApplication targetApp = OPELContext.getAppList().getApp(appId);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(this).setCategory
                (appId).setContentTitle(targetApp.getTitle()).setContentText(jp.getValueByKey
                ("description")).setTicker(" " + jp.getValueByKey("appTitle")).setLargeIcon
                (OPELContext.getAppList().getApp(appId).getImage()).setSmallIcon(R.drawable.opel)
                .setContentIntent(contentIntent).setAutoCancel(true).setWhen(System
                        .currentTimeMillis()).setDefaults(Notification.DEFAULT_SOUND |
                        Notification.DEFAULT_VIBRATE | Notification.DEFAULT_LIGHTS).setNumber(1);
        Notification n = builder.build();
        nm.notify(1234, n);
    }

    public void updateUI() {
        mIconListAdapter.updateDisplay();
    }

    private void terminateUserApp(OPELApplication app) {
        // Apps Running
        AlertDialog.Builder alt_bld = new AlertDialog.Builder(MainActivity
                .this);
        final OPELApplication fApp = app;
        alt_bld.setMessage("Terminate this App ?").setCancelable(false).setPositiveButton("Yes",
                new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                // Action for 'Yes' Button
                OPELContext.getAppCore().requestTermination("" + fApp.getAppId());
                Log.d("OPEL", "Request to kill ");
            }
        }).setNegativeButton("No", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                // Action for 'NO' Button
                dialog.cancel();
            }
        });

        AlertDialog alert = alt_bld.create();
        alert.setTitle(app.getTitle());
        Drawable d = new BitmapDrawable(getResources(), app.getImage());
        alert.setIcon(d);
        alert.show();
    }

    private void launchAppManager() {
        if (this.mCommChannelService != null && !this.mCommChannelService.isDefaultPortAvailable
                ()) {
            Toast.makeText(getApplicationContext(), "Disconnected to OPEL", Toast.LENGTH_SHORT)
                    .show();
        }
        Intent intent = new Intent(MainActivity.this, AppManagerActivity.class);
        startActivity(intent);
    }

    private void launchAppMarket() {
        if (this.mCommChannelService != null && !this.mCommChannelService.isDefaultPortAvailable
                ()) {
            Toast.makeText(getApplicationContext(), "Disconnected to OPEL", Toast.LENGTH_SHORT)
                    .show();
        }
        Intent intent = new Intent(MainActivity.this, AppMarketActivity.class);
        startActivity(intent);
    }

    private void launchConnect() {
        if (this.mCommChannelService != null && !this.mCommChannelService.isDefaultPortAvailable
                ()) {
            this.mCommChannelService.connectChannel();
        } else
            Toast.makeText(getApplicationContext(), "Already connected to OPEL device", Toast
                    .LENGTH_SHORT).show();
        return;
    }

    private void launchFileManager() {
        if (this.mCommChannelService != null && !this.mCommChannelService.isDefaultPortAvailable
                ()) {
            Toast.makeText(getApplicationContext(), "Disconnected to OPEL", Toast.LENGTH_SHORT)
                    .show();
            return;
        }

        Intent intent = new Intent(MainActivity.this, FileManagerActivity.class);
        startActivity(intent);
    }

    private void launchCameraBeforeWifiDirectConnected() {
        if (this.mCommChannelService != null && !this.mCommChannelService.isDefaultPortAvailable
                ()) {
            Toast.makeText(getApplicationContext(), "Disconnected to OPEL", Toast.LENGTH_SHORT)
                    .show();
            return;
        }

        mIsWaitingWifiDirectOnForCamera = true;
        this.mCommChannelService.enableLargeDataMode();
    }

    private void launchCameraAfterWifiDirectConnected() {
        Intent intent = new Intent(MainActivity.this, CameraViewerActivity.class);
        startActivity(intent);
    }

    private void launchSensor() {
        if (this.mCommChannelService != null && !this.mCommChannelService.isDefaultPortAvailable
                ()) {
            Toast.makeText(getApplicationContext(), "Disconnected" + " to " + "OPEL", Toast
                    .LENGTH_SHORT).show();
            return;
        }
        Intent intent = new Intent(MainActivity.this, SensorViewerActivity.class);
        startActivity(intent);
    }

    private void launchEventLogger() {
        Intent intent = new Intent(MainActivity.this, EventLoggerActivity.class);
        startActivity(intent);
    }

    private void launchDefaultApp(String appName) {
        if (appName.compareTo("App Manager") == 0) {
            launchAppManager();
        } else if (appName.compareTo("App Market") == 0) {
            launchAppMarket();
        } else if (appName.compareTo("Connect") == 0) {
            launchConnect();
        } else if (appName.compareTo("File Manager") == 0) {
            launchFileManager();
        } else if (appName.compareTo("Camera") == 0) {
            launchCameraBeforeWifiDirectConnected();
        } else if (appName.compareTo("Sensor") == 0) {
            launchSensor();
        } else if (appName.compareTo("Event Logger") == 0) {
            launchEventLogger();
        }
    }

    private void launchUserApp(String appId, String appName) {
        LegacyAppCoreStub legacyAppCoreStub = OPELContext.getAppCore();
        legacyAppCoreStub.requestStart(appId, appName);
    }

    private void showRemoteConfigUI(OPELApplication app) {
        //Open configuration view if it is running
        if (app.getConfigJson().equals("N/A")) {
            Toast.makeText(getApplicationContext(), "Configurable " + "data " + "is N/A", Toast
                    .LENGTH_SHORT).show();
        } else {
            Intent intent = new Intent(MainActivity.this, RemoteConfigUIActivity.class);

            Bundle extras = new Bundle();
            extras.putString("title", app.getTitle());
            extras.putString("appID", "" + app.getAppId());
            extras.putString("jsonData", app.getConfigJson());

            intent.putExtras(extras);
            startActivity(intent);
        }
    }

    // CommChannelEventListener
    @Override
    public void onCommChannelStateChanged(int prevState, int newState) {
        Log.d(TAG, "CommChannel State change: " + prevState + " -> " + newState);
        switch (newState) {
            case CommChannelService.STATE_DISCONNECTED:
                mDefaultPortIndicator.setDisconnected();
                mLargeDataPortIndicator.setDisconnected();

                switch (prevState) {
                    case CommChannelService.STATE_CONNECTING_DEFAULT:
                        Toast.makeText(getApplicationContext(), "Failed connecting to OPEL " +
                                "device. Retry it.", Toast.LENGTH_LONG).show();
                        break;
                    case CommChannelService.STATE_CONNECTED_DEFAULT:
                    case CommChannelService.STATE_CONNECTING_LARGE_DATA:
                    case CommChannelService.STATE_CONNECTED_LARGE_DATA:
                        Toast.makeText(getApplicationContext(), "OPEL device is disconnected.",
                                Toast.LENGTH_LONG).show();
                        break;
                }
                break;
            case CommChannelService.STATE_CONNECTING_DEFAULT:
                mDefaultPortIndicator.setConnecting();
                mLargeDataPortIndicator.setDisconnected();
                break;
            case CommChannelService.STATE_CONNECTED_DEFAULT:
                mDefaultPortIndicator.setConnected();
                mLargeDataPortIndicator.setDisconnected();

                switch (prevState) {
                    case CommChannelService.STATE_CONNECTING_DEFAULT:
                        Toast.makeText(getApplicationContext(), "OPEL device is connected.",
                                Toast.LENGTH_LONG).show();
                        OPELContext.getAppCore().requestUpdateAppInfomation();
                        break;
                    case CommChannelService.STATE_CONNECTING_LARGE_DATA:
                        Toast.makeText(getApplicationContext(), "Opening large data port is " +
                                "failed.", Toast.LENGTH_LONG).show();
                        break;
                    case CommChannelService.STATE_CONNECTED_LARGE_DATA:
                        Toast.makeText(getApplicationContext(), "Large data port is closed.",
                                Toast.LENGTH_LONG).show();
                        break;
                }
                break;
            case CommChannelService.STATE_CONNECTING_LARGE_DATA:
                mDefaultPortIndicator.setConnected();
                mLargeDataPortIndicator.setConnecting();
                break;
            case CommChannelService.STATE_CONNECTED_LARGE_DATA:
                mDefaultPortIndicator.setConnected();
                mLargeDataPortIndicator.setConnected();

                Toast.makeText(getApplicationContext(), "Large data port is opened.", Toast
                        .LENGTH_LONG).show();

                if (this.mIsWaitingWifiDirectOnForCamera) {
                    this.launchCameraAfterWifiDirectConnected();
                    this.mIsWaitingWifiDirectOnForCamera = false;
                }
                break;
        }
    }

    @Override
    public void onReceivedRawMessage(String message, String filePath) {
        // TODO: forward to LegacyAppCoreStub
        OPELContext.getAppCore().onReceivedMessage(message, filePath);
    }

    // TODO: Convert to CommServiceListener
    // TODO: Convert to Comm-related callbacks
    private android.os.Handler mHandler = new android.os.Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message inputMessage) {
            if (inputMessage.what == LegacyAppCoreStub.HANDLER_UPDATE_UI) {
                //String msg = (String) inputMessage.obj;
                updateUI();
                // TODO: convert to CommBroadcastReceiver
                AppManagerActivity.updateDisplay();
                EventLoggerActivity.updateDisplay();

                //update All of the UI page
            } else if (inputMessage.what == LegacyAppCoreStub.HANDLER_UPDATE_TOAST) {
                String toastMsg = (String) inputMessage.obj;
                Toast.makeText(getApplicationContext(), toastMsg, Toast.LENGTH_SHORT).show();
                //update All of the UI page
            } else if (inputMessage.what == LegacyAppCoreStub.HANDLER_MAKE_NOTI) {
                String notiJson = (String) inputMessage.obj;
                updateUI();
                AppManagerActivity.updateDisplay();
                makeNotification(notiJson);

                //update All of the UI page
            } else if (inputMessage.what == LegacyAppCoreStub.HANDLER_UPDATE_FILEMANAGER) {
                LegacyJSONParser jp = (LegacyJSONParser) inputMessage.obj;
                FileManagerActivity.updateDisplay(jp);

            } else if (inputMessage.what == LegacyAppCoreStub.HANDLER_EXE_FILE) {
                LegacyJSONParser jp = (LegacyJSONParser) inputMessage.obj;
                FileManagerActivity.runRequestedFile(getApplicationContext(), jp);

            } else if (inputMessage.what == LegacyAppCoreStub.HANDLER_SHARE_FILE) {
                LegacyJSONParser jp = (LegacyJSONParser) inputMessage.obj;
                FileManagerActivity.runSharingFile(getApplicationContext(), jp);
            }
        }
    };
}


class IconListAdapter extends ArrayAdapter<OPELApplication> {
    Context context;
    int layoutResourceId;
    ArrayList<OPELApplication> data;


    public IconListAdapter(Context context, int layoutResourceId, ArrayList<OPELApplication> data) {
        super(context, layoutResourceId, data);
        this.layoutResourceId = layoutResourceId;
        this.context = context;
        this.data = data;
    }

    public void updateDisplay() {
        this.data = getAppList().getList();
        this.notifyDataSetChanged();
    }


    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View row = convertView;
        RecordHolder holder = null;

        if (row == null) {
            LayoutInflater inflater = ((Activity) context).getLayoutInflater();
            row = inflater.inflate(layoutResourceId, parent, false);

            holder = new RecordHolder();
            holder.txtTitle = (TextView) row.findViewById(R.id.item_text);
            holder.imageItem = (ImageView) row.findViewById(R.id.item_image);
            row.setTag(holder);
        } else {
            holder = (RecordHolder) row.getTag();
        }

        OPELApplication item = data.get(position);
        holder.txtTitle.setText(item.getTitle());
        holder.imageItem.setImageBitmap(item.getImage());
        return row;

    }

    static class RecordHolder {
        TextView txtTitle;
        ImageView imageItem;
    }
}