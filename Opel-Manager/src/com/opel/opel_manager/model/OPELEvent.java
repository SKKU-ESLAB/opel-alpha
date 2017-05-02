package com.opel.opel_manager.model;

/**
 * Created by redcarrottt on 2017. 5. 2..
 */

public class OPELEvent {
    private long eventID; 		//DB Primary key
    private String eventAppID;		//App ID
    private String eventTime;
    private String eventJsonData;
    private String eventDescription;
    private String eventAppName;
    private int status;
    //0: unchecked OPELEvent, 1: unchecked noti, 2:checked OPELEvent/noti

    public OPELEvent(long eventID, String eventAppID, String appName, String description, String time, String jsonData){
        this.eventID = eventID;
        this.eventAppID = eventAppID;
        this.eventAppName = appName;
        this.eventDescription = description;
        this.eventTime = time;
        this.eventJsonData = jsonData;
        this.status = 0;
    }

    public long getEventID(){
        return this.eventID;
    }
    public String getEventAppID(){
        return this.eventAppID;
    }

    public String getEventTime(){
        return this.eventTime;
    }

    public String getEventJsonData(){
        return this.eventJsonData;
    }

    public String getEventDescription(){
        return this.eventDescription;
    }

    public String getEventAppName(){
        return this.eventAppName;
    }

    public String getEvent(){
        return 	this.eventID +" "+ this.eventAppID + " " + this.eventAppName + " " + this.eventDescription + " "+this.eventTime +" " + this.eventJsonData;
    }
}