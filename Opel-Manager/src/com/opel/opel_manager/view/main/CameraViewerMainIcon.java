package com.opel.opel_manager.view.main;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.BitmapFactory;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.widget.Toast;

import com.opel.opel_manager.R;
import com.opel.opel_manager.model.OPELApp;
import com.opel.opel_manager.view.CameraViewerActivity;
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
public class CameraViewerMainIcon extends MainIcon {
    private int mAppId;

    public CameraViewerMainIcon(MainActivity ownerActivity, int appId) {
        super(ownerActivity, "Camera", BitmapFactory.decodeResource(ownerActivity.getResources(),
                R.drawable.cam));
        this.mAppId = appId;
    }

    @Override
    public void onClick() {
        if (!this.mOwnerActivity.isTargetDeviceConnected()) {
            Toast.makeText(this.mOwnerActivity.getApplicationContext(),
                    "Target device is not connected", Toast.LENGTH_SHORT).show();
            return;
        }
        Intent intent = new Intent(this.mOwnerActivity, CameraViewerActivity.class);
        mOwnerActivity.startActivity(intent);
    }

    @Override
    public void onLongClick() {
        // System app -> cannot terminate
    }
}
