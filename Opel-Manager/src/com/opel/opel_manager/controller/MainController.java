package com.opel.opel_manager.controller;

public class MainController {
    private boolean mIsInitialized;
    private boolean mIsLoading;

    public MainController() {
        this.mIsInitialized = false;
        this.mIsLoading = false;
    }

    // TODO: refactor getter/setter name
    // Getters/Setters
    public boolean getIsLoading() {
        return this.mIsLoading;
    }

    public void setIsLoading(boolean b) {
        this.mIsLoading = b;
    }

    public boolean isInit() {
        return this.mIsInitialized;
    }

    public void initComplete() {
        this.mIsInitialized = true;
    }
}
