/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.opel.cmfw.view;

import android.Manifest;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;

import com.opel.opel_manager.R;

import java.util.Set;

public class BluetoothDeviceSettingActivity extends Activity {
    private static final String TAG = "BTDeviceSettingActivity";

    // Intent to BluetoothDeviceSettingActivity
    public static final String INTENT_KEY_RECEIVER =
            "TryCommDeviceSettingResult";
    public static final String INTENT_KEY_DEFAULT_BT_NAME =
            "DefaultBluetooothDeviceName";
    public static final String INTENT_KEY_DEFAULT_BT_ADDRESS =
            "DefaultBluetooothDeviceAddress";

    public static String EXTRA_DEVICE_ADDRESS = "device_address";

    // Parameters to BluetoothDeviceSettingActivity
    // Result receiver
    private CommService.BluetoothDeviceSettingResultReceiver mReceiver;
    private String mDefaultBluetoothName;
    private String mDefaultBluetoothAddress;

    private BluetoothAdapter mBluetoothAdapter;

    // Newly discovered devices
    private ArrayAdapter<String> mNewDevicesArrayAdapter;

    private void resultInSuccess(String bluetoothName, String
            bluetoothAddress) {
        Bundle bundle = new Bundle();
        bundle.putString(CommService.BluetoothDeviceSettingResultReceiver
                .RECEIVER_KEY_BT_NAME, bluetoothName);
        bundle.putString(CommService.BluetoothDeviceSettingResultReceiver
                .RECEIVER_KEY_BT_ADDRESS, bluetoothAddress);
        this.mReceiver.send(Activity.RESULT_OK, bundle);
        finish();
    }

    private void resultInFail(String failMessage) {
        Bundle bundle = new Bundle();
        this.mReceiver.send(Activity.RESULT_CANCELED, bundle);
        finish();
    }

    // * Bluetooth Device Setting Process
    // 1. Initialize UI Layout
    // 2. Ensure that communication device permission is granted
    // 3. Ensure that bluetooth device is turned on
    // 4. Find already-bonded bluetooth device once
    //    (bonded = paired + connected)
    // 5. If there is no bonded bluetooth device, discover devices then pair and
    //    connect one of them.
    // * If one of step 2~5 is failed, it returns Activity.RESULT_CANCELED.
    // * If step 4~5 is succeeded, it returns Activity.RESULT_OK.
    // * (If step 2~3 is succeeded, it proceeds to the next step.)

    // Step 1. Initialize UI layout
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.activity_bluetooth_device_setting);

        // Get Parameters
        Intent callerIntent = this.getIntent();
        this.mReceiver = (CommService.BluetoothDeviceSettingResultReceiver)
                callerIntent.getParcelableExtra(INTENT_KEY_RECEIVER);
        this.mDefaultBluetoothName = callerIntent.getStringExtra
                (INTENT_KEY_DEFAULT_BT_NAME);
        this.mDefaultBluetoothAddress = callerIntent.getStringExtra
                (INTENT_KEY_DEFAULT_BT_ADDRESS);

        // Proceed to Step 2. Check communication device permission
        this.checkCommPermission();
    }

    @Override
    public void onBackPressed() {
        // Cancel communication setting.
        resultInFail("User canceled Bluetooth device setting!");
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

    // Step 4. Find already-bonded bluetooth device
    private void findBluetoothDevice() {
        if (this.mDefaultBluetoothName.isEmpty() == false && this
                .mDefaultBluetoothAddress.isEmpty() == false) {
            BluetoothAdapter bluetoothAdapter = BluetoothAdapter
                    .getDefaultAdapter();
            Set<BluetoothDevice> bluetoothDevices = bluetoothAdapter
                    .getBondedDevices();
            if (bluetoothDevices.size() > 0) {
                for (BluetoothDevice bluetoothDevice : bluetoothDevices) {
                    String deviceName = bluetoothDevice.getName();
                    String deviceAddress = bluetoothDevice.getAddress();
                    if (deviceName.compareTo(this.mDefaultBluetoothName) == 0
                            && deviceAddress.compareTo(this
                            .mDefaultBluetoothAddress) == 0) {
                        // If already-bonded bluetooth device is found, use it!
                        resultInSuccess(deviceName, deviceAddress);
                        return;
                    }
                }
            }
        }

        // Proceed to Step 5.
        // If there is no bonded bluetooth device, discover devices
        this.startToDiscover();
    }

    // Step 5. Discover devices
    private void startToDiscover() {
        // Initialize array adapters. One for already paired devices and
        // one for newly discovered devices
        ArrayAdapter<String> pairedDevicesArrayAdapter = new
                ArrayAdapter<String>(this, R.layout
                .template_listview_item_device);
        mNewDevicesArrayAdapter = new ArrayAdapter<String>(this, R.layout
                .template_listview_item_device);

        // Find and set up the ListView for paired devices
        ListView pairedListView = (ListView) findViewById(R.id.paired_devices);
        pairedListView.setAdapter(pairedDevicesArrayAdapter);
        pairedListView.setOnItemClickListener(mDeviceClickListener);

        // Find and set up the ListView for newly discovered devices
        ListView newDevicesListView = (ListView) findViewById(R.id.new_devices);
        newDevicesListView.setAdapter(mNewDevicesArrayAdapter);
        newDevicesListView.setOnItemClickListener(mDeviceClickListener);

        // Register for broadcasts when a device is discovered
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothDevice.ACTION_FOUND);
        filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        this.registerReceiver(mBluetoothBroadcastReceiver, filter);

        // Get the local Bluetooth adapter
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        // Get a set of currently paired devices
        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter
                .getBondedDevices();

        // If there are paired devices, add each one to the ArrayAdapter
        if (pairedDevices.size() > 0) {
            findViewById(R.id.title_paired_devices).setVisibility(View.VISIBLE);
            for (BluetoothDevice device : pairedDevices) {
                pairedDevicesArrayAdapter.add(device.getName() + "\n" +
                        device.getAddress());
            }
        } else {
            String noDevices = getResources().getText(R.string.none_paired)
                    .toString();
            pairedDevicesArrayAdapter.add(noDevices);
        }

        // Initialize the button to perform device discovery
        Button scanButton = (Button) findViewById(R.id.button_scan);
        doDiscovery();
        scanButton.setVisibility(View.GONE);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        // Make sure we're not doing discovery anymore
        if (mBluetoothAdapter != null) {
            mBluetoothAdapter.cancelDiscovery();
        }

        // Unregister broadcast listeners
        this.unregisterReceiver(mBluetoothBroadcastReceiver);
    }

    /**
     * Start device discover with the BluetoothAdapter
     */
    private void doDiscovery() {
        Log.d(TAG, "doDiscovery()");

        // Indicate scanning in the title
        setProgressBarIndeterminateVisibility(true);
        setTitle(R.string.scanning);

        // Turn on sub-title for new devices
        findViewById(R.id.title_new_devices).setVisibility(View.VISIBLE);

        // If we're already discovering, stop it
        if (mBluetoothAdapter.isDiscovering()) {
            mBluetoothAdapter.cancelDiscovery();
        }

        // Request discover from BluetoothAdapter
        mBluetoothAdapter.startDiscovery();
    }

    /**
     * The on-click listener for all devices in the ListViews
     */
    private AdapterView.OnItemClickListener mDeviceClickListener = new
            AdapterView.OnItemClickListener() {
        public void onItemClick(AdapterView<?> av, View v, int arg2, long
                arg3) {
        }
    };

    /**
     * The BroadcastReceiver that listens for discovered devices and changes
     * the title when
     * discovery is finished
     */
    private final BroadcastReceiver mBluetoothBroadcastReceiver = new
            BroadcastReceiver() {
        private boolean found = false;

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            // When discovery finds a device
            if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                // Get the BluetoothDevice object from the Intent
                Log.d(TAG, "Found");
                BluetoothDevice device = intent.getParcelableExtra
                        (BluetoothDevice.EXTRA_DEVICE);
                // If it's already paired, skip it, because it's been listed
                // already
                if (device.getBondState() != BluetoothDevice.BOND_BONDED) {
                    if (device.getName() != null) {
                        mNewDevicesArrayAdapter.add(device.getName() + "\n" +
                                device.getAddress());
                        mBluetoothAdapter.cancelDiscovery();
                        setTitle("Found Device - Pairing");
                        found = true;

                        device.createBond();
                    }
                }
                // When discovery is finished, change the Activity title
            } else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals
                    (action)) {
                if (!found) {
                    setTitle(R.string.select_device);
                    setProgressBarIndeterminateVisibility(false);
                }
                Log.d(TAG, "Discovery done");
                if (mNewDevicesArrayAdapter.getCount() == 0) {
                    String noDevices = getResources().getText(R.string
                            .none_found).toString();
                    mNewDevicesArrayAdapter.add(noDevices);
                }
            } else if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.equals
                    (action)) {
                // Step 5. Succeeded to bind device
                BluetoothDevice device = intent.getParcelableExtra
                        (BluetoothDevice.EXTRA_DEVICE);
                Log.d(TAG, "BondStatus");
                if (device.getBondState() == BluetoothDevice.BOND_BONDED) {
                    Intent resultIntent = new Intent();
                    resultIntent.putExtra(EXTRA_DEVICE_ADDRESS, device
                            .getAddress());

                    Toast.makeText(BluetoothDeviceSettingActivity.this,
                            "Bluetooth pairing success!", Toast.LENGTH_SHORT)
                            .show();

                    // Use it!
                    resultInSuccess(device.getName(), device.getAddress());
                }
            }
        }
    };
}
