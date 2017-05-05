package com.opel.opel_manager.view;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.NotificationCompat;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.opel.cmfw.controller.CommController;
import com.opel.cmfw.view.BluetoothDeviceSettingActivity;
import com.opel.cmfw.view.CommBroadcastReceiver;
import com.opel.cmfw.view.CommEventListener;
import com.opel.cmfw.view.CommService;
import com.opel.opel_manager.R;
import com.opel.opel_manager.controller.JSONParser;
import com.opel.opel_manager.controller.OPELContext;
import com.opel.opel_manager.controller.OPELDevice;
import com.opel.opel_manager.model.OPELAppList;
import com.opel.opel_manager.model.OPELApplication;

import java.io.File;
import java.util.ArrayList;
import java.util.Set;

import static com.opel.opel_manager.controller.OPELContext.getAppList;

public class MainActivity extends Activity implements CommEventListener {
    // RPC on CommService
    private CommService mCommService;
    private CommBroadcastReceiver mCommBroadcastReceiver;
    private boolean mIsCommServiceBound = false;
    private ServiceConnection mCommServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder
                inputBinder) {
            CommService.CommBinder serviceBinder = (CommService.CommBinder)
                    inputBinder;
            mCommService = serviceBinder.getService();
            mIsCommServiceBound = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            mIsCommServiceBound = false;
        }
    };

    // IconGridView
    private GridView mIconGridView;
    private IconListAdapter mIconListAdapter;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Initialize UI Layout
        setContentView(R.layout.activity_main);

        // Check Permission
        this.checkStoragePermission();

        // TODO: move to Settings
        // Initialize storage space
        this.initStorageWorkspace();

        // TODO: move to MainController
        // Initialize "only once on launch"
        if (!OPELContext.getManagerState().isInit()) {
            // Add native applications to AppList
            // TODO: move to AppList
            OPELContext.getAppList().addNativeAppList(this.getResources());

            // Restore EventList
            // TODO: move to EventList
            OPELContext.getEventList().open(getApplicationContext());

            OPELContext.getManagerState().initComplete();
        }

        // Initialize UI Contents
        //Create IconListAdapter
        mIconGridView = (GridView) findViewById(R.id.iconGridView);
        OPELAppList list = getAppList();
        mIconListAdapter = new IconListAdapter(this, R.layout
                .template_gridview_icon_main, list.getList());
        mIconGridView.setAdapter(mIconListAdapter);
        mIconGridView.setOnItemClickListener(mItemClickListener);
        mIconGridView.setOnItemLongClickListener(mItemLongClickListener);

        // Launch CommService for setting connection with target OPEL device.
        Intent serviceIntent = new Intent(this, CommService.class);
        bindService(serviceIntent, this.mCommServiceConnection, Context
                .BIND_AUTO_CREATE);
        this.mCommService.initializeConnection(); // RPC to CommService

        // Set CommBroadcastReceiver and CommEventListener
        IntentFilter broadcastIntentFilter = new IntentFilter();
        broadcastIntentFilter.addAction(CommBroadcastReceiver.ACTION);
        this.mCommBroadcastReceiver = new CommBroadcastReceiver(this);
        this.registerReceiver(this.mCommBroadcastReceiver, broadcastIntentFilter);
    }

    private void checkStoragePermission() {
        // Check storage read/write permission
        if (this.checkSelfPermission(Manifest.permission
                .READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            Log.d("OPEL", "Requesting Permission");

            if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                    Manifest.permission.READ_EXTERNAL_STORAGE)) {
                Toast.makeText(this, "Storage permission is required to run "
                        + "OPEL Manager.", Toast.LENGTH_SHORT).show();
            }
            ActivityCompat.requestPermissions(this, new String[]{Manifest
                    .permission.READ_EXTERNAL_STORAGE, Manifest.permission
                    .WRITE_EXTERNAL_STORAGE}, 1);
        } else {
            Log.d("OPEL", "Permission granted");
        }
    }

    public void onRequestPermissionsResult(int requestCode, String
            permissions[], int[] grantResults) {
        if (grantResults.length <= 0 || grantResults[0] != PackageManager
                .PERMISSION_GRANTED) {
            Toast.makeText(this, "Failed to be granted", Toast.LENGTH_LONG)
                    .show();
            this.finish();
        }
    }

    // WifiDirectListener
    // TODO: convert to CommEventListener
    public void onWifiDirectStateChanged(boolean isOn) {
        this.setIndicatorWFD(isOn);
    }

    protected void onRestart() {
        super.onRestart();
        this.updateDisplayItem();
    }

    protected void onPause() {
        super.onPause();
        this.updateDisplayItem();
        // TODO
    }

    protected void onResume() {
        super.onResume();
        this.updateDisplayItem();
        // TODO

    }

    protected void onDestroy() {
        super.onDestroy();
        unbindService(this.mCommServiceConnection);

        // TODO
        OPELContext.get().exitApp();
    }

    // IconGridView
    public void updateDisplayItem() {
        mIconListAdapter.updateDisplay();
    }

    private GridView.OnItemClickListener mItemClickListener = new GridView
            .OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> arg0, View v, int position,
                                long arg3) {
            AlertDialog.Builder alertDlg = new AlertDialog.Builder
                    (MainActivity.this);
            OPELAppList appList = getAppList();
            OPELDevice opelDevice = OPELContext.getCommController();

            // Native Menu Handling
            int appType = appList.getList().get(position).getType();
            String appTitle = appList.getList().get(position).getTitle();
            if (appType == -1) {
                if (appTitle.compareTo("App Manager") == 0) {
                    if (opelDevice.isDisconnected()) {
                        Toast.makeText(getApplicationContext(),
                                "Disconnected" + " to OPEL", Toast
                                        .LENGTH_SHORT).show();
                    }
                    Intent intent = new Intent(MainActivity.this,
                            AppManagerActivity.class);
                    startActivity(intent);
                } else if (appTitle.compareTo("App Market") == 0) {
                    if (opelDevice.isDisconnected()) {
                        Toast.makeText(getApplicationContext(),
                                "Disconnected" + " to OPEL", Toast
                                        .LENGTH_SHORT).show();
                    }
                    Intent intent = new Intent(MainActivity.this,
                            AppMarketActivity.class);
                    startActivity(intent);
                } else if (appTitle.compareTo("Connect") == 0) {
                    if (opelDevice.isDisconnected()) {
                        boolean found = false;
                        BluetoothAdapter ba = BluetoothAdapter
                                .getDefaultAdapter();

                        Set<BluetoothDevice> bds = ba.getBondedDevices();

                        if (bds.size() > 0) {
                            for (BluetoothDevice tmpDevice : bds) {
                                if (tmpDevice.getName().contains
                                        (CommController.getTargetBtName())) {
                                    found = true;
                                    break;
                                }
                            }
                        }

                        if (!found) {
                            Intent serverIntent = new Intent(MainActivity
                                    .this, BluetoothDeviceSettingActivity
                                    .class);
                            startActivityForResult(serverIntent, 2);
                        } else {
                            OPELContext.getCommController().Connect();
                        }
                    } else
                        Toast.makeText(getApplicationContext(), "Not " +
                                "disconnected", Toast.LENGTH_SHORT).show();
                } else if (appTitle.compareTo("File Manager") == 0) {
                    if (opelDevice.isDisconnected()) {
                        Toast.makeText(getApplicationContext(),
                                "Disconnected" + " to OPEL", Toast
                                        .LENGTH_SHORT).show();
                    }


                    Intent intent = new Intent(MainActivity.this,
                            FileManagerActivity.class);
                    startActivity(intent);

                } else if (appTitle.compareTo("Camera") == 0) {
                    if (opelDevice.isDisconnected()) {
                        Toast.makeText(getApplicationContext(),
                                "Disconnected" + " to OPEL", Toast
                                        .LENGTH_SHORT).show();
                    }
                    Intent intent = new Intent(MainActivity.this,
                            CameraViewerActivity.class);
                    startActivity(intent);

                } else if (appTitle.compareTo("Sensor") == 0) {
                    if (opelDevice.isDisconnected()) {
                        Toast.makeText(getApplicationContext(),
                                "Disconnected" + " to OPEL", Toast
                                        .LENGTH_SHORT).show();
                    }
                    Intent intent = new Intent(MainActivity.this,
                            SensorViewerActivity.class);
                    startActivity(intent);
                } else if (appTitle.compareTo("Event Logger") == 0) {

                    Intent intent = new Intent(MainActivity.this,
                            EventLoggerActivity.class);
                    startActivity(intent);
                }
            } else if (appType == 0) {
                //Run OPELApplication if it is not running

                String appID = "" + appList.getList().get(position).getAppId();
                String appName = appList.getList().get(position).getTitle();
                OPELContext.getCommController().requestStart(appID, appName);
            } else if (appType == 1) {
                //Open configuration view if it is running
                Toast.makeText(getApplicationContext(), appList.getList().get
                        (position).getTitle() + " is " +
                        "OPEN", Toast.LENGTH_SHORT).show();

                if (appList.getList().get(position).getConfigJson().equals
                        ("N/A")) {
                    Toast.makeText(getApplicationContext(), "Configurable " +
                            "data is N/A", Toast.LENGTH_SHORT).show();
                } else {
                    Intent intent = new Intent(MainActivity.this,
                            RemoteConfigUIActivity.class);

                    Bundle extras = new Bundle();
                    extras.putString("title", appList.getList().get(position)
                            .getTitle());
                    extras.putString("appID", "" + appList.getList().get
                            (position).getAppId());
                    extras.putString("jsonData", appList.getList().get
                            (position).getConfigJson());

                    intent.putExtras(extras);
                    startActivity(intent);
                }
            }

        }
    };

    private GridView.OnItemLongClickListener mItemLongClickListener = new
            GridView.OnItemLongClickListener() {

        public boolean onItemLongClick(AdapterView<?> arg0, View arg1, int
                position, long arg3) {
            final int tmp = position;
            final OPELAppList appList = getAppList();
            if (appList.getList().get(position).getType() == -1) {
                Toast.makeText(getApplicationContext(), "[native]" + appList
                        .getList().get(position).getTitle(), Toast
                        .LENGTH_SHORT).show();
            }

            if (appList.getList().get(position).getType() == 0) {
                Toast.makeText(getApplicationContext(), "[installed]" +
                        appList.getList().get(position).getTitle(), Toast
                        .LENGTH_SHORT).show();
            }

            if (appList.getList().get(position).getType() == 1) {
                AlertDialog.Builder alt_bld = new AlertDialog.Builder
                        (MainActivity.this);
                alt_bld.setMessage("Terminate this App ?").setCancelable
                        (false).setPositiveButton("Yes", new DialogInterface
                        .OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        // Action for 'Yes' Button

                        OPELContext.getCommController().requestTermination(""
                                + appList.getList().get(tmp).getAppId());
                        Log.d("OPEL", "Request to kill ");
                    }
                }).setNegativeButton("No", new DialogInterface
                        .OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        // Action for 'NO' Button
                        dialog.cancel();
                    }
                });

                AlertDialog alert = alt_bld.create();

                alert.setTitle(appList.getList().get(position).getTitle());

                Drawable d = new BitmapDrawable(getResources(), appList
                        .getList().get(position).getImage());
                alert.setIcon(d);
                alert.show();
            }

            return true;
        }
    };

    // TODO: Convert to CommServiceListener
    private Handler mHandler = new Handler(Looper.getMainLooper()) {

        @Override
        public void handleMessage(Message inputMessage) {
            if (inputMessage.what == OPELContext.getCommController()
                    .getUPDATE_UI()) {

                //String msg = (String) inputMessage.obj;
                updateDisplayItem();
                AppManagerActivity.updateDisplay();
                EventLoggerActivity.updateDisplay();

                //update All of the UI page
            } else if (inputMessage.what == OPELContext.getCommController()
                    .getUPDATE_TOAST()) {
                String toastMsg = (String) inputMessage.obj;
                Toast.makeText(getApplicationContext(), toastMsg, Toast
                        .LENGTH_SHORT).show();
                //update All of the UI page
            } else if (inputMessage.what == OPELContext.getCommController()
                    .getMAKE_NOTI()) {
                String notiJson = (String) inputMessage.obj;
                updateDisplayItem();
                AppManagerActivity.updateDisplay();
                makeNotification(notiJson);

                //update All of the UI page
            } else if (inputMessage.what == OPELContext.getCommController()
                    .getUPDATE_FILEMANAGER()) {
                JSONParser jp = (JSONParser) inputMessage.obj;
                FileManagerActivity.updateDisplay(jp);

            } else if (inputMessage.what == OPELContext.getCommController()
                    .getEXE_FILE()) {
                JSONParser jp = (JSONParser) inputMessage.obj;
                FileManagerActivity.runRequestedFile(getApplicationContext(),
                        jp);

            } else if (inputMessage.what == OPELContext.getCommController()
                    .getSHARE_FILE()) {
                JSONParser jp = (JSONParser) inputMessage.obj;

                FileManagerActivity.runSharingFile(getApplicationContext(), jp);

            } else if (inputMessage.what == OPELDevice.COMM_CONNECTED) {
                Log.d("OPEL", "Toast connected");
                Toast.makeText(getApplicationContext(), "Successfully " +
                        "connected to OPEL", Toast.LENGTH_SHORT).show();
                setIndicatorBT(true);
            } else if (inputMessage.what == OPELDevice.COMM_DISCONNECTED) {
                Log.d("OPEL", "Toast disconnected");
                Toast.makeText(getApplicationContext(), "Disconnected to " +
                        "OPEL", Toast.LENGTH_SHORT).show();
                setIndicatorBT(false);
            } else if (inputMessage.what == OPELDevice.COMM_CONNECTING) {
                Log.d("OPEL", "Toast disconnected");
                Toast.makeText(getApplicationContext(), "Connecting to OPEL",
                        Toast.LENGTH_SHORT).show();
                setIndicatorBT(false);
            } else if (inputMessage.what == OPELDevice.COMM_CONNECT_FAILED) {
                Log.d("OPEL", "Toast disconnected");
                Toast.makeText(getApplicationContext(), "Failed connecting " +
                        "to" + " OPEL, re-connect with CONNECT button", Toast
                        .LENGTH_LONG).show();
                setIndicatorBT(false);
            } else if (inputMessage.what == OPELDevice.COMM_ALREADY_CONNECTED) {
                Log.d("OPEL", "Toast already connecteed");
                Toast.makeText(getApplicationContext(), "Already conneceted",
                        Toast.LENGTH_SHORT).show();
                setIndicatorBT(true);
            } else if (inputMessage.what == OPELDevice
                    .COMM_ALREADY_CONNECTING) {
                Log.d("OPEL", "Toast already connecteed");
                Toast.makeText(getApplicationContext(), "Already connecting",
                        Toast.LENGTH_SHORT).show();
                setIndicatorBT(false);
            }

        }
    };

    private void setIndicatorBT(boolean isOn) {
        ImageView imageView = (ImageView) this.findViewById(R.id.indicatorBT);
        if (isOn == true) imageView.setImageResource(R.drawable.bluetooth);
        else imageView.setImageResource(R.drawable.bluetooth_disabled);
    }

    private void setIndicatorWFD(boolean isOn) {
        ImageView imageView = (ImageView) this.findViewById(R.id.indicatorWFD);
        if (isOn == true) imageView.setImageResource(R.drawable.wifidirect);
        else imageView.setImageResource(R.drawable.wifidirect_disabled);
    }

    // TODO: move to Settings
    private void initStorageWorkspace() {

        File opelDir = new File(Environment.getExternalStorageDirectory()
                .getPath() + "/OPEL");
        File opelRUIDir = new File(Environment.getExternalStorageDirectory()
                .getPath() + "/OPEL/RemoteUI");
        File opelRemoteStorageDir = new File(Environment
                .getExternalStorageDirectory().getPath() +
                "/OPEL/RemoteStorage");
        File opelIconDir = new File(Environment.getExternalStorageDirectory()
                .getPath() + "/OPEL/Icon");
        File opelCloudDir = new File(Environment.getExternalStorageDirectory
                ().getPath() + "/OPEL/CloudService");

        if (!opelDir.exists()) {
            opelDir.mkdir();
        }
        if (!opelRUIDir.exists()) {
            opelRUIDir.mkdir();
        }
        if (!opelRemoteStorageDir.exists()) {
            opelRemoteStorageDir.mkdir();
        }
        if (!opelIconDir.exists()) {
            opelIconDir.mkdir();
        }
        if (!opelCloudDir.exists()) {
            opelCloudDir.mkdir();
        }

        OPELContext.getSettings().setOpelStoragePath(opelDir);
        OPELContext.getSettings().setRUIStoragePath(opelRUIDir);
        OPELContext.getSettings().setRemoteStorageStoragePath
                (opelRemoteStorageDir);
        OPELContext.getSettings().setIconDirectoryPath(opelIconDir);
        OPELContext.getSettings().setCloudStoragePath(opelCloudDir);
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
    public void makeNotification(String JsonData) {

        NotificationManager nm = (NotificationManager) getSystemService
                (Context.NOTIFICATION_SERVICE);
        Resources res = getResources();

        Intent notificationIntent = new Intent(this, RemoteNotiUIActivity
                .class);
        Bundle extras = new Bundle();
        extras.putString("jsonData", JsonData);
        extras.putString("checkNoti", "1");
        notificationIntent.putExtras(extras);

        PendingIntent contentIntent = PendingIntent.getActivity(this, 1,
                notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        JSONParser jp = new JSONParser(JsonData);
        String appId = jp.getValueByKey("appID");
        OPELApplication targetApp = OPELContext.getAppList().getApp(appId);

        NotificationCompat.Builder builder = new NotificationCompat.Builder
                (this).setCategory(appId).setContentTitle(targetApp.getTitle
                ()).setContentText(jp.getValueByKey("description")).setTicker
                (" " + jp.getValueByKey("appTitle")).setLargeIcon(OPELContext
                .getAppList().getApp(appId).getImage()).setSmallIcon(R
                .drawable.opel).setContentIntent(contentIntent).setAutoCancel
                (true).setWhen(System.currentTimeMillis()).setDefaults
                (Notification.DEFAULT_SOUND | Notification.DEFAULT_VIBRATE |
                Notification.DEFAULT_LIGHTS).setNumber(1);
        Notification n = builder.build();
        nm.notify(1234, n);
    }
}


class IconListAdapter extends ArrayAdapter<OPELApplication> {
    Context context;
    int layoutResourceId;
    ArrayList<OPELApplication> data;


    public IconListAdapter(Context context, int layoutResourceId,
                           ArrayList<OPELApplication> data) {
        super(context, layoutResourceId, data);
        this.layoutResourceId = layoutResourceId;
        this.context = context;
        this.data = data;
    }

    public void updateDisplay() {
        this.data = getAppList().getList();
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

        OPELApplication item = data.get(position);
        holder.txtTitle.setText(item.getTitle());
        holder.imageItem.setImageBitmap(item.getImage());
        return row;

    }

    static class RecordHolder {
        TextView txtTitle;
        ImageView imageItem;
    }
}