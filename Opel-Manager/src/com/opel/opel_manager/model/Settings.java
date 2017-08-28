package com.opel.opel_manager.model;

import android.os.Environment;
import android.util.Log;

import java.io.File;

public class Settings {
    private static final String TAG = "Settings";

    private File mOPELDir;
    private File mOPELIconDir;
    private File mOPELRemoteUIDir;
    private File mOPELRemoteStorageDir;
    private File mOPELCloudDir;
    private File mOPELTempDir;

    public void initializeDirectories() {
        File opelDir = new File(Environment.getExternalStorageDirectory().getPath() + "/OPEL");
        File opelRUIDir = new File(Environment.getExternalStorageDirectory().getPath() +
                "/OPEL/RemoteUI");
        File opelRemoteStorageDir = new File(Environment.getExternalStorageDirectory().getPath()
                + "/OPEL/RemoteStorage");
        File opelIconDir = new File(Environment.getExternalStorageDirectory().getPath() +
                "/OPEL/Icon");
        File opelCloudDir = new File(Environment.getExternalStorageDirectory().getPath() +
                "/OPEL/CloudService");
        File tempDir = new File(Environment.getExternalStorageDirectory().getPath() + "/OPEL/Temp");

        if (!opelDir.exists()) {
            if (!opelDir.mkdir()) {
                Log.e(TAG, "Failed to make OPEL root directory");
            }
        }
        if (!opelRUIDir.exists()) {
            if (!opelRUIDir.mkdir()) {
                Log.e(TAG, "Failed to make OPEL remote UI directory");
            }
        }
        if (!opelRemoteStorageDir.exists()) {
            if (!opelRemoteStorageDir.mkdir()) {
                Log.e(TAG, "Failed to make OPEL remote storage directory");
            }
        }
        if (!opelIconDir.exists()) {
            if (!opelIconDir.mkdir()) {
                Log.e(TAG, "Failed to make OPEL icon directory");
            }
        }
        if (!opelCloudDir.exists()) {
            if (!opelCloudDir.mkdir()) {
                Log.e(TAG, "Failed to make OPEL cloud service directory");
            }
        }

        if (tempDir.exists()) {
            if (!tempDir.mkdir()) {
                Log.e(TAG, "Failed to make OPEL temp directory");
            }
        }

        this.mOPELDir = opelDir;
        this.mOPELRemoteUIDir = opelRUIDir;
        this.mOPELRemoteStorageDir = opelRemoteStorageDir;
        this.mOPELIconDir = opelIconDir;
        this.mOPELCloudDir = opelCloudDir;
        this.mOPELTempDir = tempDir;
    }

    // Getters/Setters
    public File getOpelDir() {
        return mOPELDir;
    }

    public File getIconDir() {
        return mOPELIconDir;
    }

    public File getRemoteUIDir() {
        return mOPELRemoteUIDir;
    }

    public File getRemoteStorageDir() {
        return mOPELRemoteStorageDir;
    }

    public File getCloudDir() {
        return mOPELCloudDir;
    }

    public File getTempDir() {
        return mOPELTempDir;
    }
}
