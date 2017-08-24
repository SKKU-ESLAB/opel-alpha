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

import android.os.Parcel;
import android.os.Parcelable;

public class OPELApp implements Parcelable {
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

    public OPELApp(int appId, String name, String iconImagePath, boolean isDefaultApp) {
        this(appId, name, iconImagePath, "", State_Initialized, isDefaultApp);
    }

    public OPELApp(int appId, String name, String iconImagePath, String configJSONString, int
            state, boolean isDefaultApp) {
        this.mAppId = appId;
        this.mName = name;
        this.mIconImagePath = iconImagePath;
        this.mConfigJSONString = configJSONString;
        this.mState = state;
        this.mIsDefaultApp = isDefaultApp;
    }

    public int getAppId() {
        return mAppId;
    }

    public String getName() {
        return mName;
    }

    public String getIconImagePath() {
        return this.mIconImagePath;
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

    // Android Parcelable
    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        // OPELApp -> Parcel
        // appId
        out.writeInt(this.mAppId);

        // name
        out.writeString(this.mName);

        // iconImagePath
        out.writeString(this.mIconImagePath);

        // configJSONString
        out.writeString(this.mConfigJSONString);

        // state
        out.writeInt(this.mState);

        // isDefaultApp
        out.writeString(Boolean.toString(mIsDefaultApp));
    }

    public static final Parcelable.Creator<OPELApp> CREATOR = new Parcelable.Creator<OPELApp>() {
        public OPELApp createFromParcel(Parcel in) {
            // Parcel -> OPELApp
            // appId
            int appId = in.readInt();

            // name
            String name = in.readString();

            // iconImagePath
            String iconImagePath = in.readString();

            // configJSONString
            String configJSONString = in.readString();

            // state
            int state = in.readInt();

            // isDefaultApp
            boolean isDefaultApp = Boolean.parseBoolean(in.readString());

            return new OPELApp(appId, name, iconImagePath, configJSONString, state, isDefaultApp);
        }

        public OPELApp[] newArray(int size) {
            return new OPELApp[size];
        }
    };
}