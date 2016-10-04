package com.gst_sdk_tutorials.rpi_control;


import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.StrictMode;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import com.example.opel_manager.R;
import com.example.opel_manager.globalData;

import org.freedesktop.gstreamer.GStreamer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;



public class rpiviewer extends Activity implements SurfaceHolder.Callback {
    private native void nativeInit(String addr, String vport);     // Initialize native code, build pipeline, etc
    private native void nativeFinalize(); // Destroy pipeline and shutdown native code
    private native void nativeSetUri(String uri); // Set the URI of the media to play
    private native void nativePlay();     // Set pipeline to PLAYING
    private native void nativePause();    // Set pipeline to PAUSED
    private static native boolean nativeClassInit(); // Initialize native class: cache Method IDs for callbacks
    private native void nativeSurfaceInit(Object surface); // A new surface is available
    private native void nativeSurfaceFinalize(); // Surface about to be destroyed
    private long native_custom_data;      // Native code will use this to keep private data


    private boolean is_playing_desired;   // Whether the user asked to go to PLAYING
    private int position;                 // Current position, reported by native code
    private int duration;                 // Current clip duration, reported by native code
    private boolean is_local_media;       // Whether this clip is stored locally or is being streamed
    private int desired_position;         // Position where the users wants to seek to
    private String mediaUri;              // URI of the clip being played

    private final String defaultMediaUri = "http://docs.gstreamer.com/media/sintel_trailer-368p.ogv";
    private String vport="5000";

    private EditText editTextIPAddress;
    private InputMethodManager imm;
    private String server  = "192.168.49.1";
    private int port = 5000;
	private Socket socket;
	private OutputStream outs;

	private Button buttonUp;
	private Button buttonLeftTurn;
	private Button buttonRightTurn;
	private Button buttonDown;
	private Button buttonCenter;
	private Button buttonLedOn;
	private Button buttonLedOff;
	private Button buttonCamUp;
	private Button buttonCamDown;

	// WakeLock
	private PowerManager.WakeLock mCpuWakeLock;

	private void acquireCpuWakeLock() {
		Context context = this;
		PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
		mCpuWakeLock = pm.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK
						| PowerManager.ACQUIRE_CAUSES_WAKEUP
						| PowerManager.ON_AFTER_RELEASE,
				"OPEL Camera Wakelock");
		mCpuWakeLock.acquire();
	}

	private void releaseCpuWakeLock() {
		if(mCpuWakeLock != null)
			mCpuWakeLock.release();
	}

    // Called when the activity is first created.
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        // Initialize GStreamer and warn if it fails
        try {
            GStreamer.init(this);
            //Toast.makeText(getApplicationContext(), "Gstreamer Set success", Toast.LENGTH_LONG).show();

        } catch (Exception e) {
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        setContentView(R.layout.main);

//        Intent intent = getIntent();
//
//        server = intent.getStringExtra("server");
//        vport = intent.getStringExtra("port");

        if (android.os.Build.VERSION.SDK_INT > 9) {
            StrictMode.ThreadPolicy policy =
                    new StrictMode.ThreadPolicy.Builder().permitAll().build();
            StrictMode.setThreadPolicy(policy);
        }





        editTextIPAddress = (EditText)this.findViewById(R.id.editTextIPAddress);
		editTextIPAddress.setText(server);
        imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);

        ImageButton play = (ImageButton) this.findViewById(R.id.button_play);
        ImageButton pause = (ImageButton) this.findViewById(R.id.button_stop);
        buttonUp        = (Button)this.findViewById(R.id.buttonUp);
		buttonLeftTurn  = (Button)this.findViewById(R.id.buttonLeftTurn);
		buttonRightTurn = (Button)this.findViewById(R.id.buttonRightTurn);
		buttonDown      = (Button)this.findViewById(R.id.buttonDown);
		buttonCenter    = (Button)this.findViewById(R.id.buttonCenter);
		buttonLedOn     = (Button)this.findViewById(R.id.buttonLedOn);
		buttonLedOff    = (Button)this.findViewById(R.id.buttonLedOff);
		buttonCamUp     = (Button)this.findViewById(R.id.buttonCamUp);
		buttonCamDown    = (Button)this.findViewById(R.id.buttonCamDown);


        play.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {

            	imm.hideSoftInputFromWindow(editTextIPAddress.getWindowToken(), 0);

            	try{

        			if(socket!=null)
        			{
        				socket.close();
        				socket = null;
        			}

        			server = editTextIPAddress.getText().toString();


        			socket = new Socket(server, port);

        			outs = socket.getOutputStream();

        		} catch (UnknownHostException e) {

        			Toast.makeText(getApplicationContext(), "Socket error", Toast.LENGTH_LONG).show();

        		} catch (IOException e){


        			e.printStackTrace();
        		}


                is_playing_desired = true;
                nativePlay();
            }
        });


        pause.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {

            	imm.hideSoftInputFromWindow(editTextIPAddress.getWindowToken(), 0);

    			if(socket!=null)
    			{
    				exitFromRunLoop();
    				try{
    					socket.close();
    					socket = null;


    				} catch (IOException e){

    					e.printStackTrace();
    				}
    			}

                is_playing_desired = false;
                nativePause();
            }
        });



        buttonUp.setOnTouchListener(new View.OnTouchListener() {
			public boolean onTouch(View v, MotionEvent event) {

				String sndOpkey;

				switch(event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					sndOpkey = "Up";

					try{
						outs.write(sndOpkey.getBytes("UTF-8"));
						outs.flush();
					} catch (IOException e){
						e.printStackTrace();
    				}

					break;

				case MotionEvent.ACTION_UP:
					sndOpkey = "Stop";
					try{
						outs.write(sndOpkey.getBytes("UTF-8"));
						outs.flush();
					} catch (IOException e){
						e.printStackTrace();
    				}

					break;
				}
				return true;
			}
        });


        buttonDown.setOnTouchListener(new View.OnTouchListener() {
			public boolean onTouch(View v, MotionEvent event) {

				String sndOpkey;

				switch(event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					sndOpkey = "Down";
					try{
						outs.write(sndOpkey.getBytes("UTF-8"));
						outs.flush();
					} catch (IOException e){
						e.printStackTrace();
    				}
					break;
				case MotionEvent.ACTION_UP:
					sndOpkey = "Stop";
					try{
						outs.write(sndOpkey.getBytes("UTF-8"));
						outs.flush();
					} catch (IOException e){
						e.printStackTrace();
    				}
					break;
				}
				return true;
			}
        });



        buttonLeftTurn.setOnTouchListener(new View.OnTouchListener() {
			public boolean onTouch(View v, MotionEvent event) {

				String sndOpkey;

				switch(event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					sndOpkey = "Left";

					try{
						outs.write(sndOpkey.getBytes("UTF-8"));
						outs.flush();
					} catch (IOException e){
						e.printStackTrace();
    				}
					break;
				case MotionEvent.ACTION_UP:
					sndOpkey = "Stop";
					try{
						outs.write(sndOpkey.getBytes("UTF-8"));
						outs.flush();
					} catch (IOException e){
						e.printStackTrace();
    				}
					break;
				}
				return true;
			}
        });


        buttonRightTurn.setOnTouchListener(new View.OnTouchListener() {
			public boolean onTouch(View v, MotionEvent event) {

				String sndOpkey;

				switch(event.getAction()) {
				case MotionEvent.ACTION_DOWN:
					sndOpkey = "Right";
					try{
						outs.write(sndOpkey.getBytes("UTF-8"));
						outs.flush();
					} catch (IOException e){
						e.printStackTrace();
    				}
					break;
				case MotionEvent.ACTION_UP:
					sndOpkey = "Stop";
					try{
						outs.write(sndOpkey.getBytes("UTF-8"));
						outs.flush();
					} catch (IOException e){
						e.printStackTrace();
    				}
					break;
				}
				return true;
			}
        });

        buttonCenter.setOnClickListener(new OnClickListener() {
		    public void onClick(View v) {

		    	String sndOpkey = "Stop";

				try{
					outs.write(sndOpkey.getBytes("UTF-8"));
					outs.flush();
				} catch (IOException e){

					e.printStackTrace();
				}
            }
        });

        buttonLedOn.setOnClickListener(new OnClickListener() {
		    public void onClick(View v) {

		    	String sndOpkey = "Led On";

				try{
					outs.write(sndOpkey.getBytes("UTF-8"));
					outs.flush();
				} catch (IOException e){

					e.printStackTrace();
				}
            }
        });

        buttonLedOff.setOnClickListener(new OnClickListener() {
		    public void onClick(View v) {

		    	String sndOpkey = "Led Off";

				try{
					outs.write(sndOpkey.getBytes("UTF-8"));
					outs.flush();
				} catch (IOException e){

					e.printStackTrace();
				}
            }
        });

        buttonCamUp.setOnClickListener(new OnClickListener() {
		    public void onClick(View v) {

		    	String sndOpkey = "Cam Up";

				try{
					outs.write(sndOpkey.getBytes("UTF-8"));
					outs.flush();
				} catch (IOException e){

					e.printStackTrace();
				}
            }
        });


        buttonCamDown.setOnClickListener(new OnClickListener() {
		    public void onClick(View v) {

		    	String sndOpkey = "Cam Down";

				try{
					outs.write(sndOpkey.getBytes("UTF-8"));
					outs.flush();
				} catch (IOException e){

					e.printStackTrace();
				}
            }
        });

        SurfaceView sv = (SurfaceView) this.findViewById(R.id.surface_video);
        SurfaceHolder sh = sv.getHolder();
        sh.addCallback(this);

         // Retrieve our previous state, or initialize it to default values
        if (savedInstanceState != null) {
            is_playing_desired = savedInstanceState.getBoolean("playing");
            position = savedInstanceState.getInt("position");
            duration = savedInstanceState.getInt("duration");
            mediaUri = savedInstanceState.getString("mediaUri");
            Log.i ("GStreamer", "Activity created with saved state:");
        } else {
            is_playing_desired = false;
            position = duration = 0;
            mediaUri = defaultMediaUri;
            Log.i ("GStreamer", "Activity created with no saved state:");
        }
        is_local_media = false;
        Log.i ("GStreamer", "  playing:" + is_playing_desired + " position:" + position +
                " duration: " + duration + " uri: " + mediaUri);

        // Start with disabled buttons, until native code is initialized
        this.findViewById(R.id.button_play).setEnabled(false);
        this.findViewById(R.id.button_stop).setEnabled(false);

        nativeInit(server, vport);

    }



    protected void onSaveInstanceState (Bundle outState) {
        Log.d ("GStreamer", "Saving state, playing:" + is_playing_desired + " position:" + position +
                " duration: " + duration + " uri: " + mediaUri);
        outState.putBoolean("playing", is_playing_desired);
        outState.putString("mediaUri", mediaUri);
    }

	@Override
	protected void onResume() {
		super.onResume();
		this.acquireCpuWakeLock();
	}

	@Override
	protected void onPause() {
		super.onPause();
		globalData.getInstance().getCommManager().requestTermNativeJSAppCameraViewer();
		globalData.getInstance().getCommManager().opelCommunicator.cmfw_wfd_off();
		this.releaseCpuWakeLock();
	}

	@Override
    protected void onDestroy() {
        nativeFinalize();
        super.onDestroy();
    }

    // Called from native code. This sets the content of the TextView from the UI thread.
    private void setMessage(final String message) {
        final TextView tv = (TextView) this.findViewById(R.id.textview_message);
        runOnUiThread (new Runnable() {
          public void run() {
            tv.setText(message);
          }
        });
    }

    // Set the URI to play, and record whether it is a local or remote file
    private void setMediaUri() {
        nativeSetUri (mediaUri);
        is_local_media = mediaUri.startsWith("file://");
    }

    // Called from native code. Native code calls this once it has created its pipeline and
    // the main loop is running, so it is ready to accept commands.
    private void onGStreamerInitialized () {
        Log.i ("GStreamer", "GStreamer initialized:");
        Log.i ("GStreamer", "  playing:" + is_playing_desired + " position:" + position + " uri: " + mediaUri);

        // Restore previous playing state
        setMediaUri ();
        //nativeSetPosition (position);
        if (is_playing_desired) {
            nativePlay();
        } else {
            nativePause();
        }

        // Re-enable buttons, now that GStreamer is initialized
        final Activity activity = this;
        runOnUiThread(new Runnable() {
            public void run() {
                activity.findViewById(R.id.button_play).setEnabled(true);
                activity.findViewById(R.id.button_stop).setEnabled(true);
            }
        });
    }

    static {
        System.loadLibrary("gstreamer_android");
        System.loadLibrary("tutorial-4");
        nativeClassInit();
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int width,
            int height) {
        Log.d("GStreamer", "Surface changed to format " + format + " width "
                + width + " height " + height);
        nativeSurfaceInit (holder.getSurface());
    }

    public void surfaceCreated(SurfaceHolder holder) {
        Log.d("GStreamer", "Surface created: " + holder.getSurface());
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d("GStreamer", "Surface destroyed");
        nativeSurfaceFinalize ();
    }

    // Called from native code when the size of the media changes or is first detected.
    // Inform the video surface about the new size and recalculate the layout.
    private void onMediaSizeChanged (int width, int height) {
        Log.i ("GStreamer", "Media size changed to " + width + "x" + height);
        final GStreamerSurfaceView gsv = (GStreamerSurfaceView) this.findViewById(R.id.surface_video);
        gsv.media_width = width;
        gsv.media_height = height;
        runOnUiThread(new Runnable() {
            public void run() {
                gsv.requestLayout();
            }
        });
    }

    void exitFromRunLoop(){
    	try {
    		String sndOpkey = "[close]";
    		outs.write(sndOpkey.getBytes("UTF-8"));
    		outs.flush();
    	} catch (IOException e) {

			e.printStackTrace();
    	}
    }

}
