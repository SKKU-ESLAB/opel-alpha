package com.opel.opel_manager.model;

import android.os.Environment;

import java.io.File;

public class Settings {
    // Settings
    // TODO: determine whether OpelDataDir should be string or file
    private String mOpelDataDir;
    private File mOPELDir;
    private File mOPELIconDir;
    private File mOPELRemoteUIDir;
    private File mOPELRemoteStorageDir;
    private File mOPELCloudDir;

    public void initializeDirectories() {

        File opelDir = new File(Environment.getExternalStorageDirectory()
                .getPath() + "/OPEL");
        File opelRUIDir = new File(Environment.getExternalStorageDirectory()
                .getPath() + "/OPEL/RemoteUI");
        File opelRemoteStorageDir = new File(Environment
                .getExternalStorageDirectory().getPath() +
                "/OPEL/RemoteStorage");
        File opelIconDir = new File(Environment.getExternalStorageDirectory()
                .getPath() + "/OPEL/Icon");
        File opelCloudDir = new File(Environment.getExternalStorageDirectory
                ().getPath() + "/OPEL/CloudService");

        if (!opelDir.exists()) {
            opelDir.mkdir();
        }
        if (!opelRUIDir.exists()) {
            opelRUIDir.mkdir();
        }
        if (!opelRemoteStorageDir.exists()) {
            opelRemoteStorageDir.mkdir();
        }
        if (!opelIconDir.exists()) {
            opelIconDir.mkdir();
        }
        if (!opelCloudDir.exists()) {
            opelCloudDir.mkdir();
        }

        this.setOpelDir(opelDir);
        this.setRUIStorageDir(opelRUIDir);
        this.setRemoteStorageDir(opelRemoteStorageDir);
        this.setIconDir(opelIconDir);
        this.setCloudDir(opelCloudDir);
    }

    // Getters/Setters
    public String getOpelDataDir() {
        return this.mOpelDataDir;
    }

    public void setOpelDataDir(String opelDataDir) {
        this.mOpelDataDir = opelDataDir;
    }

    public File getOpelDir() {
        return mOPELDir;
    }

    public void setOpelDir(File f) {
        this.mOPELDir = f;
    }

    public void setIconDir(File f) {
        this.mOPELIconDir = f;
    }

    public File getIconDir() {
        return mOPELIconDir;
    }

    public void setRUIStorageDir(File f) {
        this.mOPELRemoteUIDir = f;
    }

    public File getRemoteUIDir() {
        return mOPELRemoteUIDir;
    }

    public File getRemoteStorageDir() {
        return mOPELRemoteStorageDir;
    }

    public void setRemoteStorageDir(File f) {
        this.mOPELRemoteStorageDir = f;
    }

    public void setCloudDir(File f) {
        this.mOPELCloudDir = f;
    }

    public File getCloudDir() {
        return mOPELCloudDir;
    }
}
