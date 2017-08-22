package com.opel.opel_manager.controller;

import android.bluetooth.BluetoothSocket;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.opel.cmfw.service.CommChannelService;
import com.opel.opel_manager.model.OPELApplication;
import com.opel.opel_manager.view.AppMarketActivity;
import com.opel.opel_manager.view.SensorViewerActivity;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;

import static com.opel.opel_manager.controller.OPELContext.getAppList;

public class LegacyAppCoreStub {
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

    //UI Handling type on Handler of MainUIThread
    public static final int HANDLER_UPDATE_UI = 3000;
    public static final int HANDLER_UPDATE_TOAST = 3001;
    public static final int HANDLER_MAKE_NOTI = 3002;
    public static final int HANDLER_UPDATE_FILEMANAGER = 3003;
    public static final int HANDLER_EXE_FILE = 3004;
    public static final int HANDLER_SHARE_FILE = 3005;

    // MainActivity-dependent part
    private Handler mMainUIHandler;
    private CommChannelService mCommChannelService;

    public LegacyAppCoreStub() {
        mBluetoothSocket = null;
        mMainUIHandler = null;
    }

    public void setMainUIHandler(Handler mainUIHandler) {
        this.mMainUIHandler = mainUIHandler;
    }

    public void setCommService(CommChannelService commChannelService) {
        this.mCommChannelService = commChannelService;
    }

    private SensorViewerActivity mRegisteredSensorView = null;

    public void registerSensorView(SensorViewerActivity sview) {
        this.mRegisteredSensorView = sview;
    }

    public void unregisterSensorView() {
        this.mRegisteredSensorView = null;
    }

    public void onReceivedMessage(String message, String filePath) {
        if (message.equals("")) {
            return;
        }
        Log.d(TAG, "Received message: " + message);
        LegacyJSONParser jp = new LegacyJSONParser(message);

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
            handleUpdateAppInfomation(jp);
        } else if (req.equals(NOTI)) {
            if (jp.getValueByKey("isNoti").equals("1")) {
                handleEventWithNoti(jp);
            } else {
                handleEventWithoutNoti(jp);
            }
        } else if (req.equals(NOTI_PRELOAD_IMG)) {
            handleNotiPreLoadImg(jp);
        } else if (req.equals(NIL_TERMINATION)) {
            String appID = jp.getValueByKey("appID");
            getAppList().getApp(appID).setTerminationJson(jp.getJsonData());
        } else if (req.equals(NIL_MSG_TO_SENSOR_VIEWER)) {
            handleMsgToSensorViewer(message);
        } else if (req.equals(CONFIG_REGISTER)) {
            handleRegisterConfig(jp);
        } else if (req.equals(RemoteFileManager_getListOfCurPath)) {
            handleUpdateFileManager(jp);
        } else if (req.equals(RemoteFileManager_requestFile)) {
            handleRequestFileManager_requestFile(jp);
        } else if (req.equals(RemoteFileManager_requestFile_Preload)) {
            handleRequestFileManager_requestFile_Preload(jp);
        } else if (req.equals(CloudService_faceRecognition)) {
            handleCloudService(jp);
        } else if (req.equals(CloudService_faceRecognition_preload)) {
            handleCloudService_File_Preload(jp);
        } else {
            Log.d(TAG, "Unexpected response/request:  " + req);
        }
    }

    void handleMsgToSensorViewer(String message) {
        if (mRegisteredSensorView != null) {
            mRegisteredSensorView.onMsgToSensorViewer(message);
        }
    }

    void handleInstall(LegacyJSONParser jp) {
        // TODO: remake form the bottom
        String appID = jp.getValueByKey("appID");
        String appName = jp.getValueByKey("appName");

        String fName = rcvFile(OPELContext.getSettings().getIconDir(), jp);
        if (fName.equals("")) return;

        Bitmap bitmap = null;
        File f = new File(OPELContext.getSettings().getIconDir(), fName);
        BitmapFactory.Options options = new BitmapFactory.Options();
        options.inPreferredConfig = Bitmap.Config.ARGB_8888;

        try {
            bitmap = BitmapFactory.decodeStream(new FileInputStream(f), null, options);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        OPELContext.getAppList().installApplication(new OPELApplication(appID, appName, bitmap, 0));

        //AppMarketActivity release
        AppMarketActivity.marketProgDialog.dismiss();
        updateMainUIThread(HANDLER_UPDATE_UI);
        updateMainUIThread(HANDLER_UPDATE_TOAST, "Complete the installation:" + " " + appName);

        try {
            File downloaddir = Environment.getExternalStoragePublicDirectory(Environment
                    .DIRECTORY_DOWNLOADS);
            File pkgFile = new File(downloaddir, jp.getValueByKey("pkgFileName"));
            pkgFile.delete();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    //{type:UPDATEAPPINFO, appID/Type:appName, .....}
    //get all information : appID, appName, type (bitmap)
    void handleUpdateAppInfomation(LegacyJSONParser jp) {
        if (OPELContext.isAppInfoLoading() == true) return;

        while (jp.hasMoreValue()) {
            String ret[] = new String[2];
            ret = jp.getNextKeyValue();

            if (ret[0].equals("type") && ret[1].equals(UPDATEAPPINFO)) {
                continue;
            } else if (ret[0].compareTo("OPEL_DATA_DIR") == 0) {
                OPELContext.getSettings().setOpelDataDir(ret[1]);
                continue;
            }

            // [MORE] If the icon img file doesn't exist, need to request to
            // OPEL device
            Bitmap bitmap = null;

            String fileName = ret[0].substring(0, ret[0].length() - 2) + "" + ".icon";
            File f = new File(OPELContext.getSettings().getIconDir(), fileName);

            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inPreferredConfig = Bitmap.Config.ARGB_8888;

            try {
                bitmap = BitmapFactory.decodeStream(new FileInputStream(f), null, options);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }

            String appID = ret[0].substring(0, ret[0].length() - 2);
            int type = Integer.parseInt(ret[0].substring(ret[0].length() - 1, ret[0].length()));

            OPELContext.getAppList().add(new OPELApplication(appID, ret[1], bitmap, type));
        }
        updateMainUIThread(HANDLER_UPDATE_UI);
        OPELContext.setIsAppInfoLoading(true);
    }

    void handleUninstall(LegacyJSONParser jp) {
        String appID = jp.getValueByKey("appID");

        OPELApplication deleteTargetApp = OPELContext.getAppList().getApp(appID);

        //Remove the item of app list
        OPELContext.getAppList().uninstallApplication(deleteTargetApp);

        //Remove the icon file
        String fileName = appID + ".icon";
        File f = new File(OPELContext.getSettings().getIconDir(), fileName);
        f.delete();

        updateMainUIThread(HANDLER_UPDATE_UI);
        updateMainUIThread(HANDLER_UPDATE_TOAST, "Complete the uninstallation");
    }

    //{type:EXEAPP, appID:id, .....}
    void handleStart(LegacyJSONParser jp) {

        String appID = jp.getValueByKey("appID");
        OPELContext.getAppList().getApp(appID).setTypeToRunning();
        updateMainUIThread(HANDLER_UPDATE_UI);
        updateMainUIThread(HANDLER_UPDATE_TOAST, "Run OPELApplication : " + OPELContext
                .getAppList().getApp(appID).getTitle());
    }

    //{type:EXIXAPP, appID:id, .....}
    void handleTermination(LegacyJSONParser jp) {
        Log.d("OPEL", "handTermination > json : " + jp.getJsonData());
        String appID = jp.getValueByKey("appID");
        OPELContext.getAppList().getApp(appID).setTypeToInstalled();
        updateMainUIThread(HANDLER_UPDATE_UI);

        //set termination js to N/A
    }

    void handleRegisterConfig(LegacyJSONParser jp) {

        String appID = jp.getValueByKey("appID");
        OPELContext.getAppList().getApp(appID).setConfigJson(jp.getJsonData());
    }

    void handleEventWithNoti(LegacyJSONParser jp) {

        OPELContext.getEventList().addEvent(jp.getValueByKey("appID"), jp.getValueByKey
                ("appTitle"), jp.getValueByKey("description"), jp.getValueByKey("dateTime"), jp
                .getJsonData());

        updateMainUIThread(HANDLER_MAKE_NOTI, jp.getJsonData());
        updateMainUIThread(HANDLER_UPDATE_UI);
    }

    void handleEventWithoutNoti(LegacyJSONParser jp) {
        OPELContext.getEventList().addEvent(jp.getValueByKey("appID"), jp.getValueByKey
                ("appTitle"), jp.getValueByKey("description"), jp.getValueByKey("dateTime"), jp
                .getJsonData());
        updateMainUIThread(HANDLER_UPDATE_UI);
    }

    String handleNotiPreLoadImg(LegacyJSONParser jp) {
        // TODO: remake from the bottom
//        if(this.mService == null) return "";
//        String res = this.mService.rcvFile(OPELContext.getSettings()
//                .getRemoteUIDir(), jp);
//        return res;
        return "";
    }

    void handleUpdateFileManager(LegacyJSONParser jp) {
        updateMainUIThread(HANDLER_UPDATE_FILEMANAGER, jp);
    }

    void handleRequestFileManager_requestFile(LegacyJSONParser jp) {
        String fileName = jp.getValueByKey("filename");
        if (jp.getValueByKey("share").equals("0")) {
            updateMainUIThread(HANDLER_EXE_FILE, jp);
        } else if (jp.getValueByKey("share").equals("1")) {
            updateMainUIThread(HANDLER_SHARE_FILE, jp);
        }

    }

    String handleRequestFileManager_requestFile_Preload(LegacyJSONParser jp) {
        // TODO: remake from the bottom
//        return rcvFile(OPELContext.getSettings().getRemoteStorageDir(), jp);
        return "";
    }

    void handleCloudService(LegacyJSONParser jp) {

        int fileSize;
        BufferedInputStream bis;
        try {

            File fd = new File(OPELContext.getSettings().getCloudDir(), jp.getValueByKey("img"));
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

            OPELContext.getMQTTController().mqttSendFile("opel/img", res);

        } catch (Exception e) {
            e.printStackTrace();
            Log.d("OPEL", "Exception: MQTT Send file exception");

            return;
        }

    }


    String handleCloudService_File_Preload(LegacyJSONParser jp) {
        // TODO: remake from the bottom
        //return rcvFile(OPELContext.getSettings().getCloudDir(), jp);
        return null;
    }

    private void updateMainUIThread(int what) {
        Message msg = Message.obtain();
        msg.what = what;

        if (mMainUIHandler != null) {
            mMainUIHandler.sendMessage(msg);
        }
    }

    private void updateMainUIThread(int what, Object obj) {

        Message msg = Message.obtain();
        msg.what = what;
        msg.obj = obj;

        if (mMainUIHandler != null) {
            mMainUIHandler.sendMessage(msg);
        }
    }


    public void requestInstall(String fileName) {
        // InstallApp (int appId + file attachment)
        // send file & msg

        LegacyJSONParser jp = new LegacyJSONParser();
        jp.makeNewJson();
        jp.addJsonKeyValue("type", INSTALLPKG);
        jp.addJsonKeyValue("pkgFileName", fileName);

        File file = new File(fileName);
        this.mCommChannelService.sendRawMessage(jp.getJsonData(), file);
    }

    public void requestUninstall(String appID) {
        // RemoveApp (int appId)
        LegacyJSONParser jp = new LegacyJSONParser();
        jp.makeNewJson();
        jp.addJsonKeyValue("type", DELETEAPP);
        jp.addJsonKeyValue("appID", appID);

        this.mCommChannelService.sendRawMessage(jp.getJsonData());
    }

    public void requestStart(String appID, String appName) {
        // LaunchApp (int appId)
        LegacyJSONParser jp = new LegacyJSONParser();
        jp.makeNewJson();
        jp.addJsonKeyValue("type", EXEAPP);
        jp.addJsonKeyValue("appID", appID);
        jp.addJsonKeyValue("appName", appName);
        this.mCommChannelService.sendRawMessage(jp.getJsonData());
    }

    public void requestTermination(String appID) {
        // TerminateApp (int appId)
        String terminationJson = OPELContext.getAppList().getApp(appID).getTerminationJson();
        LegacyJSONParser jp = new LegacyJSONParser(terminationJson);
        Log.d("OPEL", "REQ Termination json : " + terminationJson);

        LegacyJSONParser sendJP = new LegacyJSONParser();
        sendJP.makeNewJson();
        sendJP.addJsonKeyValue("type", EXITAPP);
        sendJP.addJsonKeyValue("appID", appID);
        sendJP.addJsonKeyValue("appName", jp.getValueByKey("appName"));
        sendJP.addJsonKeyValue("rqID", jp.getValueByKey("rqID"));
        sendJP.addJsonKeyValue("pid", jp.getValueByKey("pid"));

        this.mCommChannelService.sendRawMessage(sendJP.getJsonData());
    }

    public void requestTermination() {
        // deprecated
        LegacyJSONParser sendJP = new LegacyJSONParser();

        sendJP.makeNewJson();
        sendJP.addJsonKeyValue("type", ANDROID_TERMINATE);
        Log.d("OPEL", "Termination json : " + sendJP);
        this.mCommChannelService.sendRawMessage(sendJP.getJsonData());
    }


    public void requestUpdateAppInfomation() {
        // GetAppList (void)
        LegacyJSONParser jp = new LegacyJSONParser();
        jp.makeNewJson();
        jp.addJsonKeyValue("type", UPDATEAPPINFO);

        this.mCommChannelService.sendRawMessage(jp.getJsonData());
    }

    public void requestConfigSetting(LegacyJSONParser _jp) {
        // UpdateAppConfig (string legacyData)
        _jp.addJsonKeyValue("type", CONFIG_EVENT);

        this.mCommChannelService.sendRawMessage(_jp.getJsonData());
    }

    public void requestRunNativeJSAppCameraViewer() {
        // deprecated
        LegacyJSONParser jp = new LegacyJSONParser();
        jp.makeNewJson();

        jp.addJsonKeyValue("type", RUN_NATIVE_CAMERAVIEWER);


        this.mCommChannelService.sendRawMessage(jp.getJsonData());
    }

    public void requestRunNativeJSAppSensorViewer() {
        // deprecated
        LegacyJSONParser jp = new LegacyJSONParser();
        jp.makeNewJson();

        jp.addJsonKeyValue("type", RUN_NATIVE_SENSORVIEWER);


        this.mCommChannelService.sendRawMessage(jp.getJsonData());
    }

    public void requestTermNativeJSAppCameraViewer() {
        // deprecated
        LegacyJSONParser jp = new LegacyJSONParser();
        jp.makeNewJson();

        jp.addJsonKeyValue("type", TERM_NATIVE_CAMERAVIEWER);


        this.mCommChannelService.sendRawMessage(jp.getJsonData());
    }

    public void requestTermNativeJSAppSensorViewer() {
        // deprecated
        LegacyJSONParser jp = new LegacyJSONParser();
        jp.makeNewJson();

        jp.addJsonKeyValue("type", TERM_NATIVE_SENSORVIEWER);

        this.mCommChannelService.sendRawMessage(jp.getJsonData());
    }

    public void requestUpdateFileManager(String path) {
        // GetFileList (string path)
        LegacyJSONParser jp = new LegacyJSONParser();
        jp.makeNewJson();

        jp.addJsonKeyValue("type", RemoteFileManager_getListOfCurPath);
        jp.addJsonKeyValue("path", path);

        this.mCommChannelService.sendRawMessage(jp.getJsonData());
    }

    public void requestFilebyFileManager(String path, int share) {
        // GetFile (string path)
        LegacyJSONParser jp = new LegacyJSONParser();
        jp.makeNewJson();

        jp.addJsonKeyValue("type", RemoteFileManager_requestFile);
        jp.addJsonKeyValue("path", path);

        jp.addJsonKeyValue("share", String.valueOf(share)); // deprecated

        this.mCommChannelService.sendRawMessage(jp.getJsonData());
    }
}