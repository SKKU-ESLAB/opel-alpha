package com.opel.cmfw.view;

import android.Manifest;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.view.Window;
import android.widget.Toast;

import com.opel.opel_manager.R;

public class BluetoothTurningOnActivity extends Activity {
    private static final String TAG = "BTTurningOnActivity";

    // Intent to BluetoothConnectingActivity
    public static final String INTENT_KEY_RECEIVER =
            "BTTurningOnResult";

     /* Bluetooth Device Setting Process
     1. Initialize UI Layout
     2. Ensure that communication device permission is granted
       - On fail: return Activity.RESULT_CANCELED
     3. Ensure that bluetooth device is turned on
       - On success: return Activity.RESULT_OK
       - On fail: return Activity.RESULT_CANCELED
     * If one of step 1~3 is failed, it returns Activity.RESULT_CANCELED.
     * If step 3 is succeeded, it returns Activity.RESULT_OK.
      (If step 1~2 is succeeded, it proceeds to the next step.)
     */

    // Step 1. Initialize UI layout
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.activity_bluetooth_turning_on);

        // Proceed to Step 2. Check communication device permission
        this.checkCommPermission();
    }

    // Step 2-1. Check communication device permission
    private void checkCommPermission() {
        if (checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {
            // If communication device permission is not granted, request the
            // permission once more.
            ActivityCompat.requestPermissions(this, new String[]{Manifest
                    .permission.ACCESS_COARSE_LOCATION}, 1);
        } else {
            // Proceed to Step 3.
            // If communication device permission is granted, check if bluetooth
            // device is turned on
            checkBluetoothOn();
        }
    }

    // Step 2-2. Handle the result of communication device permission request
    public void onRequestPermissionsResult(int requestCode, String
            permissions[], int[] grantResults) {
        if (grantResults.length <= 0 || grantResults[0] != PackageManager
                .PERMISSION_GRANTED) {
            // If communication device permission is eventually not granted,
            // cancel communication setting.
            Toast.makeText(this, "Cannot be granted communication device " +
                    "permission!", Toast.LENGTH_LONG).show();
            resultInFail("Cannot be granted communication device permission!");
        } else {
            // Proceed to Step 3.
            // If communication device permission is granted, check if
            // bluetooth device is turned on.
            checkBluetoothOn();
        }
    }

    // Step 3-1. Check whether bluetooth is turned on or off
    public void checkBluetoothOn() {
        // Checking Bluetooth device via BluetoothAdapter
        if (BluetoothAdapter.getDefaultAdapter() == null) {
            // If bluetooth adapter is not found, cancel communication setting.
            Toast.makeText(this, "Bluetooth device is required!", Toast
                    .LENGTH_SHORT).show();
            resultInFail("Bluetooth device is required!");
        } else if (!BluetoothAdapter.getDefaultAdapter().isEnabled()) {
            // If bluetooth device is turned off, try to turn bluetooth on.
            Intent enable_bt_intent = new Intent(BluetoothAdapter
                    .ACTION_REQUEST_ENABLE);
            startActivityForResult(enable_bt_intent, 1);
        } else {
            // Proceed to Step 4.
            // If bluetooth device is turned on, find already-bonded
            // bluetooth device.
            this.findBluetoothDevice();
        }
    }

    // Step 3-2. Handle the result of checking bluetooth on/off
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent
            data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (resultCode == Activity.RESULT_CANCELED) {
            // If trying to turn bluetooth on is failed, cancel communication
            // setting.
            Toast.makeText(this, "Failed to be granted bluetooth " +
                    "permission", Toast.LENGTH_SHORT);
            resultInFail("Failed to be granted bluetooth permission!");
        } else {
            // Proceed to Step 4.
            // Find already-bonded Bluetooth device once.
            this.findBluetoothDevice();
        }
    }

    // Result Part
    private void resultInSuccess(String bluetoothName, String
            bluetoothAddress) {
        Bundle bundle = new Bundle();
        bundle.putString(CommService.BluetoothConnectingResultReceiver
                .RECEIVER_KEY_BT_NAME, bluetoothName);
        bundle.putString(CommService.BluetoothConnectingResultReceiver
                .RECEIVER_KEY_BT_ADDRESS, bluetoothAddress);
        this.mReceiver.send(Activity.RESULT_OK, bundle);
        finish();
    }

    private void resultInFail(String failMessage) {
        Bundle bundle = new Bundle();
        this.mReceiver.send(Activity.RESULT_CANCELED, bundle);
        finish();
    }
}