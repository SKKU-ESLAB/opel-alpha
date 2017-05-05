package com.opel.opel_manager.controller;

import android.bluetooth.BluetoothSocket;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.opel.cmfw.controller.CommController;
import com.opel.opel_manager.model.OPELApplication;
import com.opel.opel_manager.view.AppMarketActivity;
import com.opel.opel_manager.view.SensorViewerActivity;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.UUID;

public class OPELDevice {

    public CommController mCMFW;

    BluetoothSocket mBluetoothSocket;

    //Communication type
    private static final String INSTALLPKG = "1000";
    private static final String EXEAPP = "1001";
    private static final String EXITAPP = "1002";
    private static final String DELETEAPP = "1003";
    private static final String UPDATEAPPINFO = "1004";
    private static final String NOTI = "1005";
    private static final String NOTI_PRELOAD_IMG = "1008";

    private static final String CONFIG_REGISTER = "1009";
    private static final String CONFIG_EVENT = "1010";

    private static final String RUN_NATIVE_CAMERAVIEWER = "1011";
    private static final String RUN_NATIVE_SENSORVIEWER = "1012";
    private static final String TERM_NATIVE_CAMERAVIEWER = "2011";
    private static final String TERM_NATIVE_SENSORVIEWER = "2012";
    private static final String ANDROID_TERMINATE = "1013";

    private static final String RemoteFileManager_getListOfCurPath = "1014";
    private static final String RemoteFileManager_requestFile = "1015";
    private static final String RemoteFileManager_requestFile_Preload = "1016";

    private static final String CloudService_faceRecognition = "1017";
    private static final String CloudService_faceRecognition_preload = "1018";

    private static final String NIL_TERMINATION = "1100";
    private static final String NIL_CONFIGURATION = "1101";
    private static final String NIL_MSG_TO_SENSOR_VIEWER = "1102";

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

    private CommManagerThread myClientTask;
    private Handler handler;

    private int stat;

    private void handle_disconnected() throws NullPointerException {
        Log.d(TAG, "Disconnected");
        stat = COMM_DISCONNECTED;
        myClientTask.Cancel();
        int port = CommController.CMFW_DEFAULT_PORT;
        mCMFW.close(port);
        handler.obtainMessage(COMM_DISCONNECTED).sendToTarget();
    }

    private void connect_failed() {
        stat = COMM_DISCONNECTED;
        myClientTask.Cancel();
        int port = CommController.CMFW_DEFAULT_PORT;
        mCMFW.close(port);
        handler.obtainMessage(COMM_CONNECT_FAILED).sendToTarget();
    }

    private void handle_connected() {
        stat = COMM_CONNECTED;
        Log.d(TAG, "Connected!");
        handler.obtainMessage(COMM_CONNECTED).sendToTarget();
        requestUpdateAppInfomation();

        //Temporal bug fix
        //OPELContext.get().getCommController().requestRunNativeJSAppSensorViewer(); // [CHECK]
    }

    private void handle_connecting() {
        stat = COMM_CONNECTING;
        handler.obtainMessage(COMM_CONNECTING).sendToTarget();
    }

    private void already_connected() {
        handler.obtainMessage(COMM_ALREADY_CONNECTED).sendToTarget();
    }

    private void already_connecting() {
        handler.obtainMessage(COMM_ALREADY_CONNECTING).sendToTarget();
    }

    public OPELDevice() {
        stat = COMM_DISCONNECTED;
        mBluetoothSocket = null;
        myClientTask = null;
        handler = null;
        mCMFW = null;
    }

    public boolean isConnected() {
        if (stat == COMM_CONNECTED) {
            return true;
        } else {
            return false;
        }
    }

    public boolean isDisconnected() {
        if (stat == COMM_DISCONNECTED) {
            return true;
        } else {
            return false;
        }
    }

    public boolean isConnecting() {
        if (stat == COMM_CONNECTING) {
            return true;
        } else {
            return false;
        }
    }

    public void setOpelCommunicator() {
        mCMFW = new CommController(OPELContext.get().getWifiP2pManager(), OPELContext.get().getChannel());
    }

    public void setHandler(Handler handler) {
        this.handler = handler;
        if (myClientTask != null) myClientTask.setHandler(handler);

		/*
        if(mBluetoothSocket == null || mBluetoothSocket.isConnected() == false){
			Connect();
		}
		*/
    }

    public int getUPDATE_UI() {
        return UPDATE_UI;
    }

    public int getUPDATE_TOAST() {
        return UPDATE_TOAST;
    }


    public int getMAKE_NOTI() {
        return MAKE_NOTI;
    }

    public int getUPDATE_FILEMANAGER() {
        return UPDATE_FILEMANAGER;
    }

    public int getEXE_FILE() {
        return EXE_FILE;
    }

    public int getSHARE_FILE() {
        return SHARE_FILE;
    }

    public void Kill() {
        if (myClientTask != null) myClientTask.Cancel();
    }

    // TODO: move to CommService
    public void Connect() {
        int port = CommController.CMFW_DEFAULT_PORT;
        if (stat != COMM_DISCONNECTED) {
            Log.d(TAG, "duplicated connect");
            return;
        }
        if (handler == null) {
            Log.d(TAG, "Handler is not set");
            return;
        }
        if (mBluetoothSocket != null && mBluetoothSocket.isConnected() == true) {
            already_connected();
            return;
        } else if (mBluetoothSocket != null && stat == COMM_CONNECTING) {
            already_connecting();
            return;
        }
        handle_connecting();
        if (myClientTask != null) {
            myClientTask.Cancel();
            myClientTask = null;
        }

        if (myClientTask == null) {
            myClientTask = new CommManagerThread();
            myClientTask.setHandler(handler);
        }

        myClientTask.start();
    }

    private SensorViewerActivity mRegisteredSensorView = null;
    public void registerSensorView(SensorViewerActivity sview) {
        this.mRegisteredSensorView = sview;
    }

    public void unregisterSensorView() {
        this.mRegisteredSensorView = null;
    }

    class CommManagerThread extends Thread {
        Handler handler;
        boolean sch;

        public CommManagerThread() {
            sch = false;
        }

        public void setHandler(Handler handler) {
            this.handler = handler;


        }

        public long char2long(char c) {
            return c & 0x00000000000000FFL;
        }

        public UUID name2uuid(String str) {
            char[] inputString = str.toCharArray();
            int length = inputString.length;
            long[] longData = new long[2];
            for (int i = 0; i < 2; i++) {
                longData[i] = 0;
                if (i * 8 < length) longData[i] = char2long(inputString[i * 8]) << 56;
                if (i * 8 + 1 < length) longData[i] |= char2long(inputString[i * 8 + 1]) << 48;
                if (i * 8 + 2 < length) longData[i] |= char2long(inputString[i * 8 + 2]) << 40;
                if (i * 8 + 3 < length) longData[i] |= char2long(inputString[i * 8 + 3]) << 32;
                if (i * 8 + 4 < length) longData[i] |= char2long(inputString[i * 8 + 4]) << 24;
                if (i * 8 + 5 < length) longData[i] |= char2long(inputString[i * 8 + 5]) << 16;
                if (i * 8 + 6 < length) longData[i] |= char2long(inputString[i * 8 + 6]) << 8;
                if (i * 8 + 7 < length) longData[i] |= char2long(inputString[i * 8 + 7]);

            }

            return new UUID(longData[0], longData[1]);
        }

        public void Cancel() {
            sch = false;
        }

        @Override
        public void run() throws NullPointerException {

            sch = true;
            int port = CommController.CMFW_DEFAULT_PORT;
			/* Bluetooth Connection */
            if (mCMFW.connect(port) == false) {
                connect_failed();
                return;
            } else {
                Log.d("BTConnect", "Success");
                handle_connected();
            }

            while (sch) {
                String rcvJson = rcvMsg();
                if (rcvJson.equals("")) {
                    continue;
                }
                Log.d("BTConnect", "Received message: " + rcvJson);
                JSONParser jp = new JSONParser(rcvJson);

                String req = jp.getValueByKey("type");

                if (req.equals(INSTALLPKG)) {
                    handleInstall(jp);
                } else if (req.equals(EXEAPP)) {
                    handleStart(jp);
                } else if (req.equals(EXITAPP)) {
                    handleTermination(jp);
                } else if (req.equals(DELETEAPP)) {
                    handleUninstall(jp);
                } else if (req.equals(UPDATEAPPINFO)) {
                    if (OPELContext.get().getIsLoading() == false) {

                        handleUpdateAppInfomation(jp);
                        OPELContext.get().setIsLoading(true);

						/*while(!MainActivity.mainLoadingProgDialog.isShowing()) {
							MainActivity.mainLoadingProgDialog.dismiss();
						}*/
                    }
                } else if (req.equals(NOTI)) {

                    if (jp.getValueByKey("isNoti").equals("1")) handleEventWithNoti(jp);
                    else handleEventWithoutNoti(jp);


                } else if (req.equals(NOTI_PRELOAD_IMG)) {
                    String str = handleNotiPreLoadImg(jp);
                    Log.d("OPEL", "NOTI PRELOAD COMPLETE FILENAME: " + str);
                } else if (req.equals(NIL_TERMINATION)) {

                    String appID = jp.getValueByKey("appID");
                    OPELContext.get().getAppList().getApp(appID).setTerminationJson(jp.getJsonData());

                    Log.d("OPEL", "NIL_TERMINATION :: " + jp.getJsonData());
                } else if (req.equals(NIL_MSG_TO_SENSOR_VIEWER)) {
                    onMsgToSensorViewer(rcvJson);

                    Log.d("OPEL", "NIL_MSG_TO_SENSOR_VIEWER :: " + jp.getJsonData());
                } else if (req.equals(CONFIG_REGISTER)) {

                    handleRegisterConfig(jp);
                } else if (req.equals(RemoteFileManager_getListOfCurPath)) {

                    handleUpdateFileManager(jp);
                } else if (req.equals(RemoteFileManager_requestFile)) {

                    handleRequestFileManager_requestFile(jp);
                } else if (req.equals(RemoteFileManager_requestFile_Preload)) {
                    Log.d("OPEL", "Preload for fileManager  : " + jp.getValueByKey("img"));
                    String fileName = handleRequestFileManager_requestFile_Preload(jp);

                } else if (req.equals(CloudService_faceRecognition)) {
                    Log.d("OPEL", "CloudService_faceRecognition for cloud service ");
                    handleCloudService(jp);

                } else if (req.equals(CloudService_faceRecognition_preload)) {

                    String fileName = handleCloudService_File_Preload(jp);
                    Log.d("OPEL", "Preload for cloud service  : " + fileName);
                } else {
                    Log.d("OPEL", "Unexpected responce/request  :  " + req);
                }
            }

            if (mBluetoothSocket != null && mBluetoothSocket.isConnected()) {
                try {
                    mBluetoothSocket.close();
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
            handle_disconnected();
        }

        void onMsgToSensorViewer(String message) {
            if(mRegisteredSensorView != null) {
                mRegisteredSensorView.onMsgToSensorViewer(message);
            }
        }


        void handleInstall(JSONParser jp) {
            // getAppID
            // appList.installApplication(app);

//				Move to mBluetoothSocket listener when the opk file is sent completely
/*						applicationList list = OPELContext.getmAllAppList();
			list.add(new OPELApplication("1",  title, BitmapFactory.decodeResource(getResources(), R.drawable.app), 1));
			MainActivity.updateDisplayItem();
*/

            String appID = jp.getValueByKey("appID");
            String appName = jp.getValueByKey("appName");

            String fName = rcvFile(OPELContext.get().getIconDirectoryPath(), jp);
            if (fName.equals("")) return;

            Bitmap bitmap = null;
            File f = new File(OPELContext.get().getIconDirectoryPath(), fName);
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inPreferredConfig = Bitmap.Config.ARGB_8888;

            try {
                bitmap = BitmapFactory.decodeStream(new FileInputStream(f), null, options);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }

            OPELContext.get().getAppList().installApplication(new OPELApplication(appID, appName, bitmap, 0));

            //AppMarketActivity release
            AppMarketActivity.marketProgDialog.dismiss();
            updateMainUIThread(UPDATE_UI);
            updateMainUIThread(UPDATE_TOAST, "Complete the installation : " + appName);

            try {
                File downloaddir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
                File pkgFile = new File(downloaddir, jp.getValueByKey("pkgFileName"));
                pkgFile.delete();
            } catch (Exception e) {
                e.printStackTrace();
            }

        }

        //{type:UPDATEAPPINFO, appID/Type:appName, .....}
        //get all information : appID, appName, type (bitmap)

        void handleUpdateAppInfomation(JSONParser jp) {

            while (jp.hasMoreValue()) {
                String ret[] = new String[2];
                ret = jp.getNextKeyValue();

                if (ret[0].equals("type") && ret[1].equals(UPDATEAPPINFO)) {
                    continue;
                }
                if (ret[0].equals("IP_ADDR__a")) {
                    OPELContext.get().setDeviceIP(ret[1]);
                    continue;
                }

                if (ret[0].equals("OPEL_DATA_DIR")) {
                    OPELContext.get().setOpelDataDir(ret[1]);
                    continue;
                }

                // [MORE] If the icon img file doesn't exist, need to request to OPEL device
                Bitmap bitmap = null;

                String fileName = ret[0].substring(0, ret[0].length() - 2) + ".icon";
                File f = new File(OPELContext.get().getIconDirectoryPath(), fileName);

                BitmapFactory.Options options = new BitmapFactory.Options();
                options.inPreferredConfig = Bitmap.Config.ARGB_8888;

                try {
                    bitmap = BitmapFactory.decodeStream(new FileInputStream(f), null, options);
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }

                String appID = ret[0].substring(0, ret[0].length() - 2);
                int type = Integer.parseInt(ret[0].substring(ret[0].length() - 1, ret[0].length()));

                OPELContext.get().getAppList().add(new OPELApplication(appID, ret[1], bitmap, type));
            }

            updateMainUIThread(UPDATE_UI);
        }

        void handleUninstall(JSONParser jp) {
            String appID = jp.getValueByKey("appID");

            OPELApplication deleteTargetApp = OPELContext.get().getAppList().getApp(appID);

            //Remove the item of app list
            OPELContext.get().getAppList().uninstallApplication(deleteTargetApp);

            //Remove the icon file
            String fileName = appID + ".icon";
            File f = new File(OPELContext.get().getIconDirectoryPath(), fileName);
            f.delete();

            updateMainUIThread(UPDATE_UI);
            updateMainUIThread(UPDATE_TOAST, "Complete the uninstallation");
        }

        //{type:EXEAPP, appID:id, .....}
        void handleStart(JSONParser jp) {

            String appID = jp.getValueByKey("appID");
            OPELContext.get().getAppList().getApp(appID).setTypeToRunning();
            updateMainUIThread(UPDATE_UI);
            updateMainUIThread(UPDATE_TOAST, "Run OPELApplication : " + OPELContext.get().getAppList().getApp(appID).getTitle());
        }

        //{type:EXIXAPP, appID:id, .....}
        void handleTermination(JSONParser jp) {
            Log.d("OPEL", "handTermination > json : " + jp.getJsonData());
            String appID = jp.getValueByKey("appID");
            OPELContext.get().getAppList().getApp(appID).setTypeToInstalled();
            updateMainUIThread(UPDATE_UI);

            //set termination js to N/A
        }

        void handleRegisterConfig(JSONParser jp) {

            String appID = jp.getValueByKey("appID");
            OPELContext.get().getAppList().getApp(appID).setConfigJson(jp.getJsonData());
        }

        void handleEventWithNoti(JSONParser jp) {

            OPELContext.get().getEventList().addEvent(jp.getValueByKey
                    ("appID"), jp.getValueByKey("appTitle"), jp.getValueByKey
                    ("description"), jp.getValueByKey("dateTime"), jp.getJsonData());

            updateMainUIThread(MAKE_NOTI, jp.getJsonData());
            updateMainUIThread(UPDATE_UI);
        }

        void handleEventWithoutNoti(JSONParser jp) {

            OPELContext.get().getEventList().addEvent(jp.getValueByKey
                    ("appID"), jp.getValueByKey("appTitle"), jp.getValueByKey
                    ("description"), jp.getValueByKey("dateTime"), jp.getJsonData());
            updateMainUIThread(UPDATE_UI);
        }

        String handleNotiPreLoadImg(JSONParser jp) {

            return rcvFile(OPELContext.get().getRUIStoragePath(), jp);

        }

        void handleConfigSetting(JSONParser jp) {

        }

        void handleUpdateFileManager(JSONParser jp) {
            updateMainUIThread(UPDATE_FILEMANAGER, jp);
        }

        void handleRequestFileManager_requestFile(JSONParser jp) {
            String fileName = jp.getValueByKey("filename");
            if (jp.getValueByKey("share").equals("0")) {
                updateMainUIThread(EXE_FILE, jp);
            } else if (jp.getValueByKey("share").equals("1")) {
                updateMainUIThread(SHARE_FILE, jp);
            }

        }

        String handleRequestFileManager_requestFile_Preload(JSONParser jp) {
            return rcvFile(OPELContext.get().getRemoteStorageStoragePath(), jp);
        }

        void handleCloudService(JSONParser jp) {

            int fileSize;
            BufferedInputStream bis;
            try {

                File fd = new File(OPELContext.get().getCloudStoragePath(), jp.getValueByKey("img"));
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

                OPELContext.get().getMQTTManager().mqttSendFile("opel/img", res);

            } catch (Exception e) {
                e.printStackTrace();
                Log.d("OPEL", "Exception: MQTT Send file exception");

                return;
            }

        }


        String handleCloudService_File_Preload(JSONParser jp) {
            return rcvFile(OPELContext.get().getCloudStoragePath(), jp);
        }

        private void updateMainUIThread(int what) {
            Message msg = Message.obtain();
            msg.what = what;

            handler.sendMessage(msg);
        }

        private void updateMainUIThread(int what, Object obj) {

            Message msg = Message.obtain();
            msg.what = what;
            msg.obj = obj;

            handler.sendMessage(msg);
        }
    }


    public void requestInstall(String fileName) {
        // send file & msg

        JSONParser jp = new JSONParser();
        jp.makeNewJson();
        jp.addJsonKeyValue("type", INSTALLPKG);
        jp.addJsonKeyValue("pkgFileName", fileName);

        sendMsg(jp.getJsonData());
        sendFile(fileName);
    }

    public void requestUninstall(String appID) {
        JSONParser jp = new JSONParser();
        jp.makeNewJson();
        jp.addJsonKeyValue("type", DELETEAPP);
        jp.addJsonKeyValue("appID", appID);

        sendMsg(jp.getJsonData());
    }

    public void requestStart(String appID, String appName) {
        JSONParser jp = new JSONParser();
        jp.makeNewJson();
        jp.addJsonKeyValue("type", EXEAPP);
        jp.addJsonKeyValue("appID", appID);
        jp.addJsonKeyValue("appName", appName);
        sendMsg(jp.getJsonData());
    }

    public void requestTermination(String appID) {

        String terminationJson = OPELContext.get().getAppList().getApp(appID).getTerminationJson();
        JSONParser jp = new JSONParser(terminationJson);
        Log.d("OPEL", "REQ Termination json : " + terminationJson);

        JSONParser sendJP = new JSONParser();
        sendJP.makeNewJson();
        sendJP.addJsonKeyValue("type", EXITAPP);
        sendJP.addJsonKeyValue("appID", appID);
        sendJP.addJsonKeyValue("appName", jp.getValueByKey("appName"));
        sendJP.addJsonKeyValue("rqID", jp.getValueByKey("rqID"));
        sendJP.addJsonKeyValue("pid", jp.getValueByKey("pid"));

        sendMsg(sendJP.getJsonData());
    }

    public void requestTermination() {

        JSONParser sendJP = new JSONParser();

        sendJP.makeNewJson();
        sendJP.addJsonKeyValue("type", ANDROID_TERMINATE);
        Log.d("OPEL", "Termination json : " + sendJP);
        sendMsg(sendJP.getJsonData());
    }


    public void requestUpdateAppInfomation() {

        JSONParser jp = new JSONParser();
        jp.makeNewJson();
        jp.addJsonKeyValue("type", UPDATEAPPINFO);

        sendMsg(jp.getJsonData());
    }

    public void requestConfigSetting(JSONParser _jp) {

        _jp.addJsonKeyValue("type", CONFIG_EVENT);

        sendMsg(_jp.getJsonData());
    }

    public void requestRunNativeJSAppCameraViewer() {
        JSONParser jp = new JSONParser();
        jp.makeNewJson();

        jp.addJsonKeyValue("type", RUN_NATIVE_CAMERAVIEWER);


        sendMsg(jp.getJsonData());
    }

    public void requestRunNativeJSAppSensorViewer() {
        JSONParser jp = new JSONParser();
        jp.makeNewJson();

        jp.addJsonKeyValue("type", RUN_NATIVE_SENSORVIEWER);


        sendMsg(jp.getJsonData());
    }

    public void requestTermNativeJSAppCameraViewer() {
        JSONParser jp = new JSONParser();
        jp.makeNewJson();

        jp.addJsonKeyValue("type", TERM_NATIVE_CAMERAVIEWER);


        sendMsg(jp.getJsonData());
    }

    public void requestTermNativeJSAppSensorViewer() {
        JSONParser jp = new JSONParser();
        jp.makeNewJson();

        jp.addJsonKeyValue("type", TERM_NATIVE_SENSORVIEWER);

        sendMsg(jp.getJsonData());
    }

    public void requestUpdateFileManager(String path) {
        JSONParser jp = new JSONParser();
        jp.makeNewJson();

        jp.addJsonKeyValue("type", RemoteFileManager_getListOfCurPath);
        jp.addJsonKeyValue("path", path);

        sendMsg(jp.getJsonData());
    }

    public void requestFilebyFileManager(String path, int share) {
        JSONParser jp = new JSONParser();
        jp.makeNewJson();

        jp.addJsonKeyValue("type", RemoteFileManager_requestFile);
        jp.addJsonKeyValue("path", path);

        jp.addJsonKeyValue("share", String.valueOf(share));

        sendMsg(jp.getJsonData());
    }
}