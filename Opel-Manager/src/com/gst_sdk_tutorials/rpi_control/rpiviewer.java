package com.gst_sdk_tutorials.rpi_control;


import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.StrictMode;
import android.os.SystemClock;
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

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.UnknownHostException;

import static android.R.attr.port;


public class rpiviewer extends Activity implements SurfaceHolder.Callback,
        TCPStreaming.WidiStateListener {
    private native void nativeInit(String addr, String vport);     //
    // Initialize native code, build pipeline, etc

    private native void nativeFinalize(); // Destroy pipeline and shutdown
    // native code

    private native void nativeSetUri(String uri); // Set the URI of the media
    // to play

    private native void nativePlay();     // Set pipeline to PLAYING

    private native void nativePause();    // Set pipeline to PAUSED

    private static native boolean nativeClassInit(); // Initialize native
    // class: cache Method IDs for callbacks

    private native void nativeSurfaceInit(Object surface); // A new surface
    // is available

    private native void nativeSurfaceFinalize(); // Surface about to be
    // destroyed

    private long native_custom_data;      // Native code will use this to
    // keep private data


    private boolean is_playing_desired;   // Whether the user asked to go to
    // PLAYING
    private int position;                 // Current position, reported by
    // native code
    private int duration;                 // Current clip duration, reported
    // by native code
    private boolean is_local_media;       // Whether this clip is stored
    // locally or is being streamed
    private int desired_position;         // Position where the users wants
    // to seek to
    private String mediaUri;              // URI of the clip being played

    private final String defaultMediaUri = "http://docs.gstreamer" +
            ".com/media/sintel_trailer-368p.ogv";
    private String vport = "5000";

    private EditText editTextIPAddress;
    private InputMethodManager imm;
    private String server = "192.168.49.1";
    private int port = 5000;

    private Button buttonRecord;
    private Button buttonCapture;

    private Object mDelayedSurface;

    // TCP Handling
    TCPStreaming tcpstreaming;
    private byte[] frame; // Maximum 1Frame JPEG SIZE of 1080P
    private TcpHandler handler;

    class TcpHandler extends Handler {
        public boolean processing;

        public TcpHandler() {
            processing = false;
        }

        public void handleMessage(Message msg) {
            switch (msg.what) {
                case TCPStreaming.MSG_TYPE_STAT_CHANGED:
                    break;
                case TCPStreaming.MSG_TYPE_STAT_READ:
                    processing = true;
                    processing = false;
                    break;
            }
        }
    }

    // WakeLock
    private PowerManager.WakeLock mCpuWakeLock;

    private void acquireCpuWakeLock() {
        Context context = this;
        PowerManager pm = (PowerManager) context.getSystemService(Context
                .POWER_SERVICE);
        mCpuWakeLock = pm.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK |
                PowerManager.ACQUIRE_CAUSES_WAKEUP | PowerManager
                .ON_AFTER_RELEASE, "OPEL Camera Wakelock");
        mCpuWakeLock.acquire();
    }

    private void releaseCpuWakeLock() {
        if (mCpuWakeLock != null) mCpuWakeLock.release();
    }

    // Called when the activity is first created.
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Initialize TCP handling
        handler = new TcpHandler();

        if (globalData.getInstance().getDeviceIP().equals("N/A")) {
            this.finish();
        }

        tcpstreaming = new TCPStreaming(server, port, this, handler);
        tcpstreaming.start();

        // Initialize GStreamer and warn if it fails
        try {
            GStreamer.init(this);
            //Toast.makeText(getApplicationContext(), "Gstreamer Set
            // success", Toast.LENGTH_LONG).show();

        } catch (Exception e) {
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
            finish();
            return;
        }

        // Initialize UI
        setContentView(R.layout.main);

        if (android.os.Build.VERSION.SDK_INT > 9) {
            StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy
                    .Builder().permitAll().build();
            StrictMode.setThreadPolicy(policy);
        }

        editTextIPAddress = (EditText) this.findViewById(R.id
                .editTextIPAddress);
        editTextIPAddress.setText(server);

        SurfaceView sv = (SurfaceView) this.findViewById(R.id.surface_video);
        SurfaceHolder sh = sv.getHolder();
        sh.addCallback(this);

        // Retrieve our previous state, or initialize it to default values
        if (savedInstanceState != null) {
            is_playing_desired = savedInstanceState.getBoolean("playing");
            position = savedInstanceState.getInt("position");
            duration = savedInstanceState.getInt("duration");
            mediaUri = savedInstanceState.getString("mediaUri");
            Log.i("GStreamer", "Activity created with saved state:");
        } else {
            is_playing_desired = false;
            position = duration = 0;
            mediaUri = defaultMediaUri;
            Log.i("GStreamer", "Activity created with no saved state:");
        }
        is_local_media = false;
        Log.i("GStreamer", "  playing:" + is_playing_desired + " position:" +
                position +
                " duration: " + duration + " uri: " + mediaUri);

        // Start with disabled buttons, until native code is initialized
        this.findViewById(R.id.button_capture).setEnabled(false);
        this.findViewById(R.id.button_record).setEnabled(false);
    }

    // Initialize Gstreamer connection
    private void initializeGstreamerConnection() {
        Log.d("rpiviewer", "initializeGstreamerConnection()");
        nativeInit(server, vport);

        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        nativeSurfaceInit(mDelayedSurface);

        is_playing_desired = true;
        imm = (InputMethodManager) getSystemService(Context
                .INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(editTextIPAddress.getWindowToken(), 0);
        nativePlay();
    }


    protected void onSaveInstanceState(Bundle outState) {
        Log.d("GStreamer", "Saving state, playing:" + is_playing_desired + " " +
                "position:" + position +
                " duration: " + duration + " uri: " + mediaUri);
        outState.putBoolean("playing", is_playing_desired);
        outState.putString("mediaUri", mediaUri);
    }

    @Override
    protected void onResume() {
        // Initialize TCP handling
        globalData.getInstance().getCommManager()
                .requestRunNativeJSAppCameraViewer();
        if (tcpstreaming == null) {
            tcpstreaming = new TCPStreaming(server, port, this, handler);
            tcpstreaming.start();
        }

        super.onResume();

        // Acquire wakelock
        this.acquireCpuWakeLock();

        // Initialize Wifi receiver
        registerReceiver(globalData.getInstance().getWifiReceiver(),
                globalData.getInstance().getIntentFilter());
    }

    @Override
    protected void onPause() {
        super.onPause();
        globalData.getInstance().getCommManager()
                .requestTermNativeJSAppCameraViewer();
        globalData.getInstance().getCommManager().opelCommunicator
                .cmfw_wfd_off();
        this.releaseCpuWakeLock();

        // Finalize TCP handling
        if (tcpstreaming != null) {
            tcpstreaming.Cancel();
            tcpstreaming = null;
        }

        // Finalize Wifi receiver
        unregisterReceiver(globalData.getInstance().getWifiReceiver());
    }

    @Override
    protected void onDestroy() {
        nativeFinalize();
        super.onDestroy();
    }

    // Called from native code. This sets the content of the TextView from
    // the UI thread.
    private void setMessage(final String message) {
        final TextView tv = (TextView) this.findViewById(R.id.textview_message);
        runOnUiThread(new Runnable() {
            public void run() {
                tv.setText(message);
            }
        });
    }

    // Set the URI to play, and record whether it is a local or remote file
    private void setMediaUri() {
        nativeSetUri(mediaUri);
        is_local_media = mediaUri.startsWith("file://");
    }

    // Called from native code. Native code calls this once it has created
    // its pipeline and the main loop is running, so it is ready to accept
    // commands.
    private void onGStreamerInitialized() {
        Log.i("GStreamer", "GStreamer initialized:");
        Log.i("GStreamer", "  playing:" + is_playing_desired + " position:" +
                position + " uri: " + mediaUri);

        // Restore previous playing state
        setMediaUri();
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
                activity.findViewById(R.id.button_capture).setEnabled(true);
                activity.findViewById(R.id.button_record).setEnabled(true);
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
        Log.d("GStreamer", "GetSurface(): " + holder.getSurface());
        //nativeSurfaceInit(holder.getSurface());
        this.mDelayedSurface = holder.getSurface();

        Log.d("GStreamer", "end surfaceChanged");
    }

    public void surfaceCreated(SurfaceHolder holder) {
        Log.d("GStreamer", "Surface created: " + holder.getSurface());
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.d("GStreamer", "Surface destroyed");
        nativeSurfaceFinalize();
    }

    // Called from native code when the size of the media changes or is first
    // detected.
    // Inform the video surface about the new size and recalculate the layout.
    private void onMediaSizeChanged(int width, int height) {
        Log.i("GStreamer", "Media size changed to " + width + "x" + height);
        final GStreamerSurfaceView gsv = (GStreamerSurfaceView) this
                .findViewById(R.id.surface_video);
        gsv.media_width = width;
        gsv.media_height = height;
        runOnUiThread(new Runnable() {
            public void run() {
                gsv.requestLayout();
            }
        });
    }

    void exitFromRunLoop() {
    }

    public void onWidiReady() {
        Log.d("rpiviewer", "onWidiReady()");
        this.initializeGstreamerConnection();
    }
}

class TCPStreaming extends Thread {
    interface WidiStateListener {
        public void onWidiReady();
    }

    private Socket tcpSocket;
    private SocketAddress sock_addr;
    private InputStream mInStream;
    String ip;
    int port;
    boolean sch;
    private short stat;
    static public short STAT_DISCON = 0;
    static public short STAT_CONNECTING = 1;
    static public short STAT_CONNECTED = 2;

    static public final int MSG_TYPE_STAT_CHANGED = 0;
    static public final int MSG_TYPE_STAT_READ = 1;
    private rpiviewer.TcpHandler mHandler;
    public byte frames[][];

    private WidiStateListener mListener;

    public TCPStreaming(String ip, int port, WidiStateListener listener,
                        rpiviewer.TcpHandler handler) {
        mHandler = handler;
        this.ip = new String(ip);
        this.port = port;
        stat = STAT_DISCON;
        //Socket tmpSock = null;
        sock_addr = null;
        frames = new byte[2][];

        sch = false;

        this.mListener = listener;
    }

    public void onStatChanged(short stat) {
      /*
       * Should implement this
       */

    }

    public void onReceived(byte[] frame) {
      /*
       * Should implement this
       */
        if (mHandler.processing == false)
            mHandler.obtainMessage(MSG_TYPE_STAT_READ, frame).sendToTarget();
    }

    public void Cancel() {
        sch = false;
    }

    public void run() {

        short prev_stat = stat;
        sch = true;
        while (sch) {
            if (tcpSocket == null) {
                while (globalData.getInstance().getCommManager()
                        .opelCommunicator.cmfw_wfd_on(false) < 0) {
                    try {
                        sleep(1000);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
//                connect();
                // Forward to GStreamer RPI Viewer
                Log.d("rpiviewer", "TCPStreaming.run()");
                mListener.onWidiReady();

                if (tcpSocket == null || tcpSocket.isConnected() == false)
                    break;
            }
            if (prev_stat != stat) {
                prev_stat = stat;
                onStatChanged(stat);
            }
            byte[] frame = null;
            while (sch && stat == STAT_CONNECTED) {
                if (tcpSocket.isConnected() == false) {
                    stat = STAT_DISCON;
                    break;
                }

                try {
                    ByteArrayOutputStream baos = new ByteArrayOutputStream();
                    DataInputStream bis = new DataInputStream(mInStream);
                    int len = bis.readInt();
                    Log.d("Streaming", "Frame Len:" + Integer.toString(len));
                    frame = new byte[len];
                    bis.readFully(frame);

                    onReceived(frame);

                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }

        try {
            if (null != tcpSocket) {
                tcpSocket.close();
                tcpSocket = null;
            }
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
}
