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

import android.graphics.Bitmap;

public class OPELApplication {
    public static final int State_Initializing = 1;
    public static final int State_Initialized = 2;
    public static final int State_Installing = 3;
    public static final int State_Ready = 4;
    public static final int State_Launching = 5;
    public static final int State_Running = 6;
    public static final int State_Terminating = 7;
    public static final int State_Removing = 8;
    public static final int State_Removed = 9;

    private int appId;
    private String title;

    private Bitmap image;

    private String jsonStringForTermincation;
    private String jsonStringForConfig;
    private int type; // -1: Native Menu, 0:Installed, 1:Running

    public OPELApplication(String appID, String title, Bitmap image, int type) {
        super();
        this.appId = Integer.parseInt(appID);
        this.image = image;
        this.title = title;
        this.type = type;

        this.jsonStringForConfig = "N/A";
        this.jsonStringForTermincation = "N/A";
    }

    public int getAppId() {
        return appId;
    }

    public void setAppId(int appId) {
        this.appId = appId;
    }

    public Bitmap getImage() {
        return image;
    }

    public void setImage(Bitmap image) {
        this.image = image;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
    }

    public void setTypeToNative() {
        this.type = -1;
    }

    public void setTypeToInstalled() {
        this.type = 0;
    }

    public void setTypeToRunning() {
        this.type = 1;
    }

    public String getConfigJson() {
        return jsonStringForConfig;
    }

    public void setConfigJson(String json) {
        this.jsonStringForConfig = json;
    }

    public String getTerminationJson() {
        return jsonStringForTermincation;
    }

    public void setTerminationJson(String json) {
        this.jsonStringForTermincation = json;
    }
}
