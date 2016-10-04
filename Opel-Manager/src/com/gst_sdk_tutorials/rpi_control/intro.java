package com.gst_sdk_tutorials.rpi_control;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;

import com.example.opel_manager.R;


public class intro extends Activity  {
  
    private EditText editTextIPAddress;
    private EditText editTextPortAddress;
    private InputMethodManager imm;
    private String server  = "192.168.0.100";
    private String server1 = "000.000.000.000";
    private String server2 = "192.168.219.176";
    private String server3 = "192.168.0.100";
    private String port  = "5000";
    private String port1  = "8554";
    private String port2  = "8554";
    private String port3  = "5000";
  	
	private Button button1;
	private Button button2;
	private Button button3;
	private Button buttonConnect;

    // Called when the activity is first created.
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
    
        setContentView(R.layout.intro);
 
             
        
        editTextIPAddress = (EditText)this.findViewById(R.id.editTextIPAddress);
		
		
		editTextPortAddress = (EditText)this.findViewById(R.id.editTextPortAddress);
		
		
        imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
        
        button1        = (Button)this.findViewById(R.id.button1);
        button2        = (Button)this.findViewById(R.id.button2);
        button3        = (Button)this.findViewById(R.id.button3);
        buttonConnect  = (Button)this.findViewById(R.id.buttonConnect);
 	
		
		button1.setOnClickListener(new OnClickListener() {
		    public void onClick(View v) {
                
		    	editTextIPAddress.setText(server1);
		    	editTextPortAddress.setText(port1);
		    	
		    	server = editTextIPAddress.getText().toString();
		    	port = editTextPortAddress.getText().toString();
    												
            }
        });
		
		button2.setOnClickListener(new OnClickListener() {
		    public void onClick(View v) {
                
		    	editTextIPAddress.setText(server2);
		    	editTextPortAddress.setText(port2);
		    	
		    	server = editTextIPAddress.getText().toString();
		    	port = editTextPortAddress.getText().toString();
												
            }
        });
		
		button3.setOnClickListener(new OnClickListener() {
		    public void onClick(View v) {
                
		    	editTextIPAddress.setText(server3);
		    	editTextPortAddress.setText(port3);
		    	
		    	server = editTextIPAddress.getText().toString();
		    	port = editTextPortAddress.getText().toString();
												
            }
        });
		
		buttonConnect.setOnClickListener(new OnClickListener() {
		    public void onClick(View v) {
                
		    	Intent intent = new Intent(getApplicationContext(),
                        rpiviewer.class
                    );
		    	
		    	server = editTextIPAddress.getText().toString();
		    	port = editTextPortAddress.getText().toString();
		    	
            intent.putExtra("server", server);
            intent.putExtra("port", port);
            
            startActivity(intent);
												
            }
        });
		
	       
    }
    
    



}
