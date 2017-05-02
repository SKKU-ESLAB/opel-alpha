package com.opel.opel_manager.controller.mqtt;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.Parcelable;

import com.opel.opel_manager.view.MainActivity;
import com.opel.opel_manager.R;

import org.sidibe.mqtt.android.lib.MqttAndroidClient;
import org.sidibe.mqtt.android.lib.MqttClientState;
import org.sidibe.mqtt.android.lib.MqttMessage;
import org.sidibe.mqtt.android.lib.MqttPushEventListener;
import org.sidibe.mqtt.android.lib.MqttTopic;

import java.util.List;


public class MQTTBroadCastService extends Service implements MqttPushEventListener {

    private static final int NOTIFICATION_ID = 0x1212;
    private MqttAndroidClient mqttClient;
    public static final String ACTION_PUSH_ARRIVED = "action_push_arrived";
    public static final String ACTION_STATE_CHANGED = "action_state_changed";
    public static final String ACTION_CLIEND_ID = "action_client_id";
    public static final String KEY_CLIENT_ID = "key_client_id";
    public static final String ACTION_PUSH_SEND = "action_push_send";
    public static final String KEY_STATE = "key_state";
    public static final String KEY_DATA = "key_data";
    static final String HOST = "115.145.178.127";
    static final String TOPIC_KEY = "key_topic";

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        mqttClient = new MqttAndroidClient(this, HOST);
        mqttClient.addPushListener(this);
        mqttClient.setCleanOnStart(true);
        mqttClient.bind(new MqttTopic("kalana", 2));
        mqttClient.start();
        return START_REDELIVER_INTENT;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return new MQTTBinder<Service>(this);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mqttClient.disconnect();
    }

    @Override
    public void onPushMessageReceived(MqttMessage mqttMessage) {
        Intent broadcastIntent = new Intent(ACTION_PUSH_ARRIVED);
        broadcastIntent.putExtra(KEY_DATA, (Parcelable) mqttMessage);
        sendBroadcast(broadcastIntent);
        showMessageAsNotification(mqttMessage);
    }

    private void showMessageAsNotification(MqttMessage mqttMessage) {
        NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

        Notification notification = new Notification.Builder(this)
                .setContentTitle("Mqtt Message")
                .setContentText(mqttMessage.getContentText())
                .setSmallIcon(R.drawable.ic_launcher).build();
        Intent notificationIntent = new Intent(this, MainActivity.class);
        notificationIntent.putExtra(KEY_DATA, (Parcelable) mqttMessage);
        PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
                notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        notification.contentIntent = contentIntent;
        nm.notify(NOTIFICATION_ID + 1, notification);

    }

    private void showClientStateAsNotification(MqttClientState clientState) {
        NotificationManager nm = (NotificationManager) getSystemService(NOTIFICATION_SERVICE);
        Notification notification = new Notification.Builder(this)
                .setContentTitle("Mqtt Status")
                .setContentText(clientState.name())
                .setSmallIcon(R.drawable.ic_launcher).build();
        Intent notificationIntent = new Intent(this, MainActivity.class);
        notificationIntent.putExtra(KEY_STATE, clientState);
        PendingIntent contentIntent = PendingIntent.getActivity(this, 0,
                notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        notification.contentIntent = contentIntent;
        nm.notify(NOTIFICATION_ID, notification);

    }

    private boolean connected;

    @Override
    public void onConnectionStateChanged(MqttClientState connectionState) {

        connected = connectionState == MqttClientState.CONNECTED;
        Intent broadcastIntent = new Intent(ACTION_STATE_CHANGED);
        broadcastIntent.putExtra(KEY_STATE, connectionState);
        sendBroadcast(broadcastIntent);
        showClientStateAsNotification(connectionState);

    }

    private void sendGeneratedClientId(List<String> strings) {
        //Log.i("Clien ID", strings.toString());
        Intent broadcastIntent = new Intent(ACTION_CLIEND_ID);
        broadcastIntent.putExtra(KEY_CLIENT_ID, strings.toString());
        sendBroadcast(broadcastIntent);
    }

    public void subscribeToTopic(MqttTopic mqttTopic) {
        mqttClient.bind(mqttTopic);
    }

    public String getGeneratedClient() {
        return mqttClient.getIdClient();
    }

    public void publish(MqttMessage mqttMessage, MqttTopic topic) {
        mqttClient.push(mqttMessage, topic);
    }

    public boolean isConnected() {
        return connected;

    }

    @Override
    public void onClientConnectedOnBroker(List<String> clienIds) {
        sendGeneratedClientId(clienIds);

    }
}
