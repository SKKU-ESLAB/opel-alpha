package com.opel.opel_manager.view.main;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.widget.Toast;

import com.opel.opel_manager.R;
import com.opel.opel_manager.view.MainActivity;

/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: 
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
public class UserAppMainIcon extends MainIcon {
    private static String TAG = "UserAppMainIcon";
    private int mAppId;
    private String mAppName;

    public UserAppMainIcon(MainActivity ownerActivity, int appId, String appName) {
        super(ownerActivity, appName, BitmapFactory.decodeResource(ownerActivity.getResources(),
                R.drawable.cam));
        this.mAppId = appId;
        this.mAppName = appName;
    }

    @Override
    public void onClick() {
        if (!this.mOwnerActivity.isTargetDeviceConnected()) {
            Toast.makeText(this.mOwnerActivity.getApplicationContext(),
                    "Target device is not connected", Toast.LENGTH_SHORT).show();
            return;
        }

        mOwnerActivity.launchApp(this.mAppId);
    }

    @Override
    public void onLongClick() {
        // Apps Running
        AlertDialog.Builder alt_bld = new AlertDialog.Builder(this.mOwnerActivity);
        alt_bld.setMessage("Terminate this App ?").setCancelable(false).setPositiveButton("Yes",
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        mOwnerActivity.terminateApp(mAppId);
                        Log.d(TAG, "Request to kill ");
                    }
                }).setNegativeButton("No", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                dialog.cancel();
            }
        });

        AlertDialog alert = alt_bld.create();
        alert.setTitle(this.getTitle());
        Drawable d = new BitmapDrawable(this.getIconBitmap());
        alert.setIcon(d);
        alert.show();
    }
}
