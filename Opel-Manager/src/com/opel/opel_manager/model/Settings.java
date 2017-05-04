package com.opel.opel_manager.model;

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

    // TODO: refactor getter/setter name
    // Getters/Setters
    public String getOpelDataDir() {
        return this.mOpelDataDir;
    }

    public void setOpelDataDir(String opelDataDir) {
        this.mOpelDataDir = opelDataDir;
    }

    public File getOpelStoragePath() {
        return mOPELDir;
    }

    public void setOpelStoragePath(File f) {
        this.mOPELDir = f;
    }

    public void setIconDirectoryPath(File f) {
        this.mOPELIconDir = f;
    }

    public File getIconDirectoryPath() {
        return mOPELIconDir;
    }

    public void setRUIStoragePath(File f) {
        this.mOPELRemoteUIDir = f;
    }

    public File getRUIStoragePath() {
        return mOPELRemoteUIDir;
    }

    public File getRemoteStorageStoragePath() {
        return mOPELRemoteStorageDir;
    }

    public void setRemoteStorageStoragePath(File f) {
        this.mOPELRemoteStorageDir = f;
    }

    public void setCloudStoragePath(File f) {
        this.mOPELCloudDir = f;
    }

    public File getCloudStoragePath() {
        return mOPELCloudDir;
    }
}
