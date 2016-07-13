package com.example.opel_manager;

import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteDatabase.CursorFactory;
import android.database.sqlite.SQLiteOpenHelper;
import android.graphics.Bitmap;
import android.net.wifi.p2p.WifiP2pManager;
import android.provider.BaseColumns;
import android.util.Log;

import java.io.File;
import java.util.ArrayList;
import java.util.concurrent.Semaphore;

import selectiveconnection.WifiDirectBroadcastReceiver;

public class globalData {

	private boolean isInit;
	private boolean isLoading;

	private global_communication communication = new global_communication();
	private global_appList appList = new global_appList();
	private global_eventList eventList = new global_eventList();
	private global_communication_mqtt mqtt = new global_communication_mqtt();

	/* esevan */
	private WifiP2pManager mManager = null;
	private WifiP2pManager.Channel mChannel = null;
	private WifiDirectBroadcastReceiver mReceiver = null;
	private IntentFilter mIntentFilter = null;
	//

	private String sensorInfoJson = "N/A";
	private String cameraInfoJson = "N/A";

	private String device_ip = "N/A";

	Semaphore allAppListLock = new Semaphore(1);

	private File opelDirectory;
	private File opelIconDirectory;
	private File opelRemoteUIDirectory;
	private File opelRemoteStorageDirectory;
	private File opelCloudServiceDirectory;

	public global_communication_mqtt getMqttManager(){	//ES Conflict
		return this.mqtt;
	}

	public void setIntentFilter(IntentFilter i){
		mIntentFilter = i;
	}
	public IntentFilter getIntentFilter(){
		return mIntentFilter;
	}
	public void setWifiP2pManager(WifiP2pManager manager){
		mManager = manager;
	}
	public WifiP2pManager getWifiP2pManager(){
		return mManager;
	}

	public void setWifiChannel(WifiP2pManager.Channel channel){
		mChannel = channel;
	}
	public WifiP2pManager.Channel getChannel(){
		return mChannel;
	}
	public void setWifiReceiver(WifiDirectBroadcastReceiver receiver){
		mReceiver = receiver;
	}
	public WifiDirectBroadcastReceiver getWifiReceiver(){
		return mReceiver;
	}

	public String getDeviceIP() {
		return this.device_ip;
	}

	public void setDeviceIP(String device_ip) {
		this.device_ip = device_ip;
	}

	public String getSensorInfo() {
		return this.sensorInfoJson;
	}

	public void setSensorInfo(String sensorInfo) {
		this.sensorInfoJson = sensorInfo;
	}

	public String getCameraInfo() {
		return this.cameraInfoJson;
	}
	public void setCameraInfo(String cameraInfoJson) {
		this.cameraInfoJson = cameraInfoJson;
	}

	public void setOpelStoragePath(File f) {
		this.opelDirectory = f;
	}
	public File getOpelStoragePath() {
		return opelDirectory;
	}

	public void setRUIStoragePath(File f) {
		this.opelRemoteUIDirectory = f;
	}
	public File getRUIStoragePath() {
		return opelRemoteUIDirectory;
	}

	public void setIconDirectoryPath(File f) {
		this.opelIconDirectory = f;
	}
	public File getIconDirectoryPath() {
		return opelIconDirectory;
	}

	public void setCloudStoragePath(File f) {
		this.opelCloudServiceDirectory = f;
	}
	public File getCloudStoragePath() {
		return opelCloudServiceDirectory;
	}



	public File getRStorageStoragePath() {
		return opelRemoteStorageDirectory;
	}
	public void setRStorageStoragePath(File f) {
		this.opelRemoteStorageDirectory = f;
	}

	public boolean getIsLoading(){
		return this.isLoading;
	}

	public void setIsLoading(boolean b){
		this.isLoading = b;
	}

	public global_appList getAppList() {
		return appList;
	}

	public global_communication getCommManager() {
		return this.communication;
	}

	public global_eventList getEventList(){
		return this.eventList;
	}


	private globalData() {
		isInit = false;
		isLoading = false;
	}

	public boolean isInit() {
		return this.isInit;
	}

	public void initComplete() {
		this.isInit = true;
	}

	private volatile static globalData instance = null;

	public static globalData getInstance() {
		if (instance == null) {
			synchronized (globalData.class) {
				if (instance == null) {
					instance = new globalData();
				}
			}
		}
		return instance;
	}

	public void exitApp() {
		// TODO Auto-generated method stub
		this.eventList.close();
	}
}




class global_eventList{

	private static final String DATABASE_NAME = "eventList.db";
	private static final int DATABASE_VERSION = 1;
	public static SQLiteDatabase mDB;
	private DatabaseHelper eventDatabaseHelper;
	private Context mCtx;

	private Semaphore eventListLock = new Semaphore(1);

	ArrayList<event> evList = new ArrayList<event>();

	private class DatabaseHelper extends SQLiteOpenHelper{

		public DatabaseHelper(Context context, String name,	CursorFactory factory, int version) {
			super(context, name, factory, version);
		}

		public void onCreate(SQLiteDatabase db) {
			db.execSQL(DataBases.CreateDB._CREATE);
		}

		@Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
			db.execSQL("DROP TABLE IF EXISTS "+DataBases.CreateDB.TABLENAME);
			onCreate(db);
		}
	}

	public global_eventList(){


	}
	public global_eventList(Context context){
		this.mCtx = context;

	}

	public global_eventList open(Context mCtx) throws SQLException{
		eventDatabaseHelper = new DatabaseHelper(mCtx, DATABASE_NAME, null, DATABASE_VERSION);

		mDB = eventDatabaseHelper.getWritableDatabase();
		evList = getAllEventArrayList();

		this.mCtx = mCtx;
		return this;
	}

	public void close(){
		mDB.close();
		evList.clear();
	}

	// Insert DB
	public ArrayList<event> addEvent(String eventAppID, String eventAppName, String eventDescription, String eventTime, String eventJsonData){
		ContentValues values = new ContentValues();

		values.put(DataBases.CreateDB.eventAppID, eventAppID);
		values.put(DataBases.CreateDB.eventAppName, eventAppName);
		values.put(DataBases.CreateDB.eventDescription, eventDescription);
		values.put(DataBases.CreateDB.eventTime, eventTime);
		values.put(DataBases.CreateDB.eventJsonData, eventJsonData);

		long id = mDB.insert(DataBases.CreateDB.TABLENAME, null, values);

		//success
		if( id != -1){
			evList.add(0, new event(id, eventAppID, eventAppName, eventDescription, eventTime, eventJsonData) );

			Log.d("OPEL", "SUCCESS ADD EVENT");
		}

		return this.evList;
	}
/*
	// Update DB
	public boolean updateColumn(long id ,String eventAppID, String eventAppName, String eventDescription, String eventTime, String eventJsonData){
		ContentValues values = new ContentValues();

		values.put(DataBases.CreateDB.eventAppID, eventAppID);
		values.put(DataBases.CreateDB.eventAppName, eventAppName);
		values.put(DataBases.CreateDB.eventDescription, eventDescription);
		values.put(DataBases.CreateDB.eventTime, eventTime);
		values.put(DataBases.CreateDB.eventJsonData, eventJsonData);

		return mDB.update(DataBases.CreateDB.TABLENAME, values, "_id="+id, null) > 0;
	}
*/

	// Delete ID
	// TODO : erase img file that the noti page involve
	public ArrayList<event> deleteEvent(long id){
		int _id = mDB.delete(DataBases.CreateDB.TABLENAME, "_id="+id, null);

		if (_id > 0){

			for(int i=0; i<evList.size(); i++){

				if( evList.get(i).getEventID()== id ){
					evList.remove(i);

					break;
				}
			}
		}

		return this.evList;
	}

	// Select All
	private Cursor getAllEvents(){
		return mDB.query(DataBases.CreateDB.TABLENAME, null, null, null, null, null, "eventTime desc");
	}

	private Cursor getEvent(long id){
		Cursor c = mDB.query(DataBases.CreateDB.TABLENAME, null, "_id="+id, null, null, null, null);
		if(c != null && c.getCount() != 0)
			c.moveToFirst();
		return c;
	}

	private Cursor getMatchApp(String appID){
		Cursor c = mDB.rawQuery( "select * from "+DataBases.CreateDB.TABLENAME+" where eventAppID=" + "'" + appID + "'" + "order by eventTime desc" , null);
		return c;
	}

	public ArrayList<event> eventAllClear(){
		if ( mDB.delete(DataBases.CreateDB.TABLENAME, null, null) > 0){
			evList.clear();

		}
		return this.evList;
	}

	public ArrayList<event> getCurEventArrayList(){
		return this.evList;
	}

	public ArrayList<event> getAllEventArrayList(){
		Cursor mCursor;
		ArrayList<event> list = new ArrayList<event>();
		mCursor = null;
		mCursor = getAllEvents();

		while (mCursor.moveToNext()) {

			event e = new event(
					mCursor.getLong(mCursor.getColumnIndex("_id")),
					mCursor.getString(mCursor.getColumnIndex("eventAppID")),
					mCursor.getString(mCursor.getColumnIndex("eventAppName")),
					mCursor.getString(mCursor.getColumnIndex("eventDescription")),
					mCursor.getString(mCursor.getColumnIndex("eventTime")),
					mCursor.getString(mCursor.getColumnIndex("eventJsonData"))
			);

			list.add(e);
		}

		mCursor.close();
		this.evList = list;
		return this.evList;
	}

	public ArrayList<event> getAppEventArrayList(String appID){
		Cursor mCursor;
		ArrayList<event> list = new ArrayList<event>();
		mCursor = null;
		mCursor = getMatchApp(appID);

		while (mCursor.moveToNext()) {

			event e = new event(
					mCursor.getLong(mCursor.getColumnIndex("_id")),
					mCursor.getString(mCursor.getColumnIndex("eventAppID")),
					mCursor.getString(mCursor.getColumnIndex("eventAppName")),
					mCursor.getString(mCursor.getColumnIndex("eventDescription")),
					mCursor.getString(mCursor.getColumnIndex("eventTime")),
					mCursor.getString(mCursor.getColumnIndex("eventJsonData"))
			);

			list.add(e);
		}

		mCursor.close();

		this.evList = list;
		return this.evList;
	}
}

class DataBases {

	public static final class CreateDB implements BaseColumns{

		public static final String eventAppID = "eventAppID";
		public static final String eventAppName = "eventAppName";
		public static final String eventDescription = "eventDescription";
		public static final String eventTime = "eventTime";
		public static final String eventJsonData = "eventJsonData";

		public static final String TABLENAME = "EVENTTABLE";
		public static final String _CREATE =
				"create table "+TABLENAME+"("
						+_ID+" integer primary key autoincrement, "
						+eventAppID+" text not null , "
						+eventAppName+" text not null , "
						+eventDescription+" text not null , "
						+eventTime+" text not null , "
						+eventJsonData+" text not null )";
	}
}

class event{
	private long eventID; 		//DB Primary key
	private String eventAppID;		//App ID
	private String eventTime;
	private String eventJsonData;
	private String eventDescription;
	private String eventAppName;
	private int status;
	//0: unchecked event, 1: unchecked noti, 2:checked event/noti

	public event(long eventID, String eventAppID, String appName, String description, String time, String jsonData){
		this.eventID = eventID;
		this.eventAppID = eventAppID;
		this.eventAppName = appName;
		this.eventDescription = description;
		this.eventTime = time;
		this.eventJsonData = jsonData;
		this.status = 0;
	}

	public long getEventID(){
		return this.eventID;
	}
	public String getEventAppID(){
		return this.eventAppID;
	}

	public String getEventTime(){
		return this.eventTime;
	}

	public String getEventJsonData(){
		return this.eventJsonData;
	}

	public String getEventDescription(){
		return this.eventDescription;
	}

	public String getEventAppName(){
		return this.eventAppName;
	}

	public String getEvent(){
		return 	this.eventID +" "+ this.eventAppID + " " + this.eventAppName + " " + this.eventDescription + " "+this.eventTime +" " + this.eventJsonData;
	}
}









class global_appList {

	ArrayList<application> allApplicationList;

	public global_appList() {
		allApplicationList = new ArrayList<application>();
	}

	// -> ready
	public void installApplication(application app) {

		addAllApplicationList(app);
		app.setTypeToInstalled();
	}

	// ready -> remove
	public void uninstallApplication(application app) {

		removeAllApplicationList(app);

	}

	// ready -> running
	public void runApplication(application app) {

		app.setTypeToRunning();
		//addRunningApplicationList(app);

	}

	// running -> ready
	public void terminateApplication(application app) {

		app.setTypeToInstalled();
		//removeRunningApplicationList(app);
	}

	public void addAllApplicationList(application icon) {
		allApplicationList.add(icon);
	}

	public void removeAllApplicationList(application icon) {
		for(int i=0; i<allApplicationList.size(); i++){
			if (allApplicationList.get(i).getAppId().equals(icon.getAppId())){
				allApplicationList.remove(i);
			}
		}
	}

	public ArrayList<application> getAllApplicationList() {
		return this.allApplicationList;
	}



	public application getAppInAllList(String appId) {

		for (int i = 0; i < allApplicationList.size(); i++) {
			application tmpApp = allApplicationList.get(i);
			if (tmpApp.getAppId().equals(appId)) {
				return tmpApp;
			}
		}

		return null;
	}

}


class application {

	String appId;
	String title;

	Bitmap image;

	String jsonStringForTermincation;
	String jsonStringForConfig;
	int type; // -1: Native Menu, 0:Installed, 1:Running

	public application(String appID, String title, Bitmap image, int type) {
		super();
		this.appId = appID;
		this.image = image;
		this.title = title;
		this.type = type;

		this.jsonStringForConfig = "N/A";
		this.jsonStringForTermincation = "N/A";
	}

	public String getAppId() {
		return appId;
	}

	public void setAppId(String appId) {
		this.appId = appId;
	}

	public Bitmap getImage() {
		return image;
	}

	public void setImage(Bitmap image) {
		this.image = image;
	}

	public String getTitle() {
		return title;
	}

	public void setTitle(String title) {
		this.title = title;
	}

	public int getType() {
		return type;
	}

	public void setType(int type) {
		this.type = type;
	}

	public void setTypeToNative() {
		this.type = -1;
	}

	public void setTypeToInstalled() {
		this.type = 0;
	}

	public void setTypeToRunning() {
		this.type = 1;
	}

	public String getConfigJson() {
		return jsonStringForConfig;
	}

	public void setConfigJson(String json) {
		this.jsonStringForConfig = json;
	}
	public String getTerminationJson() {
		return jsonStringForTermincation;
	}

	public void setTerminationJson(String json) {
		this.jsonStringForTermincation = json;
	}
}


