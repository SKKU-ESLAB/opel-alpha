package com.example.opel_manager;


import android.app.ActionBar;
import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;

import opel.android.comm.OpelCallbacks;
import opel.android.comm.OpelClient;
import opel.android.comm.OpelCommunicator;
import opel.android.comm.OpelMessage;
import opel.android.comm.OpelSCModel;
import opel.android.comm.OpelSocket;


class perSensorData{
	int min;
	int max;
	double curValue;
	String sensorName;

	GraphView graph;
	TextView textV;

	LineGraphSeries<DataPoint> mSeries;

	public perSensorData(int min, int max, String sensorName, GraphView graph, TextView textV){
		this.min=min;
		this.max=max;
		this.graph = graph;
		this.curValue = 0.0;
		this.sensorName = sensorName;
		this.textV = textV;
		this.textV.setText(sensorName);
		mSeries = new LineGraphSeries<DataPoint>();

		graph.addSeries(mSeries);
		graph.getViewport().setXAxisBoundsManual(true);
		graph.getViewport().setMinX(min);
		graph.getViewport().setMaxX(max);

		graph.getViewport().setYAxisBoundsManual(false);

		mSeries.setThickness(5);

	}

	public void setCurValue(double var){
		this.curValue = var;
	}

	public double getCurValue(){
		return this.curValue;
	}


	public void appendData(DataPoint a, boolean b, int c){
		mSeries.appendData(a, b, c);
	}

	public int getMax(){
		return this.max;
	}

	public String getSensorName(){
		return this.sensorName;
	}

	public void setTextViewColor(int color){
		textV.setTextColor(color);
	}

	public void setGraphLineColor(int color){
		mSeries.setColor(color);
	}
}

public class sensorView extends Activity {

	private final Handler mHandler = new Handler();

	private Runnable mTimer2;

	private LineGraphSeries<DataPoint> mSeries2;
	private double graph2LastXValue = 5d;

	perSensorData sensor1;
	perSensorData sensor2;
	perSensorData sensor3;
	perSensorData sensor4;
	perSensorData sensor5;
	perSensorData sensor6;
	perSensorData sensor7;

	String sensorName1 = "Touch";
	String sensorName2 = "Accelerometer";
	String sensorName3 = "Motion";
	String sensorName4 = "Sound";
	String sensorName5 = "Light";
	String sensorName6 = "Vibration";
	String sensorName7 = "Temperature";

	private final String TAG = "SensorViewer";
	private final String INTF_NAME = "Sensor Intf";

	private OpelClient op_cli;
	private OpelCallbacks defCb;
	private OpelCallbacks statChangedCb;

	@Override
	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		setContentView(R.layout.sensorview);

		ActionBar actionBar = getActionBar();
		actionBar.setTitle("Sensor Viewer");
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setLogo(R.drawable.sensor);
		actionBar.setDisplayUseLogoEnabled(true);


		sensor1 = new perSensorData(0, 100, sensorName1, (GraphView) findViewById(R.id.graph1), (TextView) findViewById(R.id.textView11));
		sensor2 = new perSensorData(0, 100, sensorName2, (GraphView) findViewById(R.id.graph2), (TextView) findViewById(R.id.textView22));
		sensor3 = new perSensorData(0, 100, sensorName3, (GraphView) findViewById(R.id.graph3), (TextView) findViewById(R.id.textView33));
		sensor4 = new perSensorData(0, 100, sensorName4, (GraphView) findViewById(R.id.graph4), (TextView) findViewById(R.id.textView4));
		sensor5 = new perSensorData(0, 100, sensorName5, (GraphView) findViewById(R.id.graph5), (TextView) findViewById(R.id.textView5));
		sensor6 = new perSensorData(0, 100, sensorName6, (GraphView) findViewById(R.id.graph6), (TextView) findViewById(R.id.textView6));
		sensor7 = new perSensorData(0, 100, sensorName7, (GraphView) findViewById(R.id.graph7), (TextView) findViewById(R.id.textView7));



		sensor1.setGraphLineColor(Color.RED);
		sensor2.setGraphLineColor(Color.BLUE);
		sensor3.setGraphLineColor(Color.WHITE);
		sensor4.setGraphLineColor(Color.YELLOW);
		sensor5.setGraphLineColor(Color.WHITE);
		sensor6.setGraphLineColor(Color.BLUE);
		sensor7.setGraphLineColor(Color.YELLOW);

		defCb = new OpelCallbacks(){
			public void OpelCb(OpelMessage msg, short err){
				if(err != OpelSCModel.COMM_ERR_NONE){
    			   /*
    			    * Error happens
    			    */
					Log.d(TAG, "Error Happens"+err);
					if(err == OpelClient.COMM_ERR_DISCON){
						if(op_cli != null)
							op_cli.Cancel();
						op_cli = OpelCommunicator.connectChannel(INTF_NAME, defCb, statChangedCb);
					}
				}

				else{
					String message = new String(msg.get_data(), 0, msg.get_data_len());
					Log.d(TAG,  "Messag comming : "+message);
					jsonParser jp = new jsonParser(message);

					String val1 = jp.getValueByKey( sensor1.getSensorName());
					if(val1.equals(""))
						return;
					String val2 = jp.getValueByKey( sensor2.getSensorName());
					String val3 = jp.getValueByKey( sensor3.getSensorName());
					String val4 = jp.getValueByKey( sensor4.getSensorName());
					String val5 = jp.getValueByKey( sensor5.getSensorName());
					String val6 = jp.getValueByKey( sensor6.getSensorName());
					String val7 = jp.getValueByKey( sensor7.getSensorName());

					Log.d(TAG, val1+"/"+val2+"/"+val3+"/"+val4+"/"+val5+"/"+val6+"/"+val7+"/");

					sensor1.setCurValue( Double.parseDouble(jp.getValueByKey( sensor1.getSensorName() )));
					sensor2.setCurValue( Double.parseDouble(jp.getValueByKey( sensor2.getSensorName() )));
					sensor3.setCurValue( Double.parseDouble(jp.getValueByKey( sensor3.getSensorName() )));
					sensor4.setCurValue( Double.parseDouble(jp.getValueByKey( sensor4.getSensorName() )));
					sensor5.setCurValue( Double.parseDouble(jp.getValueByKey( sensor5.getSensorName() )));
					sensor6.setCurValue( Double.parseDouble(jp.getValueByKey( sensor6.getSensorName() )));
					sensor7.setCurValue( Double.parseDouble(jp.getValueByKey( sensor7.getSensorName() )));

				}
			}
		};

		statChangedCb = new OpelCallbacks(){
			public void OpelCb(OpelMessage smg, short stat){
				Log.d(TAG,  "STat Chagned"+stat);
				switch(stat){
					case OpelSocket.STAT_DISCON:
    				/*
    				 * Disconnected operation
    				 */
					case OpelSocket.STAT_CONNECTED:
    				/*
    				 * Connected Operation
    				 */
						op_cli.SendMsg("Connected");

				}
			}
		};

		op_cli = OpelCommunicator.connectChannel(INTF_NAME, defCb, statChangedCb);

		//sensor1.setBgColor(color.darker_gray);
		//sensor1.setColor(Color.GREEN);

		//GraphView graph1 = (GraphView) findViewById(R.id.graph1);
		//mSeries2 = new LineGraphSeries<DataPoint>();
		//graph.addSeries(mSeries2);
		//graph.getViewport().setXAxisBoundsManual(true);
//		graph.getViewport().setMinX(0);
		//	graph.getViewport().setMaxX(40);

		///graph.getViewport().setYAxisBoundsManual(false);

		//graph.getViewport().setMaxY(40);
		
        /*LineGraphSeries<DataPoint> series = new LineGraphSeries<DataPoint>(new DataPoint[] {
                  new DataPoint(0, 1),
                  new DataPoint(1, 5),
                  new DataPoint(2, 3),
                  new DataPoint(3, 2),
                  new DataPoint(4, 6)
        });*/
		//graph.addSeries(series);
	}

	public void onResume() {
		super.onResume();
		globalData.getInstance().getCommManager().requestRunNativeJSAppSensorViewer();
		if(op_cli == null || op_cli.getStat() == OpelSocket.STAT_CONNECTED)
			op_cli = OpelCommunicator.connectChannel(INTF_NAME, defCb, statChangedCb);

		mTimer2 = new Runnable() {
			@Override
			public void run() {
				graph2LastXValue += 1d;

				//double randNum = getRandom();

				Double sensor1Data = sensor1.getCurValue();
				Double sensor2Data = sensor2.getCurValue();
				Double sensor3Data = sensor3.getCurValue();
				Double sensor4Data = sensor4.getCurValue();
				Double sensor5Data = sensor5.getCurValue();
				Double sensor6Data = sensor6.getCurValue();
				Double sensor7Data = sensor7.getCurValue();

				sensor1.appendData(new DataPoint(graph2LastXValue, sensor1Data), true, sensor1.getMax());
				sensor2.appendData(new DataPoint(graph2LastXValue, sensor2Data), true, sensor2.getMax());
				sensor3.appendData(new DataPoint(graph2LastXValue, sensor3Data), true, sensor3.getMax());
				sensor4.appendData(new DataPoint(graph2LastXValue, sensor4Data), true, sensor4.getMax());
				sensor5.appendData(new DataPoint(graph2LastXValue, sensor5Data), true, sensor5.getMax());
				sensor6.appendData(new DataPoint(graph2LastXValue, sensor6Data), true, sensor6.getMax());
				sensor7.appendData(new DataPoint(graph2LastXValue, sensor7Data), true, sensor7.getMax());

				//mSeries2.appendData(new DataPoint(graph2LastXValue, randNum), true, 40);
				mHandler.postDelayed(this, 200);

			}

		};
		mHandler.postDelayed(mTimer2, 1000);
	}

	@Override
	public void onBackPressed() {
		// TODO Auto-generated method stub
		if(op_cli != null){
			op_cli.Cancel();
			op_cli = null;
		}
		super.onBackPressed();
	}
	public void onPause() {
		globalData.getInstance().getCommManager().requestTermNativeJSAppSensorViewer();
		if(op_cli != null){
			op_cli.Cancel();
			op_cli = null;
		}
		mHandler.removeCallbacks(mTimer2);
		super.onPause();
	}


	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

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
    
    
/*	private DataPoint[] generateData() {
        int count = 30;
        DataPoint[] values = new DataPoint[count];
        for (int i=0; i<count; i++) {
            double x = i;
            double f = mRand.nextDouble()*0.15+0.3;
            double y = Math.sin(i*f+2) + mRand.nextDouble()*0.3;
            DataPoint v = new DataPoint(x, y);
            values[i] = v;
        }
        return values;
    }*/
	
   /* String getSensorDataJson(){
    	jsonParser jp = new jsonParser();
    	jp.makeNewJson();
    	jp.addJsonKeyValue(sensorName1, String.valueOf(getRandom()));
    	jp.addJsonKeyValue(sensorName1, String.valueOf(getRandom()));
    	jp.addJsonKeyValue(sensorName1, String.valueOf(getRandom()));
    	jp.addJsonKeyValue(sensorName1, String.valueOf(getRandom()));
    	jp.addJsonKeyValue(sensorName1, String.valueOf(getRandom()));
    	jp.addJsonKeyValue("fff", String.valueOf(getRandom()));
    	jp.addJsonKeyValue("ggg", String.valueOf(getRandom()));
    	
    	Log.d("OPEL", jp.getJsonData());
    	
    	return jp.getJsonData(); 
    }*/
    
/*	double mLastRandom = 2;
    Random mRand = new Random();
    private double getRandom() {
    	double randData = mRand.nextDouble();
    	if(randData < 0)
    		randData *= -1;
    	if (mLastRandom < 0){
    		mLastRandom *= -1;
    	}
        return mLastRandom += randData*0.5 - 0.25;
    }*/

//}

/*

public class sensorViewer extends Activity{
	private final String TAG = "SensorViewer";
	private final String INTF_NAME = "Sensor Intf";
	
	private OpelClient op_cli;
	OpelCallbacks defCb = new OpelCallbacks(){
		void OpelCb(OpelMessage msg, short err){
			if(err != OpelSCModel.COMM_ERR_NONE){
			   
			    * Error happens
			    
				Log.d(TAG, "Error Happens"+err);
				
		   }
		   else{
			   Log.d(TAG,  "Messag comming : "+msg.get_data());
			   String message = new String(msg.get_data());
		   }
		}		
	};
	OpelCallbacks statChangedCb = new OpelCallbacks(){
		void OpelCb(OpelMessage smg, short stat){
			Log.d(TAG,  "STat Chagned"+stat);
			switch(stat){
			case OpelSocket.STAT_DISCON:
				
				 * Disconnected operation
				 
			case OpelSocket.STAT_CONNECTED:
				
				 * Connected Operation
				 
				
			}
		}
	}
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.sensorview);
     
        GraphView graph = (GraphView) findViewById(R.id.graph);
        LineGraphSeries<DataPoint> series = new LineGraphSeries<DataPoint>(new DataPoint[] {
                  new DataPoint(0, 1),
                  new DataPoint(1, 5),
                  new DataPoint(2, 3),
                  new DataPoint(3, 2),
                  new DataPoint(4, 6)
        });
        graph.addSeries(series);
        op_cli = OpelCommunicator.connectChannel(INTF_NAME, defCb, statChangedCb);
    }
    
    @Override
    public void onBackPressed() {
    	// TODO Auto-generated method stub
    	if(null != op_cli)
    		op_cli.Cancel();
    	op_cli = null;
    	super.onBackPressed();
    }
    @Override
    protected void onDestroy() {
    	// TODO Auto-generated method stub
    	if(null != op_cli)
    		op_cli.Cancel();
    	op_cli = null;
    	super.onDestroy();
    }
    protected void onResume() {
    	if(null == op_cli || op_cli.getStat() != op_cli.COMM_CONNECTED){
    		op_cli = OpelCommunicator.connectChannel(INTF_NAME, defCb, statChangedCb);
    	}
    };
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
}
*/