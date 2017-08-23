package com.opel.opel_manager.model;

/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
 *              Dongig Sin<dongig@skku.edu>
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

public class OPELApp {
    // AppState
    public static final int State_Initializing = 1;
    public static final int State_Initialized = 2;
    public static final int State_Installing = 3;
    public static final int State_Ready = 4;
    public static final int State_Launching = 5;
    public static final int State_Running = 6;
    public static final int State_Terminating = 7;
    public static final int State_Removing = 8;
    public static final int State_Removed = 9;

    private int mAppId;
    private String mName;
    private String mIconImagePath;
    private String mConfigJSONString;
    private int mState;
    private boolean mIsDefaultApp;

    public OPELApp(int appID, String name, String iconImagePath, boolean mIsDefaultApp) {
        super();
        this.mAppId = appID;
        this.mName = name;
        this.mIconImagePath = iconImagePath;
        this.mConfigJSONString = "";
    }

    public int getAppId() {
        return mAppId;
    }

    public String getIconImagePath() {
        return this.mIconImagePath;
    }

    public String getName() {
        return mName;
    }

    public String getConfigJSONString() {
        return mConfigJSONString;
    }

    // Called by CompanionMessage.SendConfigPage
    public void setConfigJSONString(String configJSONString) {
        this.mConfigJSONString = configJSONString;
    }

    public int getState() {
        return this.mState;
    }

    public void setState(int state) {
        this.mState = state;
    }

    public boolean isDefaultApp() {
        return this.mIsDefaultApp;
    }
}
