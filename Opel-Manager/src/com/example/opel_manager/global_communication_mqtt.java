package com.example.opel_manager;


import java.lang.ref.WeakReference;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

import org.sidibe.mqtt.android.lib.MqttClientState;
import org.sidibe.mqtt.android.lib.MqttMessage;
import org.sidibe.mqtt.android.lib.MqttTopic;

import android.app.Activity;
import android.app.Application;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

/**
 * Created by ES Lab on 2016-06-15.
 */
public class global_communication_mqtt {
    private MqttDataReceiver dataReceiver;
    private IntentFilter intentCFilter;
    private mqttPublisher publisher;
    private String[] topic = {"opel/msg", "opel/img"} ;
    private Context ctx;

    Handler handler;

    public global_communication_mqtt(){

    }

    public void initMqttManager(Context ctx, Intent itt, Handler handler){

        boolean result = retrieveIfLaunchFromNotification(itt);
        if (!result) {
            intentCFilter = new IntentFilter(MqttBroadCastService.ACTION_PUSH_ARRIVED);
            intentCFilter.addAction(MqttBroadCastService.ACTION_STATE_CHANGED);
            intentCFilter.addAction(MqttBroadCastService.ACTION_CLIEND_ID);
            dataReceiver = new MqttDataReceiver();

            this.ctx = ctx;
            this.handler = handler;
        }
    }

    public void initPublisher(){
        publisher = new mqttPublisher(ctx, topic);
    }

    public void mqttSendMsg(String topic, String msg){

        publisher.onPushMessage(msg, topic);

    }
    public void mqttSendFile(String topic, byte[] file){

        publisher.onPushByteArray(file, topic);

    }
    //private final MessageListAdapter messageAdapter = new MessageListAdapter(new ArrayList<MqttMessage>());
    public boolean retrieveIfLaunchFromNotification(Intent intent) {

        if (intent == null) {

            return false;
        }

        Bundle bundle = intent.getExtras();
        if (bundle == null) {

            return false;
        }

        MqttClientState clientState = (MqttClientState) bundle.getSerializable(MqttBroadCastService.KEY_STATE);

        if (clientState != null) {
            setupConnectionStatePart(clientState);

            return true;
        }

        MqttMessage mqttMessage = (MqttMessage) bundle.getParcelable(MqttBroadCastService.KEY_DATA);
        if (mqttMessage != null) {
            //messageAdapter.add(mqttMessage);

            return true;
        }
        return false;

    }


    public MqttDataReceiver getDataReceiver(){
        return this.dataReceiver;
    }

    public IntentFilter getIntentFilter(){
        return this.intentCFilter;
    }

    public void submitTopics(){

        for(int i=0; i<topic.length; i++){
            publisher.onTopicSubmitted(topic[i]);
          //  Log.d("OPEL", "topic : " + topic[i]);
        }
    }

    private void setupConnectionStatePart(MqttClientState clientState) {

     //   Log.d("OPEL", "Client state : " + clientState.name()+ "     " + clientState.getMessage());

        if( !clientState.name().equals( "CONNECTED") ){

            submitTopics();
        }

        else if( !clientState.name().equals( "CONNECTING") ){
            submitTopics();
        }

        else if( !clientState.name().equals( "SUBSCRIBE_ERROR") ){

        }

        else if( !clientState.name().equals( "NOTCONNECTED_UNKNOWNREASON") ){

        }

        else if( !clientState.name().equals( "NOTCONNECTED_WAITINGFORINTERNET") ){

        }

    }

    private void showGeneratedClientId(String clientID) {

        Log.d("OPEL",  "Client ID : " + clientID);
    }


    private class MqttDataReceiver extends BroadcastReceiver {
        private final int UPDATE_UI = 3000;
        private final int UPDATE_TOAST = 3001;
        private final int MAKE_NOTI = 3002;
        private final int UPDATE_FILEMANAGER = 3003;
        private final int EXE_FILE = 3004;
        private final int SHARE_FILE = 3005;

        @Override
        public void onReceive(Context ctx, Intent intent) {
            String action = intent.getAction();
          //  Log.d("OPEL", "RCV MSG : ");
            if (MqttBroadCastService.ACTION_PUSH_ARRIVED.equalsIgnoreCase(action)) {
                MqttMessage message = intent.getExtras().getParcelable(MqttBroadCastService.KEY_DATA);
                //messageAdapter.add(message);
                String content = message.getContentText();

             //   Log.d("OPEL", "RCV MSG : "+ message + "   Content : " + content + "Topic : "+message.getTopic().getName());

                if(message.getTopic().getName().equals("opel/msg")){
               //     Log.d("OPEL","DONGIGIGIG");
                    jsonParser jp = new jsonParser(content);
                    String name = jp.getValueByKey("Name");
                    String age = jp.getValueByKey("age");
                    String gender = jp.getValueByKey("gender");
                    String school = jp.getValueByKey("info");
                    String img = jp.getValueByKey("img");

                    if(name.equals("") || age.equals("") || gender.equals("") || school.equals("") || img.equals("")) {
                        Log.d("OPEL", "[MQTT Listener -> the data from cloud server is not correct : " + jp.getJsonData());
                    }
                   String appID = String.valueOf(Integer.parseInt(img.substring(0, 5)));
                   application ap = globalData.getInstance().getAppList().getAppInAllList(appID);

                    String appTitle = ap.getTitle();


                    SimpleDateFormat sdf = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
                    String dateTime = sdf.format(new Date());
                    String description = name + "("+age+") is Dectected";
                    String txt = school + " - " + gender;



                    jsonParser sendJson = new jsonParser();
                    sendJson.makeNewJson();
                    sendJson.addJsonKeyValue("type", "1005");
                    sendJson.addJsonKeyValue("appID", appID);
                    sendJson.addJsonKeyValue("appTitle", appTitle);
                    sendJson.addJsonKeyValue("dateTime", dateTime);
                    sendJson.addJsonKeyValue("isNoti","2");
                    sendJson.addJsonKeyValue("description", description );

                    sendJson.addJsonKeyValue("text", txt);
                    sendJson.addJsonKeyValue("img", img);

                    globalData.getInstance().getEventList().addEvent(sendJson.getValueByKey("appID"), sendJson.getValueByKey("appTitle"), sendJson.getValueByKey("description"), sendJson.getValueByKey("dateTime"), sendJson.jsonData);
                    updateMainUIThread(MAKE_NOTI, sendJson.getJsonData() );
                    updateMainUIThread(UPDATE_UI);


                }
                return;

            }
            if (MqttBroadCastService.ACTION_STATE_CHANGED.equalsIgnoreCase(action)) {
                MqttClientState connectionState = (MqttClientState) intent.getSerializableExtra(MqttBroadCastService.KEY_STATE);
                setupConnectionStatePart(connectionState);
                return;
            }
            if (MqttBroadCastService.ACTION_CLIEND_ID.equalsIgnoreCase(action)) {
                String clientID = intent.getStringExtra(MqttBroadCastService.KEY_CLIENT_ID);
                showGeneratedClientId(clientID);
                return;
            }
        }

        private void updateMainUIThread(int what){

            Message msg = Message.obtain();
            msg.what = what;

            handler.sendMessage(msg);

        }

        private void updateMainUIThread(int what, Object obj){

            Message msg = Message.obtain();
            msg.what = what;

            msg.obj  = obj;

            handler.sendMessage(msg);

        }
    }
}


class mqttPublisher {

    private ServiceConnecter serviceConnecter;
    private boolean mqttOption_retain;
    private int mqttOption_Qos_Level;

    private ArrayList<MqttTopic> topicList;

    public mqttPublisher(Context ctx, String[] topic){

        serviceConnecter = new ServiceConnecter();
        serviceConnecter.bind(ctx);

        mqttOption_retain = false;
        mqttOption_Qos_Level=0;
        topicList = new ArrayList<MqttTopic>();

        for(int i=0; i<topic.length; i++){
            topicList.add(new MqttTopic(topic[i]));
        }
    }


    public void onTopicSubmitted(String topicName) {

        if (!serviceConnecter.IsConnected()) {
          //  Log.d("OPEL", "Your are not connected");
            return;
        }
        MqttTopic tp = new MqttTopic(topicName);

        serviceConnecter.subscribeToTopic(tp);

    }

    public void onPushMessage(String message, String topicName) {
        if (!serviceConnecter.IsConnected()) {
         //   Log.d("OPEL", "Your are not connected2");
            return;
        }

        if (message == null || message.length() < 1 || topicName == null || topicName.length() < 1) {
         //   Log.d("OPEL", "Topic or message cannot be empty");
        }

        else {

            MqttMessage mqttMessage = new MqttMessage();
            mqttMessage.setContent(message);
            mqttMessage.setRetains(mqttOption_retain); ///retain option
            mqttMessage.setQos(mqttOption_Qos_Level); //0, 1, 2 Qos level
            MqttTopic mqttTopic = new MqttTopic(topicName);
            serviceConnecter.publish(mqttMessage, mqttTopic);

        }
    }
    public void onPushByteArray(byte[] byteArray, String topicName) {
        if (!serviceConnecter.IsConnected()) {
         //   Log.d("OPEL", "Your are not connected2");
            return;
        }

        if (byteArray == null || byteArray.length < 1 || topicName == null || topicName.length() < 1) {
            Log.d("OPEL", "Topic or message cannot be empty");
        }

        else {

            MqttMessage mqttMessage = new MqttMessage();
            mqttMessage.setContent(byteArray);
            mqttMessage.setRetains(mqttOption_retain); ///retain option
            mqttMessage.setQos(mqttOption_Qos_Level); //0, 1, 2 Qos level
            MqttTopic mqttTopic = new MqttTopic(topicName);
            serviceConnecter.publish(mqttMessage, mqttTopic);

        }
    }
    class ServiceConnecter implements ServiceConnection {

        private MqttBroadCastService service;
        private boolean isBound = false;

        public void bind(Context context) {
            Intent intent = new Intent(context, MqttBroadCastService.class);
            context.bindService(intent, this, Context.BIND_AUTO_CREATE);
          //  Log.d("OPEL", "bind");
        }

        public void unbind(Context context) {
            context.unbindService(this);
            Log.d("OPEL", "unbind");
        }

        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            @SuppressWarnings("unchecked")
            LocalBinder<MqttBroadCastService> localBinder = (LocalBinder<MqttBroadCastService>) binder;
            service = localBinder.getService();
         //   Log.d("OPEL", "onServiceConnceted");

            isBound = true;

        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            isBound = false;

        }

        public String getGeneratedClientId() {
            if (isBound) {
                return service.getGeneratedClient();
            }
            return null;
        }

        public void subscribeToTopic(MqttTopic mqttTopic) {
            if (isBound) {
                service.subscribeToTopic(mqttTopic);

            }
        }

        public void publish(MqttMessage mqttMessage, MqttTopic topic) {
            if (isBound) {
                service.publish(mqttMessage, topic);
            }
        }

        public boolean IsConnected() {
            if(service == null){
          //      Log.d("OPEL", "Service is null");
                return false;

            }
            return this.service.isConnected();
        }
    }
}

class LocalBinder<S extends Service> extends Binder
{
    private WeakReference<S> mService;

    public LocalBinder(S service)
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




