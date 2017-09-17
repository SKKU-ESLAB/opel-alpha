package com.opel.opel_manager.view.main;

/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
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

import android.graphics.Bitmap;

import com.opel.opel_manager.view.MainActivity;

public abstract class MainIcon {
    private String mTitle;
    private Bitmap mIconBitmap;

    protected MainActivity mOwnerActivity;

    public MainIcon(MainActivity ownerActivity, String title, Bitmap iconBitmap) {
        this.mOwnerActivity = ownerActivity;
        this.mTitle = title;
        this.mIconBitmap = iconBitmap;
    }

    public String getTitle() { return this.mTitle;}
    public Bitmap getIconBitmap() { return this.mIconBitmap;}

    abstract public void onClick();

    abstract public void onLongClick();
}