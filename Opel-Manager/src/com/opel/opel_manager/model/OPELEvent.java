package com.opel.opel_manager.model;

/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: Dongig Sin<dongig@skku.edu>
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

public class OPELEvent implements Parcelable {
    private long mEventId;        // DB Primary key
    private String mEventAppId;   // App ID
    private String mEventTime;
    private String mEventJsonData;
    private String mEventDescription;
    private String mEventAppName;
    // TODO: is state needed?
    // state = 0: unchecked OPELEvent, 1: unchecked noti, 2:checked OPELEvent/noti

    public OPELEvent(long eventId, String eventAppId, String appName, String description, String
            time, String jsonData) {
        this.mEventId = eventId;
        this.mEventAppId = eventAppId;
        this.mEventTime = time;
        this.mEventJsonData = jsonData;
        this.mEventDescription = description;
        this.mEventAppName = appName;
    }

    public long getEventId() {
        return this.mEventId;
    }

    public String getEventAppId() {
        return this.mEventAppId;
    }

    public String getEventTime() {
        return this.mEventTime;
    }

    public String getEventJsonData() {
        return this.mEventJsonData;
    }

    public String getEventDescription() {
        return this.mEventDescription;
    }

    public String getEventAppName() {
        return this.mEventAppName;
    }

    // Android Parcelable
    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        // OPELEvent -> Parcel
        // mEventId
        out.writeLong(this.mEventId);

        // mEventAppId
        out.writeString(this.mEventAppId);

        // mEventTime
        out.writeString(this.mEventTime);

        // mEventJsonData
        out.writeString(this.mEventJsonData);

        // mEventDescription
        out.writeString(this.mEventDescription);

        // mEventAppName
        out.writeString(this.mEventAppName);
    }

    public static final Parcelable.Creator<OPELEvent> CREATOR = new Parcelable.Creator<OPELEvent>
            () {
        public OPELEvent createFromParcel(Parcel in) {
            // Parcel -> OPELEvent
            // mEventId
            long eventId = in.readLong();

            // mEventAppId
            String eventAppId = in.readString();

            // mEventTime
            String eventTime = in.readString();

            // mEventJsonData
            String eventJsonData = in.readString();

            // mEventDescription
            String eventDescription = in.readString();

            // mEventAppName
            String eventAppName = in.readString();

            return new OPELEvent(eventId, eventAppId, eventTime, eventJsonData,
                    eventDescription, eventAppName);
        }

        public OPELEvent[] newArray(int size) {
            return new OPELEvent[size];
        }
    };
}