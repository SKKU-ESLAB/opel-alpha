package com.opel.opel_manager.view;


import android.app.ActionBar;
import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.widget.TextView;

import com.jjoe64.graphview.GraphView;
import com.jjoe64.graphview.series.DataPoint;
import com.jjoe64.graphview.series.LineGraphSeries;
import com.opel.opel_manager.R;
import com.opel.opel_manager.controller.LegacyJSONParser;
import com.opel.opel_manager.controller.OPELContext;

public class SensorViewerActivity extends Activity {

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
    private boolean mIsUIReady = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sensor_viewer);

        ActionBar actionBar = getActionBar();
        actionBar.setTitle("Sensor Viewer");
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setLogo(R.drawable.sensor);
        actionBar.setDisplayUseLogoEnabled(true);


        sensor1 = new perSensorData(0, 100, sensorName1, (GraphView)
                findViewById(R.id.graph1), (TextView) findViewById(R.id
                .textView11));
        sensor2 = new perSensorData(0, 100, sensorName2, (GraphView)
                findViewById(R.id.graph2), (TextView) findViewById(R.id
                .textView22));
        sensor3 = new perSensorData(0, 100, sensorName3, (GraphView)
                findViewById(R.id.graph3), (TextView) findViewById(R.id
                .textView33));
        sensor4 = new perSensorData(0, 100, sensorName4, (GraphView)
                findViewById(R.id.graph4), (TextView) findViewById(R.id
                .textView4));
        sensor5 = new perSensorData(0, 100, sensorName5, (GraphView)
                findViewById(R.id.graph5), (TextView) findViewById(R.id
                .textView5));
        sensor6 = new perSensorData(0, 100, sensorName6, (GraphView)
                findViewById(R.id.graph6), (TextView) findViewById(R.id
                .textView6));
        sensor7 = new perSensorData(0, 100, sensorName7, (GraphView)
                findViewById(R.id.graph7), (TextView) findViewById(R.id
                .textView7));

        sensor1.setGraphLineColor(Color.RED);
        sensor2.setGraphLineColor(Color.BLUE);
        sensor3.setGraphLineColor(Color.WHITE);
        sensor4.setGraphLineColor(Color.YELLOW);
        sensor5.setGraphLineColor(Color.WHITE);
        sensor6.setGraphLineColor(Color.BLUE);
        sensor7.setGraphLineColor(Color.YELLOW);
    }

    public void onMsgToSensorViewer(String message) {
        Log.d(TAG, "Message coming : " + message);
        LegacyJSONParser jp = new LegacyJSONParser(message);

        Double val1 = Double.parseDouble(jp.getValueByKey(sensor1
                .getSensorName()));
        Double val2 = Double.parseDouble(jp.getValueByKey(sensor2
                .getSensorName()));
        Double val3 = Double.parseDouble(jp.getValueByKey(sensor3
                .getSensorName()));
        Double val4 = Double.parseDouble(jp.getValueByKey(sensor4
                .getSensorName()));
        Double val5 = Double.parseDouble(jp.getValueByKey(sensor5
                .getSensorName()));
        Double val6 = Double.parseDouble(jp.getValueByKey(sensor6
                .getSensorName()));
        Double val7 = Double.parseDouble(jp.getValueByKey(sensor7
                .getSensorName()));

        if (this.mIsUIReady == true) {
            if (val1 >= 0) {
                sensor1.setEnabled(true);
                sensor1.setCurValue(val1);
            } else {
                sensor1.setEnabled(false);
            }
            if (val2 >= 0) {
                sensor2.setEnabled(true);
                sensor2.setCurValue(val2);
            } else {
                sensor2.setEnabled(false);
            }
            if (val3 >= 0) {
                sensor3.setEnabled(true);
                sensor3.setCurValue(val3);
            } else {
                sensor3.setEnabled(false);
            }
            if (val4 >= 0) {
                sensor4.setEnabled(true);
                sensor4.setCurValue(val4);
            } else {
                sensor4.setEnabled(false);
            }
            if (val5 >= 0) {
                sensor5.setEnabled(true);
                sensor5.setCurValue(val5);
            } else {
                sensor5.setEnabled(false);
            }
            if (val6 >= 0) {
                sensor6.setEnabled(true);
                sensor6.setCurValue(val6);
            } else {
                sensor6.setEnabled(false);
            }
            if (val7 >= 0) {
                sensor7.setEnabled(true);
                sensor7.setCurValue(val7);
            } else {
                sensor7.setEnabled(false);
            }
        }
    }

    public void onResume() {
        super.onResume();
        OPELContext.getAppCore().registerSensorView(this);
        OPELContext.getAppCore().requestRunNativeJSAppSensorViewer();

        mTimer2 = new Runnable() {
            @Override
            public void run() {
                graph2LastXValue += 1d;

                Double sensor1Data = sensor1.getCurValue();
                Double sensor2Data = sensor2.getCurValue();
                Double sensor3Data = sensor3.getCurValue();
                Double sensor4Data = sensor4.getCurValue();
                Double sensor5Data = sensor5.getCurValue();
                Double sensor6Data = sensor6.getCurValue();
                Double sensor7Data = sensor7.getCurValue();

                sensor1.appendData(new DataPoint(graph2LastXValue,
                        sensor1Data), true, sensor1.getMax());
                sensor2.appendData(new DataPoint(graph2LastXValue,
                        sensor2Data), true, sensor2.getMax());
                sensor3.appendData(new DataPoint(graph2LastXValue,
                        sensor3Data), true, sensor3.getMax());
                sensor4.appendData(new DataPoint(graph2LastXValue,
                        sensor4Data), true, sensor4.getMax());
                sensor5.appendData(new DataPoint(graph2LastXValue,
                        sensor5Data), true, sensor5.getMax());
                sensor6.appendData(new DataPoint(graph2LastXValue,
                        sensor6Data), true, sensor6.getMax());
                sensor7.appendData(new DataPoint(graph2LastXValue,
                        sensor7Data), true, sensor7.getMax());

                //mSeries2.appendData(new DataPoint(graph2LastXValue,
                // randNum), true, 40);
                mHandler.postDelayed(this, 200);

            }

        };
        mHandler.postDelayed(mTimer2, 1000);

        this.mIsUIReady = true;
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }

    public void onPause() {
        super.onPause();
        OPELContext.getAppCore().unregisterSensorView();
        OPELContext.getAppCore().requestTermNativeJSAppSensorViewer();
        mHandler.removeCallbacks(mTimer2);
        this.mIsUIReady = false;
    }

    class perSensorData {
        int min;
        int max;
        double curValue;
        String sensorName;

        GraphView graph;
        TextView textV;

        LineGraphSeries<DataPoint> mSeries;
        int mUniqueColor;
        boolean mIsEnabled = false;

        public perSensorData(int min, int max, String sensorName, GraphView
                graph, TextView textV) {
            this.min = min;
            this.max = max;
            this.graph = graph;
            this.curValue = 0.0;
            this.sensorName = sensorName;
            this.textV = textV;
            this.textV.setText(sensorName + "(Disconnected)");
            mSeries = new LineGraphSeries<DataPoint>();

            graph.addSeries(mSeries);
            graph.getViewport().setXAxisBoundsManual(true);
            graph.getViewport().setMinX(min);
            graph.getViewport().setMaxX(max);

            graph.getViewport().setYAxisBoundsManual(false);

            mSeries.setThickness(5);

        }

        public void setCurValue(double var) {
            this.curValue = var;
        }

        public double getCurValue() {
            return this.curValue;
        }


        public void appendData(DataPoint a, boolean b, int c) {
            mSeries.appendData(a, b, c);
        }

        public int getMax() {
            return this.max;
        }

        public String getSensorName() {
            return this.sensorName;
        }

        public void setTextViewColor(int color) {
            textV.setTextColor(color);
        }

        public void setGraphLineColor(int color) {
            this.mUniqueColor = color;
            this.updateView();
        }

        public void setEnabled(boolean isEnabled) {
            this.mIsEnabled = isEnabled;
            this.updateView();
        }

        private void updateView() {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if (mIsEnabled == true) {
                        mSeries.setColor(mUniqueColor);
                        textV.setText(sensorName);
                    } else {
                        mSeries.setColor(Color.GRAY);
                        textV.setText(sensorName + " (Disconnected)");
                    }
                }
            });
        }
    }
}