package com.opel.opel_manager.view;

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
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Window;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Toast;

import com.opel.opel_manager.controller.OPELControllerService;

import java.io.File;
import java.io.FileInputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;

import static android.content.ContentValues.TAG;

public class AppMarketActivity extends Activity {
    // OPELControllerService
    private OPELControllerService mControllerServiceStub = null;
    private AppMarketActivity self = this;

    private WebView mWebView;

    DownloadManager manager;
    File destinationDir;

    long downloadID = 0;

    ArrayList<String> requestQueue;

    public static ProgressDialog marketProgDialog;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(com.opel.opel_manager.R.layout.template_webview);

        manager = (DownloadManager) getSystemService(Context.DOWNLOAD_SERVICE);

        mWebView = (WebView) findViewById(com.opel.opel_manager.R.id.webView1);
        mWebView.getSettings().setJavaScriptEnabled(true);
        mWebView.loadUrl("http://nyx.skku.ac.kr/temp_market/main.html");
        mWebView.setVerticalScrollBarEnabled(true);
        mWebView.setWebViewClient(new PrivateWebViewClient());

        requestQueue = new ArrayList<String>();

        // Connect controller service
        this.connectControllerService();
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if ((keyCode == KeyEvent.KEYCODE_BACK) && mWebView.canGoBack()) {
            mWebView.goBack();
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
                SimpleDateFormat mSimpleDateFormat = new SimpleDateFormat("yyyy_MM_dd_HH_mm_ss",
                        Locale.KOREA);
                Date currentTime = new Date();
                String fileName = mSimpleDateFormat.format(currentTime);

                while (new File(Environment.DIRECTORY_DOWNLOADS, fileName).exists()) {
                    fileName += "_";
                }

                fileName += ".opk";

                Uri source = Uri.parse(url);
                DownloadManager.Request request = new DownloadManager.Request(source);
                request.setDescription("Description for the DownloadManager " + "Bar");
                request.setTitle(fileName);

                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                    request.allowScanningByMediaScanner();
                    request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_HIDDEN);
                }

                request.setDestinationInExternalPublicDir(Environment.DIRECTORY_DOWNLOADS,
                        fileName);

                // get download service and enqueue file
                downloadID = manager.enqueue(request);
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

            query.setFilterById(downloadID);
            Cursor cursor = manager.query(query);

            if (cursor.moveToFirst()) {
                int columnIndex = cursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
                int status = cursor.getInt(columnIndex);

                //send packageFile to OPEL
                if (status == DownloadManager.STATUS_SUCCESSFUL) {
                    try {
                        ParcelFileDescriptor file = manager.openDownloadedFile(downloadID);
                        FileInputStream InputStream = new ParcelFileDescriptor
                                .AutoCloseInputStream(file);

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
    }

    //For Companion type//
    public void apkInstall(File apkFile) {
        // TODO: it is not used now, but to be used in future
        Uri apkUri = Uri.fromFile(apkFile);
        try {
            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.setDataAndType(apkUri, "OPELApp/vnd.android.package-archive");
            startActivity(intent);
        } catch (Exception e) {
            Log.d("OPEL", e.getMessage());
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
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.d(TAG, "onServiceDisconnected()");
            mControllerServiceStub = null;
        }
    };
}
