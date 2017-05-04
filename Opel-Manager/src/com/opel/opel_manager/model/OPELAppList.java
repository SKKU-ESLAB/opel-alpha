package com.opel.opel_manager.model;

import android.content.res.Resources;
import android.graphics.BitmapFactory;

import com.opel.opel_manager.R;

import java.util.ArrayList;

/**
 * Created by redcarrottt on 2017. 5. 2..
 */

public class OPELAppList {
    private ArrayList<OPELApplication> mList;
    private boolean mIsNativeAppsAdded;

    public OPELAppList() {
        mList = new ArrayList<OPELApplication>();
    }

    public void addNativeAppList(Resources appResources) {
        if (this.mIsNativeAppsAdded == false) {
            this.add(new OPELApplication("-1", "Camera", BitmapFactory
                    .decodeResource(appResources, R.drawable.cam), -1));
            this.add(new OPELApplication("-1", "Sensor", BitmapFactory
                    .decodeResource(appResources, R.drawable.sensor), -1));
            this.add(new OPELApplication("-1", "App Market", BitmapFactory
                    .decodeResource(appResources, R.drawable.market), -1));
            this.add(new OPELApplication("-1", "App Manager", BitmapFactory
                    .decodeResource(appResources, R.drawable
                            .icon_app_manager), -1));
            this.add(new OPELApplication("-1", "File Manager", BitmapFactory
                    .decodeResource(appResources, R.drawable.filemanager), -1));
            this.add(new OPELApplication("-1", "Event Logger", BitmapFactory
                    .decodeResource(appResources, R.drawable.eventlogger), -1));
            this.add(new OPELApplication("-1", "Connect", BitmapFactory
                    .decodeResource(appResources, R.drawable.connect), -1));
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
        removeAllApplicationList(app);
    }

    public void add(OPELApplication app) {
        mList.add(app);
    }

    public void removeAllApplicationList(OPELApplication app) {
        for (int i = 0; i < mList.size(); i++) {
            if (mList.get(i).getAppId() == app.getAppId()) {
                mList.remove(i);
            }
        }
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