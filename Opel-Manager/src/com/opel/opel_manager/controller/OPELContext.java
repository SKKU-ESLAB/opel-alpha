package com.opel.opel_manager.controller;

import android.content.Context;

import com.opel.opel_manager.model.OPELAppList;
import com.opel.opel_manager.model.OPELEventList;
import com.opel.opel_manager.model.Settings;

public class OPELContext {
    // OPELContext contains objects which belong to Companion device's context.
    // Models
    private Settings mSettings;
    private OPELAppList mOPELAppList;
    private OPELEventList mOPELEventList;

    // Controllers
    private OPELAppCoreStub mOPELAppCoreStub;
    private MQTTController mMQTTController;

    // State
    private boolean mIsInitialized;
    private boolean mIsAppInfoLoading;

    private OPELContext() {
        this.mIsInitialized = false;
        this.mIsAppInfoLoading = false;

        this.mSettings = new Settings();

        this.mOPELAppCoreStub = new OPELAppCoreStub();
        this.mOPELAppList = new OPELAppList();
        this.mOPELEventList = new OPELEventList();
        this.mMQTTController = new MQTTController();
    }

    static private OPELContext singleton = null;

    private static OPELContext get() {
        if (singleton == null) {
            singleton = new OPELContext();
        }
        return singleton;
    }

    public static void initialize(Context context, android.os.Handler
            handler) {
        if (!OPELContext.get().isInit()) {
            // Initialize OPEL-related directories
            OPELContext.getSettings().initializeDirectories();

            // Add native applications to AppList
            OPELContext.getAppList().addNativeAppList(context.getResources());

            // Restore EventList
            OPELContext.getEventList().open(context);

            OPELContext.getAppCore().setMainUIHandler(handler);

            OPELContext.initComplete();
        }
    }

    public static void finish() {
        OPELContext.getEventList().close();
    }

    // TODO: refactor getter/setter name
    // Getters/Setters
    public static Settings getSettings() {
        return OPELContext.get().mSettings;
    }

    public static OPELAppList getAppList() {
        return OPELContext.get().mOPELAppList;
    }

    public static OPELEventList getEventList() {
        return OPELContext.get().mOPELEventList;
    }

    public static OPELAppCoreStub getAppCore() {
        return OPELContext.get().mOPELAppCoreStub;
    }

    public static MQTTController getMQTTController() {
        return OPELContext.get().mMQTTController;
    }

    public static boolean isAppInfoLoading() {
        return OPELContext.get().mIsAppInfoLoading;
    }

    public static void setIsAppInfoLoading(boolean isAppInfoLoading) {
        OPELContext.get().mIsAppInfoLoading = isAppInfoLoading;
    }

    public static boolean isInit() {
        return OPELContext.get().mIsInitialized;
    }

    public static void initComplete() {
        OPELContext.get().mIsInitialized = true;
    }
}