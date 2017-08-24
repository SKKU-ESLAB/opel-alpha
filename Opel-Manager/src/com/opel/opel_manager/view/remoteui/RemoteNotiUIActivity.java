package com.opel.opel_manager.view.remoteui;

import android.app.ActionBar;
import android.app.Activity;
import android.app.NotificationManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.IBinder;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.MenuItem;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.TextView;

import com.opel.opel_manager.R;
import com.opel.opel_manager.controller.LegacyJSONParser;
import com.opel.opel_manager.controller.OPELContext;
import com.opel.opel_manager.controller.OPELControllerService;
import com.opel.opel_manager.model.OPELApp;
import com.opel.opel_manager.view.EventLogViewerActivity;

import java.io.File;
import java.io.FileInputStream;

import static android.content.ContentValues.TAG;

public class RemoteNotiUIActivity extends Activity {
    // OPELControllerService
    private OPELControllerService mControllerServiceStub = null;

    private LinearLayout mLayout;
    private String mIsCheckNoti;
    private int mAppId;

    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_remote_noti_ui);

        // Parameters
        Bundle extras = getIntent().getExtras();
        int id = extras.getInt("notificationId");
        String jsonString = extras.getString("jsonData");
        mIsCheckNoti = extras.getString("mIsCheckNoti");

        LegacyJSONParser jp = new LegacyJSONParser(jsonString);
        String appId = jp.getValueByKey("appID");
        this.mAppId = Integer.parseInt(appId);

        mLayout = (LinearLayout) findViewById(R.id.dynamicLayout);

        String isNoti = jp.getValueByKey("isNoti");
        while (jp.hasMoreValue()) {
            String ret[] = new String[2];
            ret = jp.getNextKeyValue();

            if (ret[0].equals("text")) {
                TextView tview = new TextView(this);
                tview.setText(ret[1]);
                tview.setTextSize(20);
                tview.setTextColor(Color.WHITE);
                tview.setGravity(Gravity.CENTER);

                LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(LayoutParams
                        .WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
                lp.gravity = Gravity.CENTER;
                lp.setMargins(60, 20, 60, 20);

                tview.setLayoutParams(lp);
                mLayout.addView(tview);

            } else if (ret[0].equals("img")) {

                try {
                    ImageView iv = new ImageView(this);
                    FileInputStream is;


                    if (isNoti.equals("2")) {
                        is = new FileInputStream(new File(OPELContext.getSettings().getCloudDir()
                                , ret[1]));
                    } else {
                        is = new FileInputStream(new File(OPELContext.getSettings()
                                .getRemoteUIDir(), ret[1]));

                    }
                    iv.setImageDrawable(Drawable.createFromStream(is, ret[1]));

                    LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(LayoutParams
                            .MATCH_PARENT, LayoutParams.MATCH_PARENT);
                    lp.gravity = Gravity.CENTER;
                    lp.setMargins(0, 0, 0, 0);

                    DisplayMetrics metrics = new DisplayMetrics();
                    WindowManager windowManager = (WindowManager) getApplicationContext()
                            .getSystemService(Context.WINDOW_SERVICE);
                    windowManager.getDefaultDisplay().getMetrics(metrics);
                    lp.width = metrics.widthPixels;
                    lp.height = metrics.heightPixels;

                    iv.setLayoutParams(lp);
                    mLayout.addView(iv);

                    is.close();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            } else if (ret[0].equals("dateTime")) {
                TextView tview = new TextView(this);
                tview.setText(ret[1]);
                tview.setTextSize(20);
                tview.setTextColor(Color.RED);
                tview.setGravity(Gravity.CENTER);

                LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(LayoutParams
                        .WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
                lp.gravity = Gravity.CENTER;
                lp.setMargins(60, 20, 60, 20);
                tview.setLayoutParams(lp);
                mLayout.addView(tview);
            } else if (ret[0].equals("description")) {
                TextView tview = new TextView(this);
                tview.setText(ret[1]);
                tview.setTextSize(20);
                tview.setTextColor(Color.RED);
                tview.setGravity(Gravity.CENTER);
                LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(LayoutParams
                        .WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
                lp.gravity = Gravity.CENTER;
                lp.setMargins(60, 20, 60, 20);
                tview.setLayoutParams(lp);
                mLayout.addView(tview);
            } else {
                //Do Nothing [appID, appName, etc]
            }

        }

        NotificationManager nm = (NotificationManager) getSystemService(Context
                .NOTIFICATION_SERVICE);
        nm.cancel(id);

    }

    private void initializeActionBar() {
        if (mControllerServiceStub == null) {
            Log.e(TAG, "Controller is not yet connected");
            return;
        }
        OPELApp targetApp = mControllerServiceStub.getApp(this.mAppId);

        try {
            ActionBar actionBar = this.getActionBar();
            actionBar.setTitle(targetApp.getName());
            actionBar.setDisplayHomeAsUpEnabled(true);

            Drawable dr = Drawable.createFromPath(targetApp.getIconImagePath());
            actionBar.setIcon(dr);
            actionBar.setDisplayUseLogoEnabled(true);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                this.finish();
                if (mIsCheckNoti.equals("1")) {
                    Intent intent = new Intent(RemoteNotiUIActivity.this, EventLogViewerActivity
                            .class);
                    startActivity(intent);
                }
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    private void connectControllerService() {
        Intent serviceIntent = new Intent(this, EventLogViewerActivity.class);
        this.bindService(serviceIntent, this.mControllerServiceConnection, Context
                .BIND_AUTO_CREATE);
    }

    private ServiceConnection mControllerServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder inputBinder) {
            OPELControllerService.ControllerBinder serviceBinder = (OPELControllerService
                    .ControllerBinder) inputBinder;
            mControllerServiceStub = serviceBinder.getService();


            // Update UI
            initializeActionBar();
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.d(TAG, "onServiceDisconnected()");
            mControllerServiceStub = null;
        }
    };
}