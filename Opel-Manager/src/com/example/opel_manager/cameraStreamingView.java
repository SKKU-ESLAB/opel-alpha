package com.example.opel_manager;

import android.app.ActionBar;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.Image;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.Log;
import android.view.MenuItem;
import android.widget.ImageView;
import android.widget.LinearLayout;

import java.io.BufferedInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;

public class cameraStreamingView extends Activity{
	TCPStreaming tcpstreaming;
	private byte[] frame; // Maximum 1Frame JPEG SIZE of 1080P
	private TcpHandler handler;
	long curr, prev;

	int idx = 0;

	String ip = "192.168.49.1";
	int port = 9488;

	class TcpHandler extends Handler{
		public boolean processing;
		public TcpHandler(){
			processing = false;
		}
		public void handleMessage(Message msg){
			switch(msg.what){
				case TCPStreaming.MSG_TYPE_STAT_CHANGED:
					break;
				case TCPStreaming.MSG_TYPE_STAT_READ:
					processing = true;
					setImg((byte[])msg.obj);
					processing = false;
					break;
			}
		}
	}
	@Override
	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		handler = new TcpHandler();

		if(globalData.getInstance().getDeviceIP().equals("N/A")){
			this.finish();
		}

		setContentView(R.layout.camera_stream);


		ActionBar actionBar = getActionBar();
		actionBar.setTitle("Camera Viewer");
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setLogo(R.drawable.cam);
		actionBar.setDisplayUseLogoEnabled(true);

		//tcpstreaming = new TCPStreaming(ip, port, handler);

		//Network Connection Initialization PSE;
		//Network Connection Start PSE;
		//handler.postDelayed(changeImage, 100); /* 10FPS */
		curr = SystemClock.elapsedRealtime();
		prev = SystemClock.elapsedRealtime();;

		//tcpstreaming.start();
	}

	@Override
	protected void onResume() {
		// TODO Auto-generated method stub


		super.onResume();
	}

	@Override
	public void onBackPressed() {
		// TODO Auto-generated method stub
		super.onBackPressed();
	}
	@Override
	protected void onPause() {
		// TODO Auto-generated method stub

		if(tcpstreaming != null){
			tcpstreaming.Cancel();
			tcpstreaming = null;
		}
		super.onPause();
	}
	synchronized private void setImg(byte[] frame)
	{
		final ImageView imageView = (ImageView) findViewById(R.id.imageView);
		//get Image Stream over Network 'Frame' is Image byte
		Bitmap bm = null;

		prev = curr;
		curr = SystemClock.elapsedRealtime();
		if((curr - prev) < 40) {
			Log.d("Streaming", "Skip");
			return;
		}

		bm = BitmapFactory.decodeByteArray(frame, 0, frame.length); //decode Stream
		if( null == bm) {
			Log.d("STreaming", "Failed");
			return;
		}

		//imageView.setImageResource(images[idx]); Decode Stream\

		imageView.setImageBitmap(bm); //displaying the Bitmap Image
		//imageView.setRotation(90);
	}

	Runnable changeImage = new Runnable() {
		@Override
		public void run() {
         /*
         if(idx == 0) {
            setImg(idx);
            idx=1;
         }
         else {
            setImg(idx);
            idx=0;
         }
         */
			handler.postDelayed(changeImage, 33); /* 10FPS */
		}
	};
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}

		else if( id == android.R.id.home){
			this.finish();
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

}


class TCPStreaming extends Thread {
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
	private cameraStreamingView.TcpHandler mHandler;
	public byte frames[][];

	public TCPStreaming(String ip, int port, cameraStreamingView.TcpHandler handler){
		mHandler = handler;
		this.ip = new String(ip);
		this.port = port;
		stat = STAT_DISCON;
		//Socket tmpSock = null;
		sock_addr = null;
		frames = new byte[2][];
      /*try {
         Log.d("What the", "Msg1");
         tmpSock = new Socket(ip, port);
         sock_addr = tmpSock.getRemoteSocketAddress();
         if(tmpSock.isConnected()){
            stat = STAT_CONNECTED;
            mInStream = tmpSock.getInputStream();
            onStatChanged(stat);
         }
      } catch (UnknownHostException e) {
         // TODO Auto-generated catch block
         e.printStackTrace();
      } catch (IOException e) {
         // TODO Auto-generated catch block
         e.printStackTrace();
      }
      tcpSocket = tmpSock;
      */

		sch = false;
	}


	public void connect(){
		Socket tmpSock = null;


		try {
			Log.d("What the", "Msg1");
			tmpSock = new Socket(ip, port);
			sock_addr = tmpSock.getRemoteSocketAddress();
			if(tmpSock.isConnected()){
				stat = STAT_CONNECTED;
				mInStream = tmpSock.getInputStream();
				onStatChanged(stat);
			}
			else
				tmpSock = null;
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		tcpSocket = tmpSock;
	}
	public void connect(String ip, int port){
		if(stat != STAT_DISCON){
			Log.d("TCPStreaming", "Already connecting or connected");
			return;
		}

		if(tcpSocket == null){
			Socket tmpSock = null;
			try {
				stat = STAT_CONNECTING;
				tmpSock = new Socket(ip, port);
				sock_addr = tmpSock.getRemoteSocketAddress();
				if(tmpSock.isConnected()){
					stat = STAT_CONNECTED;
					mInStream = tmpSock.getInputStream();
					onStatChanged(stat);
				}
			} catch (UnknownHostException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			tcpSocket = tmpSock;
		}
		else{
			stat = STAT_CONNECTING;
			try {
				tcpSocket.connect(sock_addr);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			if(tcpSocket.isConnected()){
				stat = STAT_CONNECTED;
				try {
					mInStream = tcpSocket.getInputStream();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				onStatChanged(stat);
			}
		}

	}
	public void onStatChanged(short stat){
      /*
       * Should implement this
       */

	}
	public void onReceived(byte[] frame){
      /*
       * Should implement this
       */
		if(mHandler.processing == false)
			mHandler.obtainMessage(MSG_TYPE_STAT_READ, frame).sendToTarget();
	}

	public void Cancel(){
		sch = false;
	}

	public void run(){

		short prev_stat = stat;
		sch = true;
		while(sch){
			if(tcpSocket == null) {
				globalData.getInstance().getCommManager().opelCommunicator.cmfw_wfd_on();
				connect();
				if(tcpSocket == null || tcpSocket.isConnected() == false)
					try {
						this.sleep(1000,0);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				continue;
			}
			if(prev_stat != stat){
				prev_stat = stat;
				onStatChanged(stat);
			}
			byte[] frame = null;
			while(sch && stat == STAT_CONNECTED){
				if(tcpSocket.isConnected() == false){
					stat = STAT_DISCON;
					break;
				}


				try {

					ByteArrayOutputStream baos = new ByteArrayOutputStream();
					DataInputStream bis = new DataInputStream(mInStream);
					int len = bis.readInt();
					Log.d("Streaming", "Frame Len:"+Integer.toString(len));
					frame = new byte[len];
					bis.readFully(frame);
					//Log.d("TCPStreaming", offset+"/"+totalLen);

					onReceived(frame);
               /*
               try {
                  sleep(33);
               } catch (InterruptedException e) {
                  e.printStackTrace();
               }*/




				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}

			}



		}

		try {
			if(null != tcpSocket) {
				tcpSocket.close();
				tcpSocket = null;
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		globalData.getInstance().getCommManager().opelCommunicator.cmfw_wfd_off();
	}
}