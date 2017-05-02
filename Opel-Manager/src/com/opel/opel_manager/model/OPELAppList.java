package com.opel.opel_manager.model;

import java.util.ArrayList;

/**
 * Created by redcarrottt on 2017. 5. 2..
 */

public class OPELAppList {

    ArrayList<OPELApplication> mList;

    public OPELAppList() {
        mList = new ArrayList<OPELApplication>();
    }

    // -> ready
    public void installApplication(OPELApplication app) {

        add(app);
        app.setTypeToInstalled();
    }

    // ready -> remove
    public void uninstallApplication(OPELApplication app) {

        removeAllApplicationList(app);

    }

    public void add(OPELApplication icon) {
        mList.add(icon);
    }

    public void removeAllApplicationList(OPELApplication icon) {
        for(int i = 0; i< mList.size(); i++){
            if (mList.get(i).getAppId()
                    == icon.getAppId()){
                mList.remove(i);
            }
        }
    }

    public ArrayList<OPELApplication> getList() {
        return this.mList;
    }

    public OPELApplication getAppInAllList(String appId) {

        for (int i = 0; i < mList.size(); i++) {
            OPELApplication tmpApp = mList.get(i);
            if (tmpApp.getAppId() == Integer.parseInt(appId)) {
                return tmpApp;
            }
        }

        return null;
    }

}