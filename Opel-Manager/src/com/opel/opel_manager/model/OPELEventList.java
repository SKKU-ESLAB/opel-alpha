package com.opel.opel_manager.model;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.provider.BaseColumns;
import android.util.Log;

import java.util.ArrayList;
import java.util.concurrent.Semaphore;

public class OPELEventList {

    private static final String DATABASE_NAME = "eventList.db";
    private static final int DATABASE_VERSION = 1;
    public static SQLiteDatabase mDB;
    private DatabaseHelper eventDatabaseHelper;
    private Context mCtx;

    private Semaphore eventListLock = new Semaphore(1);

    ArrayList<OPELEvent> evList = new ArrayList<OPELEvent>();

    private class DatabaseHelper extends SQLiteOpenHelper {

        public DatabaseHelper(Context context, String name, SQLiteDatabase
                .CursorFactory factory, int version) {
            super(context, name, factory, version);
        }

        public void onCreate(SQLiteDatabase db) {
            db.execSQL(DataBases.CreateDB._CREATE);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int
                newVersion) {
            db.execSQL("DROP TABLE IF EXISTS " + DataBases.CreateDB.TABLENAME);
            onCreate(db);
        }
    }

    public OPELEventList() {


    }

    public OPELEventList(Context context) {
        this.mCtx = context;

    }

    public OPELEventList open(Context mContext) throws SQLException {
        eventDatabaseHelper = new DatabaseHelper(mContext, DATABASE_NAME,
                null, DATABASE_VERSION);

        mDB = eventDatabaseHelper.getWritableDatabase();
        evList = getAllEventArrayList();

        this.mCtx = mContext;
        return this;
    }

    public void close() {
        mDB.close();
        evList.clear();
    }

    // Insert DB
    public ArrayList<OPELEvent> addEvent(String eventAppID, String
            eventAppName, String eventDescription, String eventTime, String
            eventJsonData) {
        ContentValues values = new ContentValues();

        values.put(DataBases.CreateDB.eventAppID, eventAppID);
        values.put(DataBases.CreateDB.eventAppName, eventAppName);
        values.put(DataBases.CreateDB.eventDescription, eventDescription);
        values.put(DataBases.CreateDB.eventTime, eventTime);
        values.put(DataBases.CreateDB.eventJsonData, eventJsonData);

        long id = mDB.insert(DataBases.CreateDB.TABLENAME, null, values);

        //success
        if (id != -1) {
            evList.add(0, new OPELEvent(id, eventAppID, eventAppName,
                    eventDescription, eventTime, eventJsonData));

            Log.d("OPEL", "SUCCESS ADD EVENT");
        }

        return this.evList;
    }
/*
    // Update DB
	public boolean updateColumn(long id ,String eventAppID, String
	eventAppName, String eventDescription, String eventTime, String
	eventJsonData){
		ContentValues values = new ContentValues();

		values.put(DataBases.CreateDB.eventAppID, eventAppID);
		values.put(DataBases.CreateDB.eventAppName, eventAppName);
		values.put(DataBases.CreateDB.eventDescription, eventDescription);
		values.put(DataBases.CreateDB.eventTime, eventTime);
		values.put(DataBases.CreateDB.eventJsonData, eventJsonData);

		return mDB.update(DataBases.CreateDB.TABLENAME, values, "_id="+id,
		null) > 0;
	}
*/

    // Delete ID
    // TODO : erase img file that the noti page involve
    public ArrayList<OPELEvent> deleteEvent(long id) {
        int _id = mDB.delete(DataBases.CreateDB.TABLENAME, "_id=" + id, null);

        if (_id > 0) {

            for (int i = 0; i < evList.size(); i++) {

                if (evList.get(i).getEventID() == id) {
                    evList.remove(i);

                    break;
                }
            }
        }

        return this.evList;
    }

    // Select All
    private Cursor getAllEvents() {
        return mDB.query(DataBases.CreateDB.TABLENAME, null, null, null,
                null, null, "eventTime desc");
    }

    private Cursor getEvent(long id) {
        Cursor c = mDB.query(DataBases.CreateDB.TABLENAME, null, "_id=" + id,
                null, null, null, null);
        if (c != null && c.getCount() != 0) c.moveToFirst();
        return c;
    }

    private Cursor getMatchApp(String appID) {
        Cursor c = mDB.rawQuery("select * from " + DataBases.CreateDB
                .TABLENAME + " where eventAppID=" + "'" + appID + "'" +
                "order by eventTime desc", null);
        return c;
    }

    public ArrayList<OPELEvent> eventAllClear() {
        if (mDB.delete(DataBases.CreateDB.TABLENAME, null, null) > 0) {
            evList.clear();

        }
        return this.evList;
    }

    public ArrayList<OPELEvent> getCurEventArrayList() {
        return this.evList;
    }

    public ArrayList<OPELEvent> getAllEventArrayList() {
        Cursor mCursor;
        ArrayList<OPELEvent> list = new ArrayList<OPELEvent>();
        mCursor = null;
        mCursor = getAllEvents();

        while (mCursor.moveToNext()) {

            OPELEvent e = new OPELEvent(mCursor.getLong(mCursor
                    .getColumnIndex("_id")), mCursor.getString(mCursor
                    .getColumnIndex("eventAppID")), mCursor.getString(mCursor
                    .getColumnIndex("eventAppName")), mCursor.getString
                    (mCursor.getColumnIndex("eventDescription")), mCursor
                    .getString(mCursor.getColumnIndex("eventTime")), mCursor
                    .getString(mCursor.getColumnIndex("eventJsonData")));

            list.add(e);
        }

        mCursor.close();
        this.evList = list;
        return this.evList;
    }

    public ArrayList<OPELEvent> getAppEventArrayList(String appID) {
        Cursor mCursor;
        ArrayList<OPELEvent> list = new ArrayList<OPELEvent>();
        mCursor = null;
        mCursor = getMatchApp(appID);

        while (mCursor.moveToNext()) {

            OPELEvent e = new OPELEvent(mCursor.getLong(mCursor
                    .getColumnIndex("_id")), mCursor.getString(mCursor
                    .getColumnIndex("eventAppID")), mCursor.getString(mCursor
                    .getColumnIndex("eventAppName")), mCursor.getString
                    (mCursor.getColumnIndex("eventDescription")), mCursor
                    .getString(mCursor.getColumnIndex("eventTime")), mCursor
                    .getString(mCursor.getColumnIndex("eventJsonData")));

            list.add(e);
        }

        mCursor.close();

        this.evList = list;
        return this.evList;
    }
}

class DataBases {

    public static final class CreateDB implements BaseColumns {

        public static final String eventAppID = "eventAppID";
        public static final String eventAppName = "eventAppName";
        public static final String eventDescription = "eventDescription";
        public static final String eventTime = "eventTime";
        public static final String eventJsonData = "eventJsonData";

        public static final String TABLENAME = "EVENTTABLE";
        public static final String _CREATE = "create table " + TABLENAME + "" +
                "(" + _ID + " integer primary key autoincrement, " +
                eventAppID + " text not null , " + eventAppName + " text not " +
                "null , " + eventDescription + " text not null , " +
                eventTime + " text not null , " + eventJsonData + " text not " +
                "null )";
    }
}