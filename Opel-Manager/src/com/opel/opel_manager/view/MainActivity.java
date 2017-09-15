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
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.IBinder;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
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
import com.opel.opel_manager.controller.OPELControllerBroadcastReceiver;
import com.opel.opel_manager.controller.OPELControllerService;
import com.opel.opel_manager.model.OPELApp;
import com.opel.opel_manager.view.main.AppManagerMainIcon;
import com.opel.opel_manager.view.main.AppMarketMainIcon;
import com.opel.opel_manager.view.main.CameraViewerMainIcon;
import com.opel.opel_manager.view.main.ConnectMainIcon;
import com.opel.opel_manager.view.main.EventLogViewerMainIcon;
import com.opel.opel_manager.view.main.FileManagerMainIcon;
import com.opel.opel_manager.view.main.MainIcon;
import com.opel.opel_manager.view.main.SensorViewerMainIcon;
import com.opel.opel_manager.view.main.UserAppMainIcon;
import com.opel.opel_manager.view.remoteui.RemoteConfigUIActivity;

import java.util.ArrayList;

public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";

    // OPELControllerService
    private OPELControllerService mControllerServiceStub;
    private PrivateControllerBroadcastReceiver mControllerBroadcastReceiver;
    private MainActivity self = this;

    private PortIndicator mDefaultPortIndicator;
    private PortIndicator mLargeDataPortIndicator;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Initialize UI Layout
        setContentView(R.layout.activity_main);

        // Check Permission
        this.checkStoragePermission();

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
        this.disconnectControllerService();
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
        GridView iconGridView = (GridView) findViewById(R.id.iconGridView);
        this.mMainIconListAdapter = new MainIconListAdapter(this, R.layout
                .template_gridview_icon_main, this.mMainIconList);
        iconGridView.setAdapter(mMainIconListAdapter);
        iconGridView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int position, long id) {
                MainIcon thisIcon = (MainIcon) adapterView.getItemAtPosition(position);
                thisIcon.onClick();
            }
        });
        iconGridView.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> adapterView, View view, int position,
                                           long id) {
                MainIcon thisIcon = (MainIcon) adapterView.getItemAtPosition(position);
                thisIcon.onLongClick();
                return true;
            }
        });
        this.mDefaultPortIndicator = new PortIndicator(R.id.indicatorBT, R.drawable
                .bluetooth_disabled, R.drawable.bluetooth);
        this.mLargeDataPortIndicator = new PortIndicator(R.id.indicatorWFD, R.drawable
                .wifidirect_disabled, R.drawable.wifidirect);

        // Update main icon list without any applications
        this.updateMainIconList(null);
    }

    private void initializeControllerService() {
        // Launch OPELControllerService for setting connection with target OPEL device.
        Intent serviceIntent = new Intent(this, OPELControllerService.class);
        this.bindService(serviceIntent, this.mControllerServiceConnection, Context
                .BIND_AUTO_CREATE);
    }

    private void disconnectControllerService() {
        if (this.mControllerServiceConnection != null)
            this.unbindService(this.mControllerServiceConnection);
        if(this.mControllerBroadcastReceiver != null)
            this.unregisterReceiver(this.mControllerBroadcastReceiver);
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

            // Request to open connection
            connectTargetDevice();
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.d(TAG, "onServiceDisconnected()");
            unregisterReceiver(mControllerBroadcastReceiver);
            mControllerServiceStub = null;
        }
    };

    private class PortIndicator {
        private static final int STATE_DISCONNECTED = 0;
        private static final int STATE_CONNECTING = 1;
        private static final int STATE_CONNECTED = 2;

        private ImageView mIndicatorImageView;
        private ConnectingAnimationThread mConnectingAnimationThread;
        private int mState;
        private int mDisconnectedResourceId;
        private int mConnectedResourceId;

        PortIndicator(int indicatorImageViewId, int disconnectedResourceId, int
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

        void setDisconnected() {
            this.mState = STATE_DISCONNECTED;
            this.mConnectingAnimationThread.disable();
            this.mIndicatorImageView.setImageResource(mDisconnectedResourceId);
        }

        void setConnecting() {
            this.mState = STATE_CONNECTING;
            this.mConnectingAnimationThread.start();
        }

        void setConnected() {
            this.mState = STATE_CONNECTED;
            this.mConnectingAnimationThread.disable();
            mIndicatorImageView.setImageResource(mConnectedResourceId);
        }

        class ConnectingAnimationThread extends Thread {
            private static final int SLEEP_MILLISECS = 300;
            private boolean mIsEnabled = false;

            void disable() {
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
                    } catch (InterruptedException ignored) {
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

    private final int kNotificationId = 1234;

    // Control UI
    public void updateUI() {
        mMainIconListAdapter.updateUI();
    }

    public void showRemoteConfigUI(int appId) {
        OPELApp thisApp = this.getApp(appId);
        if (thisApp.getConfigJSONString().compareTo("") == 0) {
            Toast.makeText(getApplicationContext(), "This app" + thisApp.getName() + " does not "
                    + "have available configurations.", Toast.LENGTH_SHORT).show();
        } else {
            Intent intent = new Intent(MainActivity.this, RemoteConfigUIActivity.class);

            Bundle extras = new Bundle();
            extras.putString("title", thisApp.getName());
            extras.putString("appID", "" + thisApp.getAppId());
            extras.putString("jsonData", thisApp.getConfigJSONString());

            intent.putExtras(extras);
            startActivity(intent);
        }
    }

    // Control connection
    public boolean isTargetDeviceConnected() {
        if (this.mControllerServiceStub == null) return false;

        int commChannelState = this.mControllerServiceStub.getCommChannelState();
        return (commChannelState != CommChannelService.STATE_DISCONNECTED) && (commChannelState
                != CommChannelService.STATE_CONNECTING_DEFAULT);
    }

    public void connectTargetDevice() {
        if (isTargetDeviceConnected()) {
            Toast.makeText(getApplicationContext(), "Already connected to target device", Toast
                    .LENGTH_SHORT).show();
            return;
        } else if (this.mControllerServiceStub == null) {
            Toast.makeText(getApplicationContext(), "Controller service is not initialized",
                    Toast.LENGTH_SHORT).show();
            return;
        }

        // Request to open connection
        this.mControllerServiceStub.initializeConnectionAsync();
    }

    // Control an app
    public OPELApp getApp(int appId) {
        if (this.mControllerServiceStub == null) {
            Log.e(TAG, "ControllerService is not connected");
            return null;
        }
        return this.mControllerServiceStub.getApp(appId);
    }

    public void launchApp(int appId) {
        if (this.mControllerServiceStub == null) {
            Log.e(TAG, "ControllerService is not connected");
            return;
        }
        this.mControllerServiceStub.launchAppOneWay(appId);
    }

    public void terminateApp(int appId) {
        if (this.mControllerServiceStub == null) {
            Log.e(TAG, "ControllerService is not connected");
            return;
        }
        this.mControllerServiceStub.terminateAppOneWay(appId);
    }

    class PrivateControllerBroadcastReceiver extends OPELControllerBroadcastReceiver {
        PrivateControllerBroadcastReceiver() {
            this.setOnCommChannelStateChangedListener(new OnCommChannelStateChangedListener() {
                @Override
                public void onCommChannelStateChanged(int prevState, int newState) {
                    // When connection state with target device is changed, indicator UI should
                    // be changed
                    Log.d(TAG, "CommChannel State change: " + prevState + " -> " + newState);
                    switch (newState) {
                        case CommChannelService.STATE_DISCONNECTED:
                            mDefaultPortIndicator.setDisconnected();
                            mLargeDataPortIndicator.setDisconnected();

                            switch (prevState) {
                                case CommChannelService.STATE_CONNECTING_DEFAULT:
                                    Toast.makeText(getApplicationContext(), "Failed connecting "
                                            + "to target device. Retry it.", Toast.LENGTH_LONG)
                                            .show();

                                    break;
                                case CommChannelService.STATE_CONNECTED_DEFAULT:
                                case CommChannelService.STATE_CONNECTING_LARGE_DATA:
                                case CommChannelService.STATE_CONNECTED_LARGE_DATA:
                                    Toast.makeText(getApplicationContext(), "OPEL device is " +
                                            "disconnected" + ".", Toast.LENGTH_LONG).show();
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
                                    Toast.makeText(getApplicationContext(), "OPEL device is " +
                                            "connected.", Toast.LENGTH_LONG).show();
                                    mControllerServiceStub.updateAppListAsync();
                                    break;
                                case CommChannelService.STATE_CONNECTING_LARGE_DATA:
                                    Toast.makeText(getApplicationContext(), "Opening large data "
                                            + "port is failed.", Toast.LENGTH_LONG).show();
                                    break;
                                case CommChannelService.STATE_CONNECTED_LARGE_DATA:
                                    Toast.makeText(getApplicationContext(), "Large data port is "
                                            + "closed.", Toast.LENGTH_LONG).show();
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

                            Toast.makeText(getApplicationContext(), "Large data port is opened.",
                                    Toast.LENGTH_LONG).show();
                            break;
                    }
                }
            });

            this.setOnAppStateChangedListener(new OnAppStateChangedListener() {
                @Override
                public void onAppStateChanged(int appId, int appState) {
                    boolean appFound = false;
                    for (MainIcon icon : mMainIconList) {
                        if (icon instanceof UserAppMainIcon) {
                            UserAppMainIcon userAppIcon = (UserAppMainIcon) icon;
                            if (userAppIcon.getAppId() == appId) {
                                appFound = true;

                                // Notify app change of state
                                userAppIcon.updateAppState(appState);

                                // If app is removed, remove main icon of the app
                                if (appState == OPELApp.State_Removed) {
                                    removeUserAppMainIcon(appId);
                                }
                            }
                        }
                    }
                    if (!appFound) {
                        // If main icon does not exist, make new main icon for the app
                        OPELApp newApp = mControllerServiceStub.getApp(appId);
                        addUserAppMainIcon(newApp);
                    }
                }
            });

            this.setOnResultUpdateAppListListener(new OnResultUpdateAppListListener() {
                @Override
                public void onResultUpdateAppList(OPELApp[] appList) {
                    // Add MainIcon for the apps
                    updateMainIconList(appList);
                }
            });
        }
    }

    private MainIconListAdapter mMainIconListAdapter;
    private ArrayList<MainIcon> mMainIconList = new ArrayList<>();

    private class MainIconListAdapter extends ArrayAdapter<MainIcon> {
        Context mContext;
        int mLayoutResourceId;

        MainIconListAdapter(Context context, int layoutResourceId, ArrayList<MainIcon>
                mainIconList) {
            super(context, layoutResourceId, mainIconList);
            this.mLayoutResourceId = layoutResourceId;
            this.mContext = context;
        }

        void updateUI() {
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

    private void addUserAppMainIcon(OPELApp app) {
        this.mMainIconList.add(new UserAppMainIcon(this, app.getAppId(), app.getName(), app
                .getIconImagePath(), app.getState()));
        updateUI();
    }

    private void removeUserAppMainIcon(int appId) {
        for (MainIcon icon : this.mMainIconList) {
            if (icon instanceof UserAppMainIcon) {
                UserAppMainIcon userAppIcon = (UserAppMainIcon) icon;
                if (userAppIcon.getAppId() == appId) {
                    this.mMainIconList.remove(icon);
                    updateUI();
                }
            }
        }
    }

    private void updateMainIconList(OPELApp[] appList) {
        this.mMainIconList.clear();
        this.mMainIconList.add(new ConnectMainIcon(this));
        this.mMainIconList.add(new AppManagerMainIcon(this));
        this.mMainIconList.add(new AppMarketMainIcon(this));
        this.mMainIconList.add(new EventLogViewerMainIcon(this));
        this.mMainIconList.add(new FileManagerMainIcon(this));

        if (appList != null) {
            for (OPELApp app : appList) {
                if (app.isDefaultApp()) {
                    // Default app
                    if (app.getName().compareTo("CameraViewer") == 0) {
                        this.mMainIconList.add(new CameraViewerMainIcon(this, app.getAppId()));
                    } else if (app.getName().compareTo("SensorViewer") == 0) {
                        this.mMainIconList.add(new SensorViewerMainIcon(this, app.getAppId()));
                    }
                } else {
                    // User app
                    this.mMainIconList.add(new UserAppMainIcon(this, app.getAppId(), app.getName
                            (), app.getIconImagePath(), app.getState()));
                }
            }
        }
        updateUI();
    }
}