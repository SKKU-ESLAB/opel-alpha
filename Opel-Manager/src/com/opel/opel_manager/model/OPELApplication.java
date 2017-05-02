package com.opel.opel_manager.model;

import android.graphics.Bitmap;

/**
 * Created by redcarrottt on 2017. 5. 2..
 */

public class OPELApplication {

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
