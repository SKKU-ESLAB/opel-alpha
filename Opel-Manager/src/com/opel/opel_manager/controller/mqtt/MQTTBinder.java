package com.opel.opel_manager.controller.mqtt;

import android.app.Service;
import android.os.Binder;

import java.lang.ref.WeakReference;

/**
 * Created by redcarrottt on 2017. 5. 2..
 */

public class MQTTBinder<S extends Service> extends Binder
{
    private WeakReference<S> mService;

    public MQTTBinder(S service)
    {
        mService = new WeakReference<S>(service);
    }
    public S getService()
    {
        return mService.get();
    }
    public void close()
    {
        mService = null;
    }
}