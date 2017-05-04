package com.opel.cmfw.view;

import android.app.Service;
import android.content.Intent;
import android.os.Bundle;
import android.os.ResultReceiver;
import android.util.Log;
import android.widget.Toast;

import com.opel.cmfw.controller.CommController;
import com.opel.opel_manager.controller.OPELContext;

import static android.R.attr.data;
import static android.app.Activity.RESULT_OK;

// User of CMFW should inherit CommService, not pure Activity.
public class CommService extends Service {
    private static final String TAG = "CommService";

    public static final String INTENT_KEY_RECEIVER =
            "TryCommDeviceSettingResult";
    public static final String RECEIVER_KEY_IS_BT_CONNECTED =
            "IsBluetoothConnected";
    public static final String RECEIVER_KEY_BT_NAME = "BluetooothDeviceName";
    public static final String RECEIVER_KEY_BT_ADDRESS =
            "BluetooothDeviceAddress";

    private CommController mCommController;

    // Step 1. Try communication device setting
    protected void tryCommDeviceSetting() {
        // Launch BluetoothDeviceSettingActivity for setting communication
        // devices.
        // TODO: restore bluetooth name froms local storage
        // TODO: pass bluetooth name as a parameter
        Intent btDeviceSettingIntent = new Intent(this,
                BluetoothDeviceSettingActivity.class);
        btDeviceSettingIntent.putExtra(INTENT_KEY_RECEIVER, new
                BluetoothDeviceSettingResultReceiver());
        this.startActivity(btDeviceSettingIntent);
    }

    protected void onCommSettingResult(CommController commController) {

    }

    class BluetoothDeviceSettingResultReceiver extends ResultReceiver {

        public BluetoothDeviceSettingResultReceiver() {
            // Pass in a handler or null if you don't care about the thread
            // on which your code is executed.
            super(null);
        }

        @Override
        protected void onReceiveResult(int resultCode, Bundle resultData) {
            if (resultCode != RESULT_OK) {
                return;
            }

            boolean result = resultData.getBoolean
                    (RECEIVER_KEY_IS_BT_CONNECTED);
            if (result == true) {
                try {
                    String bluetoothName = resultData.getString
                            (RECEIVER_KEY_BT_NAME);
                    String bluetoothAddress = resultData.getString
                            (RECEIVER_KEY_BT_ADDRESS);
                    Log.d(TAG, "Pairing request done");
                    // TODO: get name of the target bluetooth device
                    // TODO: store the name to local storage

                    if (bluetoothAddress.length() > 1)
                        OPELContext.getCommController().Connect();
                } catch (RuntimeException e) {
                    e.printStackTrace();
                }
            } else {
                Log.e(TAG, "Failed pairing operation");
            }
        }
    }
}
