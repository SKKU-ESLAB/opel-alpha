package com.example.opel_manager;

import android.app.Activity;
import android.app.DownloadManager;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Window;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Toast;

import java.io.File;
import java.io.FileInputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;

public class appMarketView extends Activity{

	private WebView mWebView;

	DownloadManager manager;
	File destinationDir;

	long downloadID=0;

	ArrayList<String> requestQueue;

	static ProgressDialog marketProgDialog;
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		setContentView(R.layout.webview_activity);
		/*
		File external_storage = Environment.getExternalStorageDirectory();
		File opel_dir = new File(external_storage, "OPEL");
		opel_dir.mkdir();
		
		appPackagePath = opel_dir.getAbsolutePath();
		*/

		//appPackagePath = globalData.getInstance().getStoragePath();

		manager = (DownloadManager) getSystemService(Context.DOWNLOAD_SERVICE);

		mWebView = (WebView) findViewById(R.id.webView1);
		mWebView.getSettings().setJavaScriptEnabled(true);
		mWebView.loadUrl("http://nyx.skku.ac.kr/temp_market/main.html");
		mWebView.setVerticalScrollBarEnabled(true);
		mWebView.setWebViewClient(new webViewClient());

		requestQueue = new ArrayList<String>();
		//apkInstall(new File("/storage/emulated/0/Download/abcd.apk"));

		appManagerView.updateDisplay();
	}

	private class webViewClient extends WebViewClient{
		public boolean shouldOverrideUrlLoading(WebView view, String url){

			Log.d("OPEL",url);
			//STANDALONE APP Type
			if(url.endsWith("opk") && globalData.getInstance().getCommManager().isConnect() ){

				marketProgDialog = new ProgressDialog( appMarketView.this );
				marketProgDialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
				marketProgDialog.setMessage("please wait....");
				marketProgDialog.setCancelable(false);
				marketProgDialog.show();

				// downloadFile(url); other way to download opk (sync)
				//Filename : current date and time
				SimpleDateFormat mSimpleDateFormat = new SimpleDateFormat ( "yyyy_MM_dd_HH_mm_ss", Locale.KOREA );
				Date currentTime = new Date();
				String fileName = mSimpleDateFormat.format ( currentTime);

				while(new File(Environment.DIRECTORY_DOWNLOADS, fileName).exists()){
					fileName += "_";
				}

				fileName += ".opk";

				Uri source = Uri.parse(url);
				DownloadManager.Request request = new DownloadManager.Request(source);
				request.setDescription("Description for the DownloadManager Bar");

				request.setTitle(fileName);

				if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
					request.allowScanningByMediaScanner();
					request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_HIDDEN);
				}


				request.setDestinationInExternalPublicDir(Environment.DIRECTORY_DOWNLOADS, fileName);
				// save the file in the "Downloads" folder of SDCARD
				//request.setDestinationInExternalPublicDir(appPackagePath, fileName);
				//request.setDestinationInExternalFilesDir(getApplicationContext(), globalData.getInstance().getStoragePath(), fileName);

				// get download service and enqueue file
				downloadID = manager.enqueue(request);
				if(requestQueue.isEmpty()){
					IntentFilter intentFilter = new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE);
					registerReceiver(downloadReceiver, intentFilter);
				}
				requestQueue.add(fileName);

			}
			else if(url.endsWith("opk") && !globalData.getInstance().getCommManager().isConnect() ){
				Toast.makeText(getApplicationContext(),  "Need to connect with OPEL" , 0).show();
			}
			else{
				view.loadUrl(url);
			}
			return true;
		}
	}

	protected void onResume(){
		super.onResume();
		//IntentFilter intentFilter = new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE);
		registerReceiver(globalData.getInstance().getWifiReceiver(), globalData.getInstance().getIntentFilter());
	}


	protected void onPause(){
		super.onPause();
		//if(requestQueue.isEmpty())
		unregisterReceiver(globalData.getInstance().getWifiReceiver());
	}




	private final BroadcastReceiver downloadReceiver = new BroadcastReceiver() {
		public void onReceive(Context arg0, Intent arg1) {

			DownloadManager.Query query = new DownloadManager.Query();

			query.setFilterById(downloadID);
			Cursor cursor = manager.query(query);


			if(cursor.moveToFirst()){
				int columnIndex = cursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
				int status = cursor.getInt(columnIndex);

				//send packageFile to OPEL
				if(status == DownloadManager.STATUS_SUCCESSFUL){

					try{
						ParcelFileDescriptor file = manager.openDownloadedFile(downloadID);

						FileInputStream InputStream = new ParcelFileDescriptor.AutoCloseInputStream(file);

						// handling file
						String fileName = cursor.getString(cursor.getColumnIndex(DownloadManager.COLUMN_TITLE));
						requestQueue.remove(fileName);

						Log.d("OPEL", fileName + " :: DOWNLOAD COMPLETE");

						class thth extends Thread{
							public String filename;
							thth(String fname){
								filename = fname;
							}

							public void run(){
								globalData.getInstance().getCommManager().requestInstall(filename);
							}
						}

						new thth(fileName).start();





						if(requestQueue.isEmpty())
							unregisterReceiver(downloadReceiver);

					}
					catch(Exception e){
						e.printStackTrace();
					}
				}
			}
		}
	};



	public boolean onKeyDown(int keyCode, KeyEvent event){
		if((keyCode == KeyEvent.KEYCODE_BACK) && mWebView.canGoBack()){
			mWebView.goBack();
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}

	//For Companion type//
	public void apkInstall(File apkfile){
		//Call this function
		//File apkfile = new File(Environment.getExternalStorageDirectory().getAbsolutePath()+"/(占쏙옙占쏙옙占싱몌옙).apk");
		//apkInstall(apkfile);

		Uri apkUri = Uri.fromFile(apkfile);
		try {
			Intent packageinstaller = new Intent(Intent.ACTION_VIEW);
			packageinstaller.setDataAndType( apkUri, "application/vnd.android.package-archive");
			startActivity(packageinstaller);
		} catch (Exception e) {
			Log.d("OPEL", e.getMessage());
		}
	}


}
