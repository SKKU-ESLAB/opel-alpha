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

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.provider.BaseColumns;
import android.util.Log;

import java.util.ArrayList;

public class OPELEventList {

    private static final String DATABASE_NAME = "eventList.db";
    private static final int DATABASE_VERSION = 1;
    public static SQLiteDatabase mDB;
    private DatabaseHelper mEventDBHelper;

    ArrayList<OPELEvent> evList = new ArrayList<OPELEvent>();

    private class DatabaseHelper extends SQLiteOpenHelper {
        public DatabaseHelper(Context context, String name, SQLiteDatabase.CursorFactory factory,
                              int version) {
            super(context, name, factory, version);
        }

        public void onCreate(SQLiteDatabase db) {
            db.execSQL(EventListDBColumn._CREATE);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
            db.execSQL("DROP TABLE IF EXISTS " + EventListDBColumn.TABLENAME);
            onCreate(db);
        }
    }

    public OPELEventList open(Context context) throws SQLException {
        mEventDBHelper = new DatabaseHelper(context, DATABASE_NAME, null, DATABASE_VERSION);

        mDB = mEventDBHelper.getWritableDatabase();
        evList = getAllEventArrayList();
        return this;
    }

    public void close() {
        mDB.close();
        evList.clear();
    }

    public ArrayList<OPELEvent> addEvent(String eventAppID, String eventAppName, String
            eventDescription, String eventTime, String eventJsonData) {
        ContentValues values = new ContentValues();

        values.put(EventListDBColumn.eventAppID, eventAppID);
        values.put(EventListDBColumn.eventAppName, eventAppName);
        values.put(EventListDBColumn.eventDescription, eventDescription);
        values.put(EventListDBColumn.eventTime, eventTime);
        values.put(EventListDBColumn.eventJsonData, eventJsonData);

        long id = mDB.insert(EventListDBColumn.TABLENAME, null, values);

        // success
        if (id != -1) {
            evList.add(0, new OPELEvent(id, eventAppID, eventAppName, eventDescription,
                    eventTime, eventJsonData));

            Log.d("OPEL", "SUCCESS ADD EVENT");
        }

        return this.evList;
    }

    public ArrayList<OPELEvent> removeEvent(long id) {
        // TODO : erase img file that the noti page involve
        int _id = mDB.delete(EventListDBColumn.TABLENAME, "_id=" + id, null);
        if (_id > 0) {
            for (int i = 0; i < evList.size(); i++) {
                if (evList.get(i).getEventId() == id) {
                    evList.remove(i);
                    break;
                }
            }
        }
        return this.evList;
    }

    // Select All
    private Cursor getAllEvents() {
        return mDB.query(EventListDBColumn.TABLENAME, null, null, null, null, null, "eventTime "
                + "desc");
    }

    public ArrayList<OPELEvent> eventAllClear() {
        if (mDB.delete(EventListDBColumn.TABLENAME, null, null) > 0) {
            evList.clear();

        }
        return this.evList;
    }

    // Get event
    public ArrayList<OPELEvent> getCurEventArrayList() {
        return this.evList;
    }

    public ArrayList<OPELEvent> getAllEventArrayList() {
        Cursor mCursor;
        ArrayList<OPELEvent> list = new ArrayList<OPELEvent>();
        mCursor = null;
        mCursor = getAllEvents();

        while (mCursor.moveToNext()) {

            OPELEvent e = new OPELEvent(mCursor.getLong(mCursor.getColumnIndex("_id")), mCursor
                    .getString(mCursor.getColumnIndex("eventAppID")), mCursor.getString(mCursor
                    .getColumnIndex("eventAppName")), mCursor.getString(mCursor.getColumnIndex
                    ("eventDescription")), mCursor.getString(mCursor.getColumnIndex("eventTime"))
                    , mCursor.getString(mCursor.getColumnIndex("eventJsonData")));

            list.add(e);
        }

        mCursor.close();
        this.evList = list;
        return this.evList;
    }
}

class EventListDBColumn implements BaseColumns {
    public static final String eventAppID = "eventAppID";
    public static final String eventAppName = "eventAppName";
    public static final String eventDescription = "eventDescription";
    public static final String eventTime = "eventTime";
    public static final String eventJsonData = "eventJsonData";

    public static final String TABLENAME = "EVENTTABLE";
    public static final String _CREATE = "create table " + TABLENAME + "" + "(" + _ID + " " +
            "integer primary key autoincrement, " + eventAppID + " text not null , " +
            eventAppName + " text not " + "null , " + eventDescription + " text not null , " +
            eventTime + " text not null , " + eventJsonData + " text not " + "null )";
}