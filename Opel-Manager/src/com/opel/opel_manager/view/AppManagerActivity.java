package com.opel.opel_manager.view;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.opel.opel_manager.R;
import com.opel.opel_manager.controller.OPELControllerBroadcastReceiver;
import com.opel.opel_manager.controller.OPELControllerService;
import com.opel.opel_manager.model.OPELApp;

import java.util.ArrayList;

import static android.content.ContentValues.TAG;

public class AppManagerActivity extends Activity {
    // OPELControllerService
    private OPELControllerService mControllerServiceStub = null;
    private AppManagerActivity self = this;

    private ListView mAppListView;
    private AppListAdapter mAppListAdapter;
    private ArrayList<AppListItem> mAppList = new ArrayList<AppListItem>();

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(com.opel.opel_manager.R.layout.template_listview);

        // Initialize UI
        this.initializeUI();

        // Connect controller service
        this.connectControllerService();
    }

    private void initializeUI() {
        ActionBar actionBar = getActionBar();
        actionBar.setTitle("App Manager");
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setLogo(com.opel.opel_manager.R.drawable.icon_app_manager);
        actionBar.setDisplayUseLogoEnabled(true);

        this.mAppListView = (ListView) findViewById(com.opel.opel_manager.R.id.listView1);
        this.mAppListAdapter = new AppListAdapter();
        this.mAppListView.setAdapter(mAppListAdapter);
        this.mAppListView.setOnItemClickListener(mItemClickListener);
    }

    protected void onResume() {
        super.onResume();
        mAppListAdapter.updateUI();
    }

    private AdapterView.OnItemClickListener mItemClickListener = new AdapterView
            .OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long l_position) {
            final AppListItem item = (AppListItem) parent.getAdapter().getItem(position);

            AlertDialog.Builder alt_bld = new AlertDialog.Builder(AppManagerActivity.this);
            alt_bld.setMessage("Delete this app ?").setCancelable(false).setPositiveButton("Yes",
                    new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int id) {
                    // Action for 'Yes' Button
                    int appId = item.getAppID();
                    if (mControllerServiceStub != null)
                        mControllerServiceStub.removeAppOneWay(appId);
                }
            }).setNegativeButton("No", new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int id) {
                    // Action for 'NO' Button
                    dialog.cancel();
                }
            });

            AlertDialog alert = alt_bld.create();
            alert.setTitle(item.getAppName());
            Drawable d = new BitmapDrawable(getResources(), item.getAppIcon());
            alert.setIcon(d);
            alert.show();
        }

    };

    public void updateUI() {
        if (this.mAppListAdapter != null) {
            this.mAppListAdapter.updateUI();
        }
    }

    class AppListAdapter extends BaseAdapter {
        private int pos;

        @Override
        public int getCount() {
            return mAppList.size();
        }

        @Override
        public Object getItem(int position) {
            return mAppList.get(position);
        }

        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                int res = 0;
                res = com.opel.opel_manager.R.layout.template_listview_item_icon;
                LayoutInflater mInflater = (LayoutInflater) self.getSystemService(Context
                        .LAYOUT_INFLATER_SERVICE);

                convertView = mInflater.inflate(res, parent, false);
            }
            pos = position;
            if (mAppList.size() != 0) {
                TextView ci_nickname_text = (TextView) convertView.findViewById(R.id.tv_title);
                ci_nickname_text.setText(mAppList.get(pos).getAppName());
                TextView ci_content_text = (TextView) convertView.findViewById(R.id.tv_subTitle);


                String state = "";
                AppListItem appListItem = mAppList.get(pos);
                int appState = appListItem.getAppState();
                switch (appState) {
                    case OPELApp.State_Initialized:
                    case OPELApp.State_Initializing:
                    case OPELApp.State_Installing:
                        ci_content_text.setTextColor(Color.GRAY);
                        break;
                    case OPELApp.State_Ready:
                        ci_content_text.setTextColor(Color.WHITE);
                        break;
                    case OPELApp.State_Launching:
                        ci_content_text.setTextColor(Color.RED);
                        break;
                    case OPELApp.State_Running:
                        ci_content_text.setTextColor(Color.RED);
                        break;
                    case OPELApp.State_Removing:
                    case OPELApp.State_Removed:
                        ci_content_text.setTextColor(Color.GRAY);
                        break;
                    default:
                        // ignore
                        break;
                }
                ci_content_text.setText(state);

                ImageView iv = (ImageView) convertView.findViewById(R.id.imageView11);
                iv.setImageBitmap(mAppList.get(pos).getAppIcon());

            }
            return convertView;
        }

        public void updateUI() {
            this.notifyDataSetChanged();
        }
    }

    // get app information from global list and transfer to AppListItem format
    public void updateAppList() {
        if (this.mControllerServiceStub == null) return;
        ArrayList<OPELApp> appList = this.mControllerServiceStub.getAppList();

        this.mAppList.clear();
        for (OPELApp app : appList) {
            if (!app.isDefaultApp()) {
                int appId = app.getAppId();
                String appName = app.getName();
                Bitmap iconBitmap = BitmapFactory.decodeFile(app.getIconImagePath());
                int appState = app.getState();
                this.mAppList.add(new AppListItem(appId, appName, iconBitmap, appState));
            }
        }
    }

    class AppListItem {
        private int mAppID;
        private String mAppName;
        private Bitmap mAppIcon;
        private int mAppState;

        AppListItem(int appID, String appName, Bitmap appIcon, int appState) {
            this.mAppID = appID;
            this.mAppName = appName;
            this.mAppIcon = appIcon;
            this.mAppState = appState;
        }

        int getAppID() {
            return this.mAppID;
        }

        String getAppName() {
            return this.mAppName;
        }

        Bitmap getAppIcon() {
            return mAppIcon;
        }

        int getAppState() {
            return this.mAppState;
        }

        void updateAppState(int appState) {
            this.mAppState = appState;
            self.updateUI();
        }
    }

    private void connectControllerService() {
        Intent serviceIntent = new Intent(this, OPELControllerService.class);
        this.bindService(serviceIntent, this.mControllerServiceConnection, Context
                .BIND_AUTO_CREATE);
    }

    private ServiceConnection mControllerServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder inputBinder) {
            OPELControllerService.ControllerBinder serviceBinder = (OPELControllerService
                    .ControllerBinder) inputBinder;
            mControllerServiceStub = serviceBinder.getService();

            // Update app list
            updateAppList();
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.d(TAG, "onServiceDisconnected()");
            mControllerServiceStub = null;
        }
    };

    class PrivateControllerBroadcastReceiver extends OPELControllerBroadcastReceiver {
        PrivateControllerBroadcastReceiver() {
            this.setOnAppStateChangedListener(new OnAppStateChangedListener() {
                @Override
                public void onAppStateChanged(int appId, int appState) {
                    for (AppListItem appListItem : mAppList) {
                        if (appListItem.getAppID() == appId) {
                            if (appState == OPELApp.State_Removed) {
                                // App is removed
                                mAppList.remove(appListItem);
                            } else {
                                // App state is updated
                                appListItem.updateAppState(appState);
                            }
                        }
                    }
                }
            });
        }
    }
}