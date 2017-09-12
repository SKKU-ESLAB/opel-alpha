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

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.DownloadManager;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Window;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Toast;

import com.opel.opel_manager.controller.OPELControllerBroadcastReceiver;
import com.opel.opel_manager.controller.OPELControllerService;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;

import static android.content.ContentValues.TAG;

public class AppMarketActivity extends Activity {
    // OPELControllerService
    private OPELControllerService mControllerServiceStub = null;
    private PrivateControllerBroadcastReceiver mControllerBroadcastReceiver;
    private AppMarketActivity self = this;

    private WebView mWebView;

    DownloadManager mDownloadManager;

    private long mDownloadID = 0;

    ArrayList<String> requestQueue;

    public static ProgressDialog marketProgDialog;

    @SuppressLint("SetJavaScriptEnabled")
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(com.opel.opel_manager.R.layout.template_webview);

        mDownloadManager = (DownloadManager) getSystemService(Context.DOWNLOAD_SERVICE);

        this.mWebView = (WebView) findViewById(com.opel.opel_manager.R.id.webView1);
        this.mWebView.getSettings().setJavaScriptEnabled(true);

        String kMarketURI = "http://nyx.skku.ac.kr/temp_market/main.html";
        this.mWebView.loadUrl(kMarketURI);
        this.mWebView.setVerticalScrollBarEnabled(true);
        this.mWebView.setWebViewClient(new PrivateWebViewClient());

        requestQueue = new ArrayList<>();

        // Connect controller service
        this.connectControllerService();
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if ((keyCode == KeyEvent.KEYCODE_BACK) && this.mWebView.canGoBack()) {
            this.mWebView.goBack();
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    private class PrivateWebViewClient extends WebViewClient {
        public boolean shouldOverrideUrlLoading(WebView view, String url) {
            Log.d("OPEL", url);
            //STANDALONE APP Type
            if (url.endsWith("opk")) {
                marketProgDialog = new ProgressDialog(AppMarketActivity.this);
                marketProgDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
                marketProgDialog.setMessage("please wait....");
                marketProgDialog.setCancelable(false);
                marketProgDialog.show();

                // downloadFile(url); other way to download opk (sync)
                //Filename : current date and time
                SimpleDateFormat simpleDateFormat = new SimpleDateFormat("yyyy_MM_dd_HH_mm_ss",
                        Locale.KOREA);
                String fileName = simpleDateFormat.format(new Date());

                while (new File(Environment.DIRECTORY_DOWNLOADS, fileName).exists()) {
                    fileName += "_";
                }
                fileName += ".opk";

                Uri source = Uri.parse(url);
                DownloadManager.Request request = new DownloadManager.Request(source);
                request.setDescription("Description for the DownloadManager Bar");
                request.setTitle(fileName);

                request.allowScanningByMediaScanner();
                request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_HIDDEN);

                request.setDestinationInExternalPublicDir(Environment.DIRECTORY_DOWNLOADS,
                        fileName);

                // get download service and enqueue file
                mDownloadID = mDownloadManager.enqueue(request);
                if (requestQueue.isEmpty()) {
                    IntentFilter intentFilter = new IntentFilter(DownloadManager
                            .ACTION_DOWNLOAD_COMPLETE);
                    registerReceiver(mDownloadBroadcastReceiver, intentFilter);
                }
                requestQueue.add(fileName);

            } else {
                view.loadUrl(url);
            }
            return true;
        }
    }

    private final BroadcastReceiver mDownloadBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context arg0, Intent arg1) {
            DownloadManager.Query query = new DownloadManager.Query();

            query.setFilterById(mDownloadID);
            Cursor cursor = mDownloadManager.query(query);

            if (cursor.moveToFirst()) {
                int columnIndex = cursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
                int status = cursor.getInt(columnIndex);

                //send packageFile to OPEL
                if (status == DownloadManager.STATUS_SUCCESSFUL) {
                    try {
                        // handling file
                        String fileName = cursor.getString(cursor.getColumnIndex(DownloadManager
                                .COLUMN_TITLE));
                        requestQueue.remove(fileName);

                        Log.d("OPEL", fileName + " :: DOWNLOAD COMPLETE");
                        Toast.makeText(self, "Start to install...", Toast.LENGTH_LONG);
                        new InstallThread(fileName).start();
                        if (requestQueue.isEmpty()) unregisterReceiver(mDownloadBroadcastReceiver);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    };

    class InstallThread extends Thread {
        public String mFileName;

        InstallThread(String fileName) {
            mFileName = fileName;
        }

        public void run() {
            mControllerServiceStub.installAppOneWay(mFileName);
            self.finish();
        }

        public void onAppStateChanged(int appId, int appState) {

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

            // Set BroadcastReceiver
            IntentFilter broadcastIntentFilter = new IntentFilter();
            broadcastIntentFilter.addAction(OPELControllerBroadcastReceiver.ACTION);
            mControllerBroadcastReceiver = new PrivateControllerBroadcastReceiver();
            registerReceiver(mControllerBroadcastReceiver, broadcastIntentFilter);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.d(TAG, "onServiceDisconnected()");
            mControllerServiceStub = null;
        }
    };

    class PrivateControllerBroadcastReceiver extends OPELControllerBroadcastReceiver {
        PrivateControllerBroadcastReceiver() {
            // TODO: this.setOnResultInstallApp()
        }
    }
}
