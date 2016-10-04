package com.example.opel_manager;
import android.bluetooth.BluetoothSocket;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Arrays;
import java.util.UUID;

import selectiveconnection.OpelCommunicator;
import selectiveconnection.WifiDirectBroadcastReceiver;

public class global_communication {

	public OpelCommunicator opelCommunicator;
	WifiDirectBroadcastReceiver wbReceiver;

	BluetoothSocket socket;
	DataInputStream in;
	DataOutputStream out;

	//Communication type
	private static final String INSTALLPKG = "1000";
	private static final String EXEAPP = "1001";
	private static final String EXITAPP = "1002";
	private static final String DELETEAPP = "1003";
	private static final String UPDATEAPPINFO = "1004";
	private static final String NOTI = "1005";
	private static final String UPDATE_SENSOR_INFO = "1006";
	private static final String UPDATE_CAMERA_INFO = "1007";
	private static final String NOTI_PRELOAD_IMG = "1008";

	private static final String CONFIG_REGISTER  =  "1009";
	private static final String CONFIG_EVENT	=		"1010";

	private static final String RUN_NATIVE_CAMERAVIEWER	="1011";
	private static final String RUN_NATIVE_SENSORVIEWER ="1012";
	private static final String TERM_NATIVE_CAMERAVIEWER ="2011";
	private static final String TERM_NATIVE_SENSORVIEWER ="2012";
	private static final String ANDROID_TERMINATE ="1013";

	private static final String RemoteFileManager_getListOfCurPath = "1014";
	private static final String RemoteFileManager_requestFile = "1015";
	private static final String RemoteFileManager_requestFile_Preload = "1016";

	private static final String CloudService_faceRecognition = "1017";
	private static final String CloudService_faceRecognition_preload = "1018";

	private static final String NIL_TERMINATION = "1100";
	private static final String NIL_CONFIGURATION = "1101";

	private static final String TAG = "OPEL";

	public static final int COMM_DISCONNECTED = 44;
	public static final int COMM_CONNECTED = 33;
	public static final int COMM_CONNECTING = 22;
	public static final int COMM_CONNECT_FAILED = 11;
	public static final int COMM_ALREADY_CONNECTED = 55;
	public static final int COMM_ALREADY_CONNECTING = 66;


	//UI Handling type on Handler of MainUIThread
	private final int UPDATE_UI = 3000;
	private final int UPDATE_TOAST = 3001;
	private final int MAKE_NOTI = 3002;
	private final int UPDATE_FILEMANAGER = 3003;
	private final int EXE_FILE = 3004;
	private final int SHARE_FILE = 3005;

	private CommunicationHandler myClientTask;
	private Handler handler;

	private int stat;
	private void handle_disconnected() throws NullPointerException{
		Log.d(TAG, "Disconnected");
		stat = COMM_DISCONNECTED;
		myClientTask.Cancel();
		int port = OpelCommunicator.CMFW_DEFAULT_PORT;
		opelCommunicator.close(port);
		handler.obtainMessage(COMM_DISCONNECTED).sendToTarget();
	}
	private void connect_failed(){
		stat = COMM_DISCONNECTED;
		myClientTask.Cancel();
		int port = OpelCommunicator.CMFW_DEFAULT_PORT;
		opelCommunicator.close(port);
		handler.obtainMessage(COMM_CONNECT_FAILED).sendToTarget();
	}
	private void handle_connected(){
		stat = COMM_CONNECTED;
		Log.d(TAG, "Connected!");
		handler.obtainMessage(COMM_CONNECTED).sendToTarget();
		requestUpdateAppInfomation();

		//Temporal bug fix
		//globalData.getInstance().getCommManager().requestRunNativeJSAppSensorViewer(); // [CHECK]
	}
	private void handle_connecting(){
		stat = COMM_CONNECTING;
		handler.obtainMessage(COMM_CONNECTING).sendToTarget();
	}
	private void already_connected(){
		handler.obtainMessage(COMM_ALREADY_CONNECTED).sendToTarget();
	}
	private void already_connecting(){
		handler.obtainMessage(COMM_ALREADY_CONNECTING).sendToTarget();
	}
	public global_communication() {
		stat = COMM_DISCONNECTED;
		socket = null;
		myClientTask = null;
		handler = null;
		opelCommunicator = null;
	}

	public boolean isConnect(){

		if( stat == COMM_DISCONNECTED){
			return false;
		}

		return true;
	}

	public void setOpelCommunicator(){
		opelCommunicator = new OpelCommunicator(globalData.getInstance().getWifiP2pManager(), globalData.getInstance().getChannel());
	}
	public void setHandler(Handler handler){
		this.handler = handler;
		if(myClientTask != null)
			myClientTask.setHandler(handler);

		/*
		if(socket == null || socket.isConnected() == false){
			Connect();
		}
		*/
	}

	public int getUPDATE_UI(){
		return UPDATE_UI;
	}

	public int getUPDATE_TOAST(){
		return UPDATE_TOAST;
	}


	public int getMAKE_NOTI(){
		return MAKE_NOTI;
	}

	public int getUPDATE_FILEMANAGER(){
		return UPDATE_FILEMANAGER;
	}

	public int getEXE_FILE(){
		return EXE_FILE;
	}

	public int getSHARE_FILE(){
		return SHARE_FILE;
	}

	public void Kill(){
		if(myClientTask != null)
			myClientTask.Cancel();
	}

	public void Connect(){
		int port = OpelCommunicator.CMFW_DEFAULT_PORT;
		if(stat != COMM_DISCONNECTED){
			Log.d(TAG, "duplicated connect");
			return;
		}
		if(handler == null){
			Log.d(TAG, "Handler is not set");
			return;
		}
		if(socket != null && socket.isConnected() == true){
			already_connected();
			return;
		}
		else if(socket != null && stat == COMM_CONNECTING){
			already_connecting();
			return;
		}
		handle_connecting();
		if(myClientTask != null){
			myClientTask.Cancel();
			myClientTask = null;
		}

		if(myClientTask == null){
			myClientTask = new CommunicationHandler();
			myClientTask.setHandler(handler);
		}

		myClientTask.start();
	}

	class CommunicationHandler extends Thread {
		Handler handler;
		boolean sch;

		public CommunicationHandler() {
			sch = false;
		}

		public void setHandler(Handler handler){
			this.handler = handler;


		}
		public long char2long(char c){
			return c & 0x00000000000000FFL;
		}
		public UUID name2uuid(String str){
			char[] inputString = str.toCharArray();
			int length = inputString.length;
			long[] longData = new long[2];
			for(int i=0; i<2; i++){
				longData[i] = 0;
				if(i*8 < length)
					longData[i] = char2long(inputString[i*8]) <<56;
				if(i*8+1 < length)
					longData[i] |= char2long(inputString[i*8+1]) << 48;
				if(i*8+2 < length)
					longData[i] |= char2long(inputString[i*8+2]) << 40;
				if(i*8+3 < length)
					longData[i] |= char2long(inputString[i*8+3]) << 32;
				if(i*8+4 < length)
					longData[i] |= char2long(inputString[i*8+4]) << 24;
				if(i*8+5 < length)
					longData[i] |= char2long(inputString[i*8+5]) << 16;
				if(i*8+6 < length)
					longData[i] |= char2long(inputString[i*8+6]) << 8;
				if(i*8+7 < length)
					longData[i] |= char2long(inputString[i*8+7]);

			}

			return new UUID(longData[0], longData[1]);
		}

		public void Cancel(){
			sch = false;
		}
		@Override
		public void run() throws NullPointerException  {

			sch = true;
			int port = OpelCommunicator.CMFW_DEFAULT_PORT;
			/*
			Bluetooth Connection
			 */
			if(opelCommunicator.connect(port) == false){
				connect_failed();
				return;
			}
			else{
				Log.d("BTConnect", "Success");
				handle_connected();
			}

			while (sch) {
				String rcvJson = rcvMsg();
				if(rcvJson.equals("")){
					continue;
				}
				jsonParser jp = new jsonParser(rcvJson);

				String req = jp.getValueByKey("type");

				if (req.equals(INSTALLPKG)) {
					handleInstall(jp);
				}

				else if (req.equals(EXEAPP)) {
					handleStart(jp);
				}

				else if (req.equals(EXITAPP)) {
					handleTermination(jp);
				}

				else if (req.equals(DELETEAPP)) {
					handleUninstall(jp);
				}

				else if (req.equals(UPDATEAPPINFO)) {
					if(globalData.getInstance().getIsLoading() == false){

						handleUpdateAppInfomation(jp);
						globalData.getInstance().setIsLoading(true);

						/*while(!MainActivity.mainLoadingProgDialog.isShowing()) {
							MainActivity.mainLoadingProgDialog.dismiss();
						}*/
					}
				}

				else if (req.equals(NOTI)) {

					if(jp.getValueByKey("isNoti").equals("1"))
						handleEventWithNoti(jp);
					else
						handleEventWithoutNoti(jp);


				}
				else if (req.equals(NOTI_PRELOAD_IMG)) {
					String str = handleNotiPreLoadImg(jp);
					Log.d("OPEL", "NOTI PRELOAD COMPLETE FILENAME: "+ str);
				}
				else if (req.equals(UPDATE_SENSOR_INFO)) {

					handleUpdateSensorInfomation(jp);
				}

				else if (req.equals(UPDATE_CAMERA_INFO)) {

					handleUpdateCameraInfomation(jp);
				}

				else if (req.equals(NIL_TERMINATION)) {

					String appID = jp.getValueByKey("appID");
					globalData.getInstance().getAppList().getAppInAllList(appID).setTerminationJson(jp.getJsonData());

					Log.d("OPEL", "NIL_TERMINATION :: " + jp.getJsonData());
				}


				else if (req.equals(CONFIG_REGISTER)) {

					handleRegisterConfig(jp);
				}

				else if (req.equals(RemoteFileManager_getListOfCurPath)) {

					handleUpdateFileManager(jp);
				}

				else if (req.equals(RemoteFileManager_requestFile)) {

					handleRequestFileManager_requestFile(jp);
				}

				else if (req.equals(RemoteFileManager_requestFile_Preload) ) {
					Log.d("OPEL", "Preload for fileManager  : " + jp.getValueByKey("img"));
					String fileName = handleRequestFileManager_requestFile_Preload(jp);

				}

				else if (req.equals(CloudService_faceRecognition) ) {
					Log.d("OPEL", "CloudService_faceRecognition for cloud service " );
					handleCloudService(jp);

				}

				else if (req.equals(CloudService_faceRecognition_preload) ) {

					String fileName = handleCloudService_File_Preload(jp);
					Log.d("OPEL", "Preload for cloud service  : " + fileName);
				}



				else {
					Log.d("OPEL", "Unexpected responce/request  :  " + req);
				}
			}


			if(socket!=null && socket.isConnected())
				try {
					socket.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}

			handle_disconnected();

		}


		void handleInstall(jsonParser jp) {
			// getAppID
			// appList.installApplication(app);

//				Move to socket listener when the opk file is sent completely
/*						applicationList list = globalData.getmAllAppList();
			list.addAllApplicationList(new application("1",  title, BitmapFactory.decodeResource(getResources(), R.drawable.app), 1));
			MainActivity.updateDisplayItem();
*/

			String appID = jp.getValueByKey("appID");
			String appName = jp.getValueByKey("appName");

			String fName = rcvFile(globalData.getInstance().getIconDirectoryPath(), jp);
			if(fName.equals(""))
				return;

			Bitmap bitmap = null;
			File f = new File(globalData.getInstance().getIconDirectoryPath(), fName);
			BitmapFactory.Options options = new BitmapFactory.Options();
			options.inPreferredConfig = Bitmap.Config.ARGB_8888;

			try {
				bitmap = BitmapFactory.decodeStream(new FileInputStream(f),null, options);
			} catch (FileNotFoundException e) {
				e.printStackTrace();
			}

			globalData.getInstance().getAppList().installApplication(new application(appID, appName, bitmap,  0));

			//appMarketView release
			appMarketView.marketProgDialog.dismiss();
			updateMainUIThread(UPDATE_UI);
			updateMainUIThread(UPDATE_TOAST, "Complete the installation : "+ appName);

			try{
				File downloaddir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
				File pkgFile = new File(downloaddir, jp.getValueByKey("pkgFileName"));
				pkgFile.delete();
			}

			catch(Exception e){
				e.printStackTrace();
			}

		}

		//{type:UPDATEAPPINFO, appID/Type:appName, .....}
		//get all information : appID, appName, type (bitmap)

		void handleUpdateAppInfomation(jsonParser jp) {

			while(jp.hasMoreValue()){
				String ret[] = new String[2];
				ret=jp.getNextKeyValue();

				if(ret[0].equals("type") && ret[1].equals(UPDATEAPPINFO)){
					continue;
				}
				if(ret[0].equals("IP_ADDR__a")){
					globalData.getInstance().setDeviceIP( ret[1] );
					continue;
				}

				// [MORE] If the icon img file doesn't exist, need to request to OPEL device
				Bitmap bitmap = null;

				String fileName = ret[0].substring(0,ret[0].length()-2) + ".icon";
				File f = new File(globalData.getInstance().getIconDirectoryPath(),fileName);

				BitmapFactory.Options options = new BitmapFactory.Options();
				options.inPreferredConfig = Bitmap.Config.ARGB_8888;

				try {
					bitmap = BitmapFactory.decodeStream(new FileInputStream(f),null, options);
				} catch (FileNotFoundException e) {
					e.printStackTrace();
				}

				String appID = ret[0].substring(0,  ret[0].length() -2);
				int type = Integer.parseInt( ret[0].substring(ret[0].length() -1, ret[0].length()));

				globalData.getInstance().getAppList().addAllApplicationList(new application( appID, ret[1], bitmap,  type));
			}

			updateMainUIThread(UPDATE_UI);
		}

		void handleUninstall(jsonParser jp) {
			String appID = jp.getValueByKey("appID");

			application deleteTargetApp = globalData.getInstance().getAppList().getAppInAllList(appID);

			//Remove the item of app list
			globalData.getInstance().getAppList().uninstallApplication(deleteTargetApp);

			//Remove the icon file
			String fileName =appID + ".icon";
			File f = new File(globalData.getInstance().getIconDirectoryPath(),fileName);
			f.delete();

			updateMainUIThread(UPDATE_UI);
			updateMainUIThread(UPDATE_TOAST, "Complete the uninstallation");
		}

		//{type:EXEAPP, appID:id, .....}
		void handleStart(jsonParser jp) {

			String appID = jp.getValueByKey("appID");
			globalData.getInstance().getAppList().getAppInAllList(appID).setTypeToRunning();
			updateMainUIThread(UPDATE_UI);
			updateMainUIThread(UPDATE_TOAST, "Run application : "+ globalData.getInstance().getAppList().getAppInAllList(appID).getTitle());
		}

		//{type:EXIXAPP, appID:id, .....}
		void handleTermination(jsonParser jp) {
			Log.d("OPEL", "handTermination > json : "+ jp.getJsonData());
			String appID = jp.getValueByKey("appID");
			globalData.getInstance().getAppList().getAppInAllList(appID).setTypeToInstalled();
			updateMainUIThread(UPDATE_UI);

			//set termination js to N/A
		}

		void handleUpdateSensorInfomation(jsonParser jp) {
			globalData.getInstance().setSensorInfo(jp.getJsonData());
			updateMainUIThread(UPDATE_UI);
		}

		void handleUpdateCameraInfomation(jsonParser jp) {
			globalData.getInstance().setCameraInfo(jp.getJsonData());
			updateMainUIThread(UPDATE_UI);
		}

		void handleRegisterConfig(jsonParser jp){

			String appID = jp.getValueByKey("appID");
			globalData.getInstance().getAppList().getAppInAllList(appID).setConfigJson(jp.getJsonData());
		}

		void handleEventWithNoti(jsonParser jp) {

			globalData.getInstance().getEventList().addEvent(jp.getValueByKey("appID"), jp.getValueByKey("appTitle"), jp.getValueByKey("description"), jp.getValueByKey("dateTime"), jp.jsonData);

			updateMainUIThread(MAKE_NOTI, jp.getJsonData() );
			updateMainUIThread(UPDATE_UI);
		}

		void handleEventWithoutNoti(jsonParser jp) {

			globalData.getInstance().getEventList().addEvent(jp.getValueByKey("appID"), jp.getValueByKey("appTitle"), jp.getValueByKey("description"), jp.getValueByKey("dateTime"), jp.jsonData);
			updateMainUIThread(UPDATE_UI);
		}

		String handleNotiPreLoadImg(jsonParser jp){

			return rcvFile(globalData.getInstance().getRUIStoragePath(), jp);

		}

		void handleConfigSetting(jsonParser jp) {

		}

		void handleUpdateFileManager(jsonParser jp){
			updateMainUIThread(UPDATE_FILEMANAGER, jp);
		}

		void handleRequestFileManager_requestFile(jsonParser jp){
			String fileName = jp.getValueByKey("filename");
			if(jp.getValueByKey("share").equals("0")){
				updateMainUIThread(EXE_FILE, jp);
			}
			else if(jp.getValueByKey("share").equals("1")){
				updateMainUIThread(SHARE_FILE, jp);
			}

		}

		String handleRequestFileManager_requestFile_Preload(jsonParser jp){
			return rcvFile(globalData.getInstance().getRStorageStoragePath(), jp);
		}

		void handleCloudService(jsonParser jp){

			int fileSize;
			BufferedInputStream bis;
			try {

				File fd = new File(globalData.getInstance().getCloudStoragePath(), jp.getValueByKey("img"));
				bis = new BufferedInputStream(new FileInputStream(fd));
				fileSize = (int) fd.length();
				int len;
				int size = 4096;

				//meta = meta + "dd";
				String meta = jp.getValueByKey("img");

//				meta += "!";
				int totalLen = 0;

				byte[] data = new byte[fileSize];
				byte[] metadata = meta.getBytes();

				len = bis.read(data, 0, Integer.valueOf(fileSize));

				byte[] res = new byte[data.length + metadata.length];

				for (int i = 0; i < metadata.length; i++) {
					res[i] = metadata[i];
				}

				for (int j = 0; j < data.length; j++) {
					res[metadata.length + j] = data[j];
				}

				globalData.getInstance().getMqttManager().mqttSendFile("opel/img", res);

			}
			catch (Exception e) {
				e.printStackTrace();
				Log.d("OPEL", "Exception: MQTT Send file exception");

				return;
			}

		}


		String handleCloudService_File_Preload(jsonParser jp){
			return rcvFile(globalData.getInstance().getCloudStoragePath(), jp);
		}

		public String rcvMsg() {
			int port = OpelCommunicator.CMFW_DEFAULT_PORT;
			byte[] buf = new byte[4096];
			String msg;
			int res = opelCommunicator.cmfw_recv_msg(port, buf, 4096);
			if( res < 0 ){
				handle_disconnected();
				return "";
			}

			msg = new String(Arrays.copyOfRange(buf, 0, res));

			Log.d("OPEL", "COMManager - rcv Msg : "+ msg);
			return msg;
		}

		public String rcvFile(File destDir, jsonParser jp) {
			int port = OpelCommunicator.CMFW_DEFAULT_PORT;
			String name = "";
			String size = "";

			byte[] buf = new byte[1024];
			Arrays.fill(buf, (byte) 0);

			byte[] buf2 = new byte[1024];
			Arrays.fill(buf2, (byte) 0);



			name = rcvMsg();
			if(name.equals(""))
				return name;
			if(jp.getValueByKey("type").equals(INSTALLPKG)){
				name = jp.getValueByKey("appID") + ".icon";
			}
			size = rcvMsg();
			if(size.equals(""))
				return name;

			//File downloaddir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
			//File f = new File(downloaddir, name);

			File f = new File(destDir, name);
			if(opelCommunicator.cmfw_recv_file(port, f) < 0){
				Log.d("OPEL", "Recv File failed");
				return "";
			}

			return name;
		}

		private void updateMainUIThread(int what){

			Message msg = Message.obtain();
			msg.what = what;

			handler.sendMessage(msg);

		}

		private void updateMainUIThread(int what, Object obj){

			Message msg = Message.obtain();
			msg.what = what;

			msg.obj  = obj;

			handler.sendMessage(msg);

		}


	}


	void requestInstall(String fileName) {
		// send file & msg

		jsonParser jp = new jsonParser();
		jp.makeNewJson();
		jp.addJsonKeyValue("type", INSTALLPKG);
		jp.addJsonKeyValue("pkgFileName", fileName);

		sendMsg(jp.getJsonData());
		sendDownloadFile(fileName);
	}

	void requestUninstall(String appID) {
		jsonParser jp = new jsonParser();
		jp.makeNewJson();
		jp.addJsonKeyValue("type", DELETEAPP);
		jp.addJsonKeyValue("appID", appID);

		sendMsg(jp.getJsonData());
	}

	void requestStart(String appID, String appName) {
		jsonParser jp = new jsonParser();
		jp.makeNewJson();
		jp.addJsonKeyValue("type", EXEAPP);
		jp.addJsonKeyValue("appID", appID);
		jp.addJsonKeyValue("appName", appName);
		sendMsg(jp.getJsonData());
	}

	void requestTermination(String appID) {

		String terminationJson = globalData.getInstance().getAppList().getAppInAllList(appID).getTerminationJson();
		jsonParser jp = new jsonParser(terminationJson);
		Log.d("OPEL", "REQ Termination json : "+ terminationJson);

		jsonParser sendJP = new jsonParser();
		sendJP.makeNewJson();
		sendJP.addJsonKeyValue("type", EXITAPP);
		sendJP.addJsonKeyValue("appID", appID);
		sendJP.addJsonKeyValue("appName", jp.getValueByKey("appName"));
		sendJP.addJsonKeyValue("rqID", jp.getValueByKey("rqID"));
		sendJP.addJsonKeyValue("pid", jp.getValueByKey("pid"));

		sendMsg(sendJP.getJsonData());
	}

	void requestTermination() {

		jsonParser sendJP = new jsonParser();

		sendJP.makeNewJson();
		sendJP.addJsonKeyValue("type", ANDROID_TERMINATE);
		Log.d("OPEL", "Termination json : "+ sendJP);
		sendMsg(sendJP.getJsonData());
	}


	void requestUpdateAppInfomation() {

		jsonParser jp = new jsonParser();
		jp.makeNewJson();
		jp.addJsonKeyValue("type", UPDATEAPPINFO);

		sendMsg(jp.getJsonData());
	}

	void requestUpdateSensorInfomation() {
		jsonParser jp = new jsonParser();
		jp.makeNewJson();
		jp.addJsonKeyValue("type", UPDATE_SENSOR_INFO);

		sendMsg(jp.getJsonData());
	}


	void requestUpdateCameraInfomation() {
		jsonParser jp = new jsonParser();
		jp.makeNewJson();
		jp.addJsonKeyValue("type", UPDATE_CAMERA_INFO);

		sendMsg(jp.getJsonData());
	}


	void requestConfigSetting(jsonParser _jp) {

		_jp.addJsonKeyValue("type", CONFIG_EVENT);

		sendMsg(_jp.getJsonData());
	}

	void requestRunNativeJSAppCameraViewer(){
		jsonParser jp = new jsonParser();
		jp.makeNewJson();

		jp.addJsonKeyValue("type", RUN_NATIVE_CAMERAVIEWER);


		sendMsg(jp.getJsonData());
	}
	void requestRunNativeJSAppSensorViewer(){
		jsonParser jp = new jsonParser();
		jp.makeNewJson();

		jp.addJsonKeyValue("type", RUN_NATIVE_SENSORVIEWER);


		sendMsg(jp.getJsonData());
	}
	public
	void requestTermNativeJSAppCameraViewer(){
		jsonParser jp = new jsonParser();
		jp.makeNewJson();

		jp.addJsonKeyValue("type", TERM_NATIVE_CAMERAVIEWER);


		sendMsg(jp.getJsonData());
	}

	void requestTermNativeJSAppSensorViewer(){
		jsonParser jp = new jsonParser();
		jp.makeNewJson();

		jp.addJsonKeyValue("type", TERM_NATIVE_SENSORVIEWER);

		sendMsg(jp.getJsonData());
	}

	void requestUpdateFileManager(String path){
		jsonParser jp = new jsonParser();
		jp.makeNewJson();

		jp.addJsonKeyValue("type", RemoteFileManager_getListOfCurPath);
		jp.addJsonKeyValue("path", path);

		sendMsg(jp.getJsonData());
	}

	void requestFilebyFileManager(String path, int share){
		jsonParser jp = new jsonParser();
		jp.makeNewJson();

		jp.addJsonKeyValue("type", RemoteFileManager_requestFile);
		jp.addJsonKeyValue("path", path);

		jp.addJsonKeyValue("share", String.valueOf(share));

		sendMsg(jp.getJsonData());
	}

	public void sendMsg(String msg) {
		int port = OpelCommunicator.CMFW_DEFAULT_PORT;

		Log.d("SendMsg", msg);

		if( opelCommunicator.cmfw_send_msg(port, msg) < 0 ){
			Log.d("SendMsg", "failed");
			handle_disconnected();
			return;
		}
	}


	//Send the file on dowload folder
	public void sendDownloadFile(String fileName) {

		int port = OpelCommunicator.CMFW_DEFAULT_PORT;
		String fileSize = "";
		BufferedInputStream bis;


		File fd = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS), fileName);

		fileSize = String.valueOf(fd.length());

		int len;
		int size = 4096;
		byte[] data = new byte[size];

		sendMsg(fileName);
		sendMsg(fileSize);
		if (opelCommunicator.cmfw_send_file(port, fd) < 0) {
			//handle_disconnected();
			Log.d("OPEL", "Sendfile failed");
			return;
		}

		Log.d("OPEL", "send File size : " + fileSize);

		return;
	}
}