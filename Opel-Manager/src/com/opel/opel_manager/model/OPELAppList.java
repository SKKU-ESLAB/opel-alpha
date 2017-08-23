package com.opel.opel_manager.model;

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

import android.content.res.Resources;
import android.graphics.BitmapFactory;

import com.opel.opel_manager.R;

import java.util.ArrayList;

public class OPELAppList {
    private ArrayList<OPELApplication> mList;
    private boolean mIsNativeAppsAdded;

    public OPELAppList() {
        mList = new ArrayList<OPELApplication>();
    }

    public void addNativeAppList(Resources resources) {
        if (this.mIsNativeAppsAdded == false) {
            this.add(new OPELApplication("-1", "Camera", BitmapFactory.decodeResource(resources,
                    R.drawable.cam), -1));
            this.add(new OPELApplication("-1", "Sensor", BitmapFactory.decodeResource(resources,
                    R.drawable.sensor), -1));
            this.add(new OPELApplication("-1", "App Market", BitmapFactory.decodeResource
                    (resources, R.drawable.market), -1));
            this.add(new OPELApplication("-1", "App Manager", BitmapFactory.decodeResource
                    (resources, R.drawable.icon_app_manager), -1));
            this.add(new OPELApplication("-1", "File Manager", BitmapFactory.decodeResource
                    (resources, R.drawable.filemanager), -1));
            this.add(new OPELApplication("-1", "Event Logger", BitmapFactory.decodeResource
                    (resources, R.drawable.eventlogger), -1));
            this.add(new OPELApplication("-1", "Connect", BitmapFactory.decodeResource(resources,
                    R.drawable.connect), -1));
            this.mIsNativeAppsAdded = true;
        }
    }

    // -> ready
    public void installApplication(OPELApplication app) {
        this.add(app);
        app.setTypeToInstalled();
    }

    // ready -> remove
    public void uninstallApplication(OPELApplication app) {
        for (int i = 0; i < mList.size(); i++) {
            if (mList.get(i).getAppId() == app.getAppId()) {
                mList.remove(i);
            }
        }
    }

    public void add(OPELApplication app) {
        mList.add(app);
    }

    public ArrayList<OPELApplication> getList() {
        return this.mList;
    }

    public OPELApplication getApp(String appId) {
        for (int i = 0; i < mList.size(); i++) {
            OPELApplication tmpApp = mList.get(i);
            if (tmpApp.getAppId() == Integer.parseInt(appId)) {
                return tmpApp;
            }
        }
        return null;
    }
}