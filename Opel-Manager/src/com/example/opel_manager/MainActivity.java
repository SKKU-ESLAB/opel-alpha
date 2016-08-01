package com.example.opel_manager;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.wifi.p2p.WifiP2pManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.NotificationCompat;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.util.ArrayList;

import selectiveconnection.WifiDirectBroadcastReceiver;

public class MainActivity extends Activity {

	GridView gridView;
	static mainView _mainView;
	private boolean conn_stat = false;
	global_appList list; //Main Icon List
	static ProgressDialog mainLoadingProgDialog;
	WifiDirectBroadcastReceiver mReceiver = null;
	IntentFilter mIntentFilter;

	private Intent mqttServiceIntent;

	public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults){
		switch(requestCode){
			case 1:
				if(grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED){

				}
				else{
					Toast.makeText(this, "권한 획득 실패", Toast.LENGTH_SHORT);
					this.finish();
				}

		}
	}
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main_activity);

		if(checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED){
			Log.d("OPEL", "Requesting Permission");
			if(ActivityCompat.shouldShowRequestPermissionRationale(this, Manifest.permission.READ_EXTERNAL_STORAGE)) {
				Toast.makeText(this, "앱 실행을 위해서는 저장소 권한을 설정해야 합니다.", Toast.LENGTH_SHORT);
				ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
			}
			else{
				ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, 1);
			}
		}
		else{
			Log.d("OPEL", "Permission granted");
		}

		mIntentFilter = new IntentFilter();
		mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_STATE_CHANGED_ACTION);
		mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_PEERS_CHANGED_ACTION);
		mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_CONNECTION_CHANGED_ACTION);
		mIntentFilter.addAction(WifiP2pManager.WIFI_P2P_THIS_DEVICE_CHANGED_ACTION);

		initStorageWorkspace();

		list = globalData.getInstance().getAppList();

		if(!globalData.getInstance().isInit()){

			getNativeAppList(list);
			globalData.getInstance().getEventList().open(getApplicationContext());
			globalData.getInstance().initComplete();
			WifiP2pManager manager = (WifiP2pManager)getSystemService(WIFI_P2P_SERVICE);
			WifiP2pManager.Channel channel = manager.initialize(this, getMainLooper(), null);
			globalData.getInstance().setWifiP2pManager(manager);
			globalData.getInstance().setWifiChannel(channel);
			globalData.getInstance().setIntentFilter(mIntentFilter);


			// For MQTT Cloud service
			/*
			globalData.getInstance().getMqttManager().initMqttManager(getApplicationContext(), getIntent(), mHandler);
			registerReceiver (globalData.getInstance().getMqttManager().getDataReceiver(), globalData.getInstance().getMqttManager().getIntentFilter());

			mqttServiceIntent = new Intent(MainActivity.this, MqttBroadCastService.class);
			startService(mqttServiceIntent);

			globalData.getInstance().getMqttManager().initPublisher();
			*/
		}



     /*   ArrayList<event> Elist = globalData.getInstance().getEventList().get();

        for(int i=0; i < Elist.size(); i++){
        	Log.d("OPEL", "Item : " + Elist.get(i).getEvent());
        }*/



		//Create mainView
		gridView = (GridView) findViewById(R.id.gridView1);
		_mainView = new mainView(this, R.layout.main_icon, list.getAllApplicationList());
		gridView.setAdapter(_mainView);
		gridView.setOnItemClickListener(mItemClickListener);
		gridView.setOnItemLongClickListener(mItemLongClickListener);

		//Request sensor, camera hw setting info [MORE]
		globalData.getInstance().getCommManager().setOpelCommunicator();
		globalData.getInstance().setWifiReceiver(new WifiDirectBroadcastReceiver(globalData.getInstance().getWifiP2pManager(), globalData.getInstance().getChannel()));
		globalData.getInstance().getCommManager().setHandler(mHandler);
	}

	@Override
	public void onBackPressed() {
		// TODO Auto-generated method stub
		super.onBackPressed();
	}

	protected void onRestart() {
		super.onRestart();
		this.updateDisplayItem();
	}

	protected void onPause() {
		super.onPause();
		this.updateDisplayItem();
		unregisterReceiver(globalData.getInstance().getWifiReceiver());
	}

	protected void onResume() {
		super.onResume();
		this.updateDisplayItem();
		registerReceiver(globalData.getInstance().getWifiReceiver(), mIntentFilter);
	}
	protected void onDestroy() {
		globalData.getInstance().exitApp();
		super.onDestroy();

		/// For MQTT Cloud service
		/*
		stopService(mqttServiceIntent);
		unregisterReceiver(globalData.getInstance().getMqttManager().getDataReceiver());
		*/

	}
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	public static void updateDisplayItem(){
		_mainView.updateDisplay();
	}

	private GridView.OnItemClickListener mItemClickListener = new GridView.OnItemClickListener() {

		public void onItemClick(AdapterView<?> arg0, View v, int position, long arg3) {
			AlertDialog.Builder alertDlg = new AlertDialog.Builder(MainActivity.this);

			// Native Menu Handling
			if (list.allApplicationList.get(position).getType() == -1) {

				// [Native] Setting
				if ((list.allApplicationList.get(position).getTitle().equals("Setting"))) {
					if(conn_stat == false){
						Toast.makeText(getApplicationContext(), "Disconnected to OPEL",0).show();
					}

					Toast.makeText(getApplicationContext(),	list.allApplicationList.get(position).getTitle(), 0).show();
					Intent intent = new Intent(MainActivity.this,settingView.class);
					intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
					startActivity(intent);

				}

				// [Native] App Manager
				else if (list.allApplicationList.get(position).getTitle().equals("App Manager")) {
					if(conn_stat == false){
						Toast.makeText(getApplicationContext(), "Disconnected to OPEL",0).show();
					}
					Intent intent = new Intent(MainActivity.this, appManagerView.class);
					startActivity(intent);
				}

				// [Native] App Market
				else if (list.allApplicationList.get(position).getTitle().equals("App Market")) {
					if(conn_stat == false){
						Toast.makeText(getApplicationContext(), "Disconnected to OPEL",0).show();
					}
					Intent intent = new Intent(MainActivity.this,appMarketView.class);
					startActivity(intent);
				}

				// [Native] Connect
				else if (list.allApplicationList.get(position).getTitle().equals("Connect")) {
					if(conn_stat == false)
						globalData.getInstance().getCommManager().Connect();
					else
						Toast.makeText(getApplicationContext(), "Not disconnected",0).show();
				}

				// [Native] FileManager
				else if (list.allApplicationList.get(position).getTitle().equals("File Manager")) {
					if(conn_stat == false){
						Toast.makeText(getApplicationContext(), "Disconnected to OPEL",0).show();
					}


					Intent intent = new Intent(MainActivity.this, fileManagerView.class);
					startActivity(intent);

				}

				// [Native] Recording
				else if (list.allApplicationList.get(position).getTitle().equals("Camera")) {
					if(conn_stat == false){
						Toast.makeText(getApplicationContext(), "Disconnected to OPEL",0).show();
					}
					Intent intent = new Intent(MainActivity.this, cameraStreamingView.class);
					startActivity(intent);

				}
				
			/*	// [Native] Gallery
				else if (list.allApplicationList.get(position).getTitle().equals("Gallery")) {
					Intent intent = new Intent(MainActivity.this, galleryView.class);
					startActivity(intent);
				}*/

				// [Native] Sensor
				else if (list.allApplicationList.get(position).getTitle().equals("Sensor")) {
					if(conn_stat == false){
						Toast.makeText(getApplicationContext(), "Disconnected to OPEL",0).show();
					}
					Intent intent = new Intent(MainActivity.this, sensorView.class);
					startActivity(intent);
				}

				// [Native] Sensor
				else if (list.allApplicationList.get(position).getTitle().equals("Event Logger")) {

					Intent intent = new Intent(MainActivity.this, eventLoggerView.class);
					startActivity(intent);
				}
			}


			//Installed application
			else if (list.allApplicationList.get(position).getType() == 0) {		//Run application if it is not running

				String appID = list.allApplicationList.get(position).getAppId();
				String appName = list.allApplicationList.get(position).getTitle();
				globalData.getInstance().getCommManager().requestStart(appID, appName);


				// [MORE] SEND running msg to appManager
		
				
				/*list.allApplicationList.get(position).setTypeToRunning(); // Do this job at socket listener handling
				opelManagerGlobalData.getInstance().getmAllAppList().addRunningApplicationList(list.allApplicationList.get(position));*/


			}

			//Running application 
			else if (list.allApplicationList.get(position).getType() == 1) {		//Open configuration view if it is running

				Toast.makeText(getApplicationContext(), list.allApplicationList.get(position).title + " is OPEN", 0).show();

				if( list.allApplicationList.get(position).getConfigJson().equals("N/A")){
					Toast.makeText(getApplicationContext(), "Configurable data is N/A", 0).show();
				}

				else {
					Intent intent = new Intent(MainActivity.this, configurationView.class);

					Bundle extras = new Bundle();
					extras.putString("title",list.allApplicationList.get(position).getTitle());
					extras.putString("appID",list.allApplicationList.get(position).getAppId());
					extras.putString("jsonData", list.allApplicationList.get(position).getConfigJson());

					intent.putExtras(extras);
					startActivity(intent);
				}
			}

		}
	};

	private GridView.OnItemLongClickListener mItemLongClickListener = new GridView.OnItemLongClickListener() {

		public boolean onItemLongClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
			final int tmp = position;
			if (list.allApplicationList.get(position).getType() == -1) {
				Toast.makeText(getApplicationContext(),	"[native]"+list.allApplicationList.get(position).getTitle(), 0).show();
			}

			if (list.allApplicationList.get(position).getType() == 0) {
				Toast.makeText(getApplicationContext(),	"[installed]"+list.allApplicationList.get(position).getTitle(), 0).show();
			}

			if (list.allApplicationList.get(position).getType() == 1) {
				AlertDialog.Builder alt_bld = new AlertDialog.Builder(MainActivity.this);
				alt_bld.setMessage("Terminate this App ?").setCancelable(
						false).setPositiveButton("Yes",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog, int id) {
								// Action for 'Yes' Button

								globalData.getInstance().getCommManager().requestTermination( list.allApplicationList.get(tmp).getAppId() );
								Log.d("OPEL", "Request to kill ");
							}
						}).setNegativeButton("No",
						new DialogInterface.OnClickListener() {
							public void onClick(DialogInterface dialog, int id) {
								// Action for 'NO' Button
								dialog.cancel();
							}
						});

				AlertDialog alert = alt_bld.create();

				alert.setTitle(list.allApplicationList.get(position).getTitle() );

				Drawable d = new BitmapDrawable(getResources(), list.allApplicationList.get(position).getImage());
				alert.setIcon(d);
				alert.show();
			}

			return true;
		}
	};

	Handler mHandler = new Handler(Looper.getMainLooper()) {

		@Override
		public void handleMessage(Message inputMessage) {

			if(inputMessage.what == globalData.getInstance().getCommManager().getUPDATE_UI()){

				//String msg = (String) inputMessage.obj;
				MainActivity.updateDisplayItem();
				appManagerView.updateDisplay();
				eventLoggerView.updateDisplay();

				//update All of the UI page
			}

			else if (inputMessage.what == globalData.getInstance().getCommManager().getUPDATE_TOAST()){
				String toastMsg = (String) inputMessage.obj;
				Toast.makeText(getApplicationContext(),	toastMsg, 0).show();
				//update All of the UI page
			}

			else if (inputMessage.what == globalData.getInstance().getCommManager().getMAKE_NOTI()){
				String notiJson = (String) inputMessage.obj;
				MainActivity.updateDisplayItem();
				appManagerView.updateDisplay();
				makeNotification(notiJson);

				//update All of the UI page
			}

			else if (inputMessage.what == globalData.getInstance().getCommManager().getUPDATE_FILEMANAGER()){
				jsonParser jp = (jsonParser) inputMessage.obj;
				fileManagerView.updateDisplay(jp);

			}

			else if (inputMessage.what == globalData.getInstance().getCommManager().getEXE_FILE() ){
				jsonParser jp = (jsonParser) inputMessage.obj;
				fileManagerView.runRequestedFile(getApplicationContext(), jp);

			}

			else if (inputMessage.what == globalData.getInstance().getCommManager().getSHARE_FILE() ){
				jsonParser jp = (jsonParser) inputMessage.obj;

				fileManagerView.runSharingFile(getApplicationContext(), jp);

			}
			else if(inputMessage.what == global_communication.COMM_CONNECTED){
				Log.d("OPEL", "Toast connected");
				conn_stat = true;
				Toast.makeText(getApplicationContext(), "Successfully connected to OPEL", 0).show();

			}
			else if(inputMessage.what == global_communication.COMM_DISCONNECTED){
				Log.d("OPEL", "Toast disconnected");
				conn_stat = false;
				Toast.makeText(getApplicationContext(), "Disconnected to OPEL", 0).show();
			}
			else if(inputMessage.what == global_communication.COMM_CONNECTING){
				Log.d("OPEL", "Toast disconnected");
				Toast.makeText(getApplicationContext(), "Connecting to OPEL", 0).show();
			}
			else if (inputMessage.what == global_communication.COMM_CONNECT_FAILED){
				Log.d("OPEL", "Toast disconnected");
				conn_stat = false;
				Toast.makeText(getApplicationContext(), "Failed connecting to OPEL", 0).show();
			}
			else if (inputMessage.what == global_communication.COMM_ALREADY_CONNECTED){
				Log.d("OPEL", "Toast already connecteed");
				conn_stat = true;
				Toast.makeText(getApplicationContext(), "Already conneceted", 0).show();
			}
			else if (inputMessage.what == global_communication.COMM_ALREADY_CONNECTING){
				Log.d("OPEL", "Toast already connecteed");
				Toast.makeText(getApplicationContext(), "Already connecting", 0).show();
			}

		}
	};

	public void initStorageWorkspace(){

		File opelDir = new File(Environment.getExternalStorageDirectory().getPath() + "/OPEL");
		File opelRUIDir = new File(Environment.getExternalStorageDirectory().getPath() + "/OPEL/RemoteUI");
		File opelRStorageDir = new File(Environment.getExternalStorageDirectory().getPath() + "/OPEL/RemoteStorage");
		File opelIconDir = new File(Environment.getExternalStorageDirectory().getPath() + "/OPEL/Icon");
		File opelCloudDir = new File(Environment.getExternalStorageDirectory().getPath() + "/OPEL/CloudService");

		if(!opelDir.exists()){
			opelDir.mkdir();
		}
		if(!opelRUIDir.exists()){
			opelRUIDir.mkdir();
		}
		if(!opelRStorageDir.exists()){
			opelRStorageDir.mkdir();
		}
		if(!opelIconDir.exists()){
			opelIconDir.mkdir();
		}
		if(!opelCloudDir.exists()){
			opelCloudDir.mkdir();
		}

		globalData.getInstance().setOpelStoragePath(opelDir );
		globalData.getInstance().setRUIStoragePath(opelRUIDir );
		globalData.getInstance().setRStorageStoragePath(opelRStorageDir );
		globalData.getInstance().setIconDirectoryPath(opelIconDir );
		globalData.getInstance().setCloudStoragePath(opelCloudDir );

		//File storage = getExternalFilesDir(STORAGE_SERVICE);

		// globalData.getInstance().setStoragePath( getApplicationContext().getExternalFilesDir( Environment.DIRECTORY_PICTURES )  );
		//globalData.getInstance().setStoragePath(Environment.getExternalStorageDirectory());		// KITKAT Data path
		//globalData.getInstance().setStoragePath( new File(Environment.getExternalStorageDirectory().getPath()+"/OPEL"));		// KITKAT Data path

	}

	public void getNativeAppList(global_appList list){

		list.addAllApplicationList(new application("-1", "Camera", BitmapFactory.decodeResource(this.getResources(), R.drawable.cam), -1));
		//list.addAllApplicationList(new application("-1", "Gallery", BitmapFactory.decodeResource(this.getResources(), R.drawable.gallery), -1));
		list.addAllApplicationList(new application("-1", "Sensor", BitmapFactory.decodeResource(this.getResources(), R.drawable.sensor),  -1));
		list.addAllApplicationList(new application("-1", "App Market", BitmapFactory.decodeResource(this.getResources(), R.drawable.market),  -1));
		list.addAllApplicationList(new application("-1", "App Manager",BitmapFactory.decodeResource(this.getResources(), R.drawable.appmanager),  -1));
		list.addAllApplicationList(new application("-1", "File Manager",BitmapFactory.decodeResource(this.getResources(), R.drawable.filemanager),  -1));
		list.addAllApplicationList(new application("-1", "Event Logger",BitmapFactory.decodeResource(this.getResources(), R.drawable.eventlogger),  -1));
		list.addAllApplicationList(new application("-1", "Connect", BitmapFactory.decodeResource(this.getResources(), R.drawable.connect),  -1));
		list.addAllApplicationList(new application("-1", "Setting", BitmapFactory.decodeResource(this.getResources(), R.drawable.setting),  -1));

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

	public void makeNotification(String JsonData){

		NotificationManager nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		Resources res = getResources();

		Intent notificationIntent = new Intent(this, notificationView.class);
		Bundle extras = new Bundle();
		extras.putString("jsonData", JsonData);
		extras.putString("checkNoti", "1");
		notificationIntent.putExtras(extras);

		PendingIntent contentIntent = PendingIntent.getActivity(this, 1, notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);

		jsonParser jp = new jsonParser(JsonData);
		String appId = jp.getValueByKey("appID");
		application targetApp = globalData.getInstance().getAppList().getAppInAllList(appId);

		NotificationCompat.Builder builder = new NotificationCompat.Builder(this)
				.setCategory(appId)
				.setContentTitle(targetApp.getTitle())
				.setContentText( jp.getValueByKey("description"))
				.setTicker(" "+jp.getValueByKey("appTitle"))
				.setLargeIcon(globalData.getInstance().getAppList().getAppInAllList(appId).getImage())
				.setSmallIcon(R.drawable.opel)
				.setContentIntent(contentIntent)
				.setAutoCancel(true)
				.setWhen(System.currentTimeMillis())
				.setDefaults( Notification.DEFAULT_SOUND|Notification.DEFAULT_VIBRATE|Notification.DEFAULT_LIGHTS)
				.setNumber(1);

		Notification  n = builder.build();

		nm.notify(1234, n);

	}
}


class mainView extends ArrayAdapter<application> {
	Context context;
	int layoutResourceId;
	ArrayList<application> data;


	public mainView(Context context, int layoutResourceId,
					ArrayList<application> data) {
		super(context, layoutResourceId, data);
		this.layoutResourceId = layoutResourceId;
		this.context = context;
		this.data = data;
	}

	public void updateDisplay(){
		this.data = globalData.getInstance().getAppList().getAllApplicationList();
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

		application item = data.get(position);
		holder.txtTitle.setText(item.getTitle());
		holder.imageItem.setImageBitmap(item.getImage());
		return row;

	}

	static class RecordHolder {
		TextView txtTitle;
		ImageView imageItem;
	}
}



