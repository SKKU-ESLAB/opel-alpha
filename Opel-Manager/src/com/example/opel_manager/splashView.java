package com.example.opel_manager;

import android.os.Bundle;
import android.app.Activity;
import android.os.Handler;
import android.os.Message;

public class splashView extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash_viewer);

        Handler handler = new Handler(){
          public void handleMessage(Message msg){
              finish();
          }
        };

        handler.sendEmptyMessageAtTime(0, 1500);
    }

}
