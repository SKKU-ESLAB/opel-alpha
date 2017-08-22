package com.opel.opel_manager.view;

import android.app.ActionBar;
import android.app.Activity;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.DisplayMetrics;
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
import com.opel.opel_manager.model.OPELApplication;

import java.io.File;
import java.io.FileInputStream;

public class RemoteNotiUIActivity extends Activity {
    private Context mContext;
    private LinearLayout mLayout;
    private String checkNoti;

    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_remote_noti_ui);

        Bundle extras = getIntent().getExtras();
        int id = extras.getInt("notificationId");

        String jsonString = extras.getString("jsonData");
        checkNoti = extras.getString("checkNoti");


        LegacyJSONParser jp = new LegacyJSONParser(jsonString);

        String appId = jp.getValueByKey("appID");

        OPELApplication targetApp = OPELContext.getAppList().getApp
                (appId);


        try {
            ActionBar actionBar = getActionBar();
            actionBar.setTitle(targetApp.getTitle());
            actionBar.setDisplayHomeAsUpEnabled(true);

            Drawable dr = new BitmapDrawable(getResources(), OPELContext
                    .getAppList().getApp(appId).getImage());
            actionBar.setIcon(dr);
            actionBar.setDisplayUseLogoEnabled(true);

        } catch (Exception e) {
            e.printStackTrace();
        }


        mContext = this;
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

                LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams
                        (LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
                lp.gravity = Gravity.CENTER;
                lp.setMargins(60, 20, 60, 20);

                tview.setLayoutParams(lp);
                mLayout.addView(tview);

            } else if (ret[0].equals("img")) {

                try {
                    ImageView iv = new ImageView(mContext);
                    FileInputStream is;


                    if (isNoti.equals("2")) {
                        is = new FileInputStream(new File(OPELContext
                                .getSettings().getCloudDir(), ret[1]));
                    } else {
                        is = new FileInputStream(new File(OPELContext
                                .getSettings().getRemoteUIDir(), ret[1]));

                    }
                    iv.setImageDrawable(Drawable.createFromStream(is, ret[1]));

                    LinearLayout.LayoutParams lp = new LinearLayout
                            .LayoutParams(LayoutParams.MATCH_PARENT,
                            LayoutParams.MATCH_PARENT);
                    lp.gravity = Gravity.CENTER;
                    lp.setMargins(0, 0, 0, 0);

                    DisplayMetrics metrics = new DisplayMetrics();
                    WindowManager windowManager = (WindowManager)
                            getApplicationContext().getSystemService(Context
                                    .WINDOW_SERVICE);
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

                LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams
                        (LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
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
                LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams
                        (LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
                lp.gravity = Gravity.CENTER;
                lp.setMargins(60, 20, 60, 20);
                tview.setLayoutParams(lp);
                mLayout.addView(tview);
            } else {
                //Do Nothing [appID, appName, etc]
            }

        }

        NotificationManager nm = (NotificationManager) getSystemService
                (Context.NOTIFICATION_SERVICE);
        nm.cancel(id);

    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                this.finish();

                if (checkNoti.equals("1")) {
                    Intent intent = new Intent(RemoteNotiUIActivity.this,
                            EventLoggerActivity.class);
                    startActivity(intent);
                }

                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    public void onBackPressed() {
        this.finish();
        if (checkNoti.equals("1") || checkNoti.equals("2")) {
            Intent intent = new Intent(RemoteNotiUIActivity.this,
                    EventLoggerActivity.class);
            startActivity(intent);
        }

    }
}