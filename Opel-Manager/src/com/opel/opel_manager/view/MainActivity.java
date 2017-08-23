package com.opel.opel_manager.view;

/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
 *              Dongig Sin<dongig@skku.edu>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.support.annotation.NonNull;
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

import com.opel.cmfw.service.CommChannelService;
import com.opel.opel_manager.R;
import com.opel.opel_manager.controller.LegacyAppCoreStub;
import com.opel.opel_manager.controller.LegacyJSONParser;
import com.opel.opel_manager.controller.OPELContext;
import com.opel.opel_manager.controller.OPELControllerBroadcastReceiver;
import com.opel.opel_manager.controller.OPELControllerService;
import com.opel.opel_manager.model.OPELApp;
import com.opel.opel_manager.model.OPELAppList;
import com.opel.opel_manager.view.main.MainIcon;

import java.util.ArrayList;

public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";

    // OPELControllerService
    private OPELControllerService mControllerServiceStub;
    private PrivateControllerBroadcastReceiver mControllerBroadcastReceiver;

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

        // Initialize OPELControllerService
        this.initializeControllerService();
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
        //Create MainIconListAdapter
        this.mIconGridView = (GridView) findViewById(R.id.iconGridView);
        this.mMainIconListAdapter = new MainIconListAdapter(this, R.layout.template_gridview_icon_main,
                this.mMainIconList);
        this.mIconGridView.setAdapter(mMainIconListAdapter);
        this.mIconGridView.setOnItemClickListener(mItemClickListener);
        this.mIconGridView.setOnItemLongClickListener(mItemLongClickListener);

        this.mDefaultPortIndicator = new PortIndicator(R.id.indicatorBT, R.drawable
                .bluetooth_disabled, R.drawable.bluetooth);
        this.mLargeDataPortIndicator = new PortIndicator(R.id.indicatorWFD, R.drawable
                .wifidirect_disabled, R.drawable.wifidirect);
    }

    public void initializeControllerService() {
        // Launch OPELControllerService for setting connection with target OPEL device.
        Intent serviceIntent = new Intent(this, MainActivity.class);
        this.bindService(serviceIntent, this.mControllerServiceConnection, Context
                .BIND_AUTO_CREATE);
    }

    private ServiceConnection mControllerServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder inputBinder) {
            OPELControllerService.ControllerBinder serviceBinder = (OPELControllerService
                    .ControllerBinder) inputBinder;
            mControllerServiceStub = serviceBinder.getService();

            // Set BroadcastReceiver
            IntentFilter broadcastIntentFilter = new IntentFilter();
            broadcastIntentFilter.addAction(OPELControllerBroadcastReceiver.ACTION);
            mControllerBroadcastReceiver = new PrivateControllerBroadcastReceiver();
            registerReceiver(mControllerBroadcastReceiver, broadcastIntentFilter);

            // Request to initialize connection
            connectTargetDevice();
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.d(TAG, "onServiceDisconnected()");
            unregisterReceiver(mControllerBroadcastReceiver);
            mControllerServiceStub = null;
        }
    };

    private GridView.OnItemClickListener mItemClickListener = new GridView.OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> arg0, View v, int position, long arg3) {
            AlertDialog.Builder alertDlg = new AlertDialog.Builder(MainActivity.this);
            OPELAppList appList = getAppList();

            // Native Menu Handling
            int appType = appList.getList().get(position).getLegacyType();
            String appName = appList.getList().get(position).getName();
            if (appType == -1) {
                launchDefaultApp(appName);
            } else if (appType == 0) {
                //Run OPELApp if it is not running
                int appId = appList.getList().get(position).getAppId();
                launchApp(appId);
            } else if (appType == 1) {
                Toast.makeText(getApplicationContext(), appList.getList().get(position).getName
                        () + " is " + "OPEN", Toast.LENGTH_SHORT).show();
                OPELApp app = appList.getList().get(position);
                showRemoteConfigUI(app);
            }
        }
    };

//    private GridView.OnItemLongClickListener mItemLongClickListener = new GridView
//            .OnItemLongClickListener() {
//        public boolean onItemLongClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
//            final OPELAppList appList = getAppList();
//            OPELApp app = OPELContext.getAppList().getList().get(position);
//            int appType = app.getLegacyType();
//            if (appType == -1) {
//                // Default Apps
//                Toast.makeText(getApplicationContext(), "[Native app]" + appList.getList().get
//                        (position).getName(), Toast.LENGTH_SHORT).show();
//            } else if (appType == 0) {
//                // Apps Not Running
//                Toast.makeText(getApplicationContext(), "[Installed]" + appList.getList().get
//                        (position).getName(), Toast.LENGTH_SHORT).show();
//            } else if (appType == 1) {
//                //terminateApp(app);
//            }
//            return true;
//        }
//    };

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
        OPELApp targetApp = OPELContext.getAppList().getApp(appId);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(this).setCategory
                (appId).setContentTitle(targetApp.getName()).setContentText(jp.getValueByKey
                ("description")).setTicker(" " + jp.getValueByKey("appTitle")).setLargeIcon
                (OPELContext.getAppList().getApp(appId).getIconImage()).setSmallIcon(R.drawable.opel)
                .setContentIntent(contentIntent).setAutoCancel(true).setWhen(System
                        .currentTimeMillis()).setDefaults(Notification.DEFAULT_SOUND |
                        Notification.DEFAULT_VIBRATE | Notification.DEFAULT_LIGHTS).setNumber(1);
        Notification n = builder.build();
        nm.notify(1234, n);
    }

    public void updateUI() {
        mMainIconListAdapter.updateUI();
    }

    public boolean isTargetDeviceConnected() {
        if (this.mControllerServiceStub == null) return false;

        int commChannelState = this.mControllerServiceStub.getCommChannelState();
        return (commChannelState != CommChannelService.STATE_DISCONNECTED) && (commChannelState
                != CommChannelService.STATE_CONNECTING_DEFAULT);
    }

    private void launchAppManager() {
        if (!isTargetDeviceConnected()) {
            Toast.makeText(getApplicationContext(), "Target device is not connected", Toast
                    .LENGTH_SHORT).show();
            return;
        }
        Intent intent = new Intent(MainActivity.this, AppManagerActivity.class);
        startActivity(intent);
    }

    private void launchAppMarket() {
        if (!isTargetDeviceConnected()) {
            Toast.makeText(getApplicationContext(), "Target device is not connected", Toast
                    .LENGTH_SHORT).show();
            return;
        }
        Intent intent = new Intent(MainActivity.this, AppMarketActivity.class);
        startActivity(intent);
    }

    public void connectTargetDevice() {
        if (isTargetDeviceConnected()) {
            Toast.makeText(getApplicationContext(), "Already connected to target device", Toast
                    .LENGTH_SHORT).show();
            return;
        }

        // Request to initialize connection
        mControllerServiceStub.initializeConnectionAsync();
    }

    private void launchFileManager() {
        if (!isTargetDeviceConnected()) {
            Toast.makeText(getApplicationContext(), "Target device is not connected", Toast
                    .LENGTH_SHORT).show();
            return;
        }
        Intent intent = new Intent(MainActivity.this, FileManagerActivity.class);
        startActivity(intent);
    }

    public void launchCameraViewer() {
        if (!isTargetDeviceConnected()) {
            Toast.makeText(getApplicationContext(), "Target device is not connected", Toast
                    .LENGTH_SHORT).show();
            return;
        }

        Intent intent = new Intent(MainActivity.this, CameraViewerActivity.class);
        startActivity(intent);
    }

    private void launchSensorViewer() {
        if (!isTargetDeviceConnected()) {
            Toast.makeText(getApplicationContext(), "Target device is not connected", Toast
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
            connectTargetDevice();
        } else if (appName.compareTo("File Manager") == 0) {
            launchFileManager();
        } else if (appName.compareTo("Camera") == 0) {
            launchCameraViewer();
        } else if (appName.compareTo("Sensor") == 0) {
            launchSensorViewer();
        } else if (appName.compareTo("Event Logger") == 0) {
            launchEventLogger();
        }
    }

    public void launchApp(int appId) {
        if (mControllerServiceStub == null) {
            Log.e(TAG, "ControllerService is not connected");
            return;
        }
        mControllerServiceStub.launchAppAsync(appId);
    }

    public void terminateApp(int appId) {
        if (mControllerServiceStub == null) {
            Log.e(TAG, "ControllerService is not connected");
            return;
        }
        mControllerServiceStub.terminateAppAsync(appId);
    }

    // TODO: Use OPELControllerServiceStub
    private void showRemoteConfigUI(OPELApp app) {
        //Open configuration view if it is running
        if (app.getConfigJSONString().equals("")) {
            Toast.makeText(getApplicationContext(), "Configurable " + "mMainIconList " + "is N/A", Toast
                    .LENGTH_SHORT).show();
        } else {
            Intent intent = new Intent(MainActivity.this, RemoteConfigUIActivity.class);

            Bundle extras = new Bundle();
            extras.putString("title", app.getName());
            extras.putString("appID", "" + app.getAppId());
            extras.putString("jsonData", app.getConfigJSONString());

            intent.putExtras(extras);
            startActivity(intent);
        }
    }

    // TODO: Convert to OPELControllerBraodcastReceiver
    private android.os.Handler mHandler = new android.os.Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message inputMessage) {
            if (inputMessage.what == LegacyAppCoreStub.HANDLER_UPDATE_UI) {
                // TODO: implement it in this Activity
                //String msg = (String) inputMessage.obj;
                updateUI();

                // TODO: move to AppManagerActivity
                AppManagerActivity.updateDisplay();

                // TODO: move to EventLoggerActivity
                EventLoggerActivity.updateDisplay();

                //update All of the UI page
            } else if (inputMessage.what == LegacyAppCoreStub.HANDLER_UPDATE_TOAST) {
                // TODO: implement it in this Activity
                String toastMsg = (String) inputMessage.obj;
                Toast.makeText(getApplicationContext(), toastMsg, Toast.LENGTH_SHORT).show();
                //update All of the UI page
            } else if (inputMessage.what == LegacyAppCoreStub.HANDLER_MAKE_NOTI) {
                // TODO: implement it in this Activity
                String notiJson = (String) inputMessage.obj;
                updateUI();
                AppManagerActivity.updateDisplay();
                makeNotification(notiJson);

                //update All of the UI page
            } else if (inputMessage.what == LegacyAppCoreStub.HANDLER_UPDATE_FILEMANAGER) {
                // TODO: move to FileManagerActivity
                LegacyJSONParser jp = (LegacyJSONParser) inputMessage.obj;
                FileManagerActivity.updateDisplay(jp);

            } else if (inputMessage.what == LegacyAppCoreStub.HANDLER_EXE_FILE) {
                // TODO: move to FileManagerActivity
                LegacyJSONParser jp = (LegacyJSONParser) inputMessage.obj;
                FileManagerActivity.runRequestedFile(getApplicationContext(), jp);

            } else if (inputMessage.what == LegacyAppCoreStub.HANDLER_SHARE_FILE) {
                // TODO: move to FileManagerActivity
                LegacyJSONParser jp = (LegacyJSONParser) inputMessage.obj;
                FileManagerActivity.runSharingFile(getApplicationContext(), jp);
            }
        }
    };

    class PrivateControllerBroadcastReceiver extends OPELControllerBroadcastReceiver {
        @Override
        public void onCommChannelStateChanged(int prevState, int newState) {
            // When connection state with target device is changed, indicator UI should be changed
            Log.d(TAG, "CommChannel State change: " + prevState + " -> " + newState);
            switch (newState) {
                case CommChannelService.STATE_DISCONNECTED:
                    mDefaultPortIndicator.setDisconnected();
                    mLargeDataPortIndicator.setDisconnected();

                    switch (prevState) {
                        case CommChannelService.STATE_CONNECTING_DEFAULT:
                            Toast.makeText(getApplicationContext(), "Failed connecting to OPEL "
                                    + "device. Retry it.", Toast.LENGTH_LONG).show();

                            break;
                        case CommChannelService.STATE_CONNECTED_DEFAULT:
                        case CommChannelService.STATE_CONNECTING_LARGE_DATA:
                        case CommChannelService.STATE_CONNECTED_LARGE_DATA:
                            Toast.makeText(getApplicationContext(), "OPEL device is disconnected"
                                    + ".", Toast.LENGTH_LONG).show();
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
                            mControllerServiceStub.getAppListAsync();
                            break;
                        case CommChannelService.STATE_CONNECTING_LARGE_DATA:
                            Toast.makeText(getApplicationContext(), "Opening large mMainIconList mPort is "
                                    + "failed.", Toast.LENGTH_LONG).show();
                            break;
                        case CommChannelService.STATE_CONNECTED_LARGE_DATA:
                            Toast.makeText(getApplicationContext(), "Large mMainIconList mPort is closed.",
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

                    Toast.makeText(getApplicationContext(), "Large mMainIconList mPort is opened.", Toast
                            .LENGTH_LONG).show();
                    break;
            }
        }
    }

    // IconGridView
    private GridView mIconGridView;
    private MainIconListAdapter mMainIconListAdapter;
    // TODO: add/remove item
    private ArrayList<MainIcon> mMainIconList = new ArrayList<>();

    class MainIconListAdapter extends ArrayAdapter<MainIcon> {
        Context mContext;
        int mLayoutResourceId;

        public MainIconListAdapter(Context context, int layoutResourceId, ArrayList<MainIcon> mainIconList) {
            super(context, layoutResourceId, mainIconList);
            this.mLayoutResourceId = layoutResourceId;
            this.mContext = context;
        }

        public void updateUI() {
            this.notifyDataSetChanged();
        }

        @NonNull
        @Override
        public View getView(int position, View convertView, @NonNull ViewGroup parent) {
            View row = convertView;
            RecordHolder holder = null;

            if (row == null) {
                LayoutInflater inflater = ((Activity) mContext).getLayoutInflater();
                row = inflater.inflate(mLayoutResourceId, parent, false);

                holder = new RecordHolder();
                holder.title = (TextView) row.findViewById(R.id.item_text);
                holder.imageItem = (ImageView) row.findViewById(R.id.item_image);
                row.setTag(holder);
            } else {
                holder = (RecordHolder) row.getTag();
            }

            MainIcon item = mMainIconList.get(position);
            holder.title.setText(item.getTitle());
            holder.imageItem.setImageBitmap(item.getIconBitmap());
            return row;
        }

        class RecordHolder {
            TextView title;
            ImageView imageItem;
        }
    }
}