package com.opel.opel_manager.view;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.opel.opel_manager.R;
import com.opel.opel_manager.controller.OPELContext;
import com.opel.opel_manager.model.OPELApplication;
import com.opel.opel_manager.model.OPELEvent;

import java.util.ArrayList;

public class EventLoggerActivity extends Activity{

	ListView listView;
	static EventLoggerAdapter ca;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(com.opel.opel_manager.R.layout.template_listview);

		ActionBar actionBar = getActionBar();
		actionBar.setTitle("Event Logger");
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setLogo(com.opel.opel_manager.R.drawable.eventlogger);
		actionBar.setDisplayUseLogoEnabled(true);

		listView = (ListView) findViewById(com.opel.opel_manager.R.id.listView1);

		ca = new EventLoggerAdapter(getApplicationContext(), EventLoggerActivity.this );
		//ca.sortingArrayByState();	
		listView.setAdapter(ca);

		listView.setOnItemClickListener(mItemClickListener);
		listView.setOnItemLongClickListener(mItemLongClickListener);

	}

	protected void onRestart() {
		super.onRestart();
		ca.updateDisplay();

	}

	protected void onPause() {
		super.onPause();
	}

	protected void onResume() {
		super.onResume();
		ca.updateDisplay();
	}


	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(com.opel.opel_manager.R.menu.menu_event_logger, menu);
		return true;
	}

	//back button
	public boolean onOptionsItemSelected(MenuItem item) {

		int id = item.getItemId();

		switch (id) {
			case android.R.id.home:
				this.finish();
				return true;
			case com.opel.opel_manager.R.id.action_filter:

				AlertDialog.Builder ab = new AlertDialog.Builder(EventLoggerActivity.this);
				ab.setTitle("OPELApplication Filtering");

				ArrayList <OPELApplication> apList = OPELContext.getAppList().getList();
				
		/*		String[] strItems = { "Alert Item 1", "Alert Item 2", "Alert Item 3" };

				AlertDialog alert = new AlertDialog.Builder( this )
				    .setIcon( R.drawable.appmanager )
				    .setTitle( "OPELApplication Filtering" )
				    .setPositiveButton( "OK", new DialogInterface.OnClickListener()
				    {
				        @Override
				        public void onClick(DialogInterface dialog, int which)
				        {
				            dialog.dismiss();
				        }
				    })
				    .setNegativeButton( "Cancel", new DialogInterface.OnClickListener()
				    {
				        @Override
				        public void onClick(DialogInterface dialog, int which)
				        {

				        }
				    })
				.setSingleChoiceItems(strItems, -1, new DialogInterface.OnClickListener()
			{
				@Override
				public void onClick(DialogInterface dialog, int which)
				{
					String msg = "";
					switch ( which )
					{
						case 0: msg = "Item 1 Selected"; break;
						case 1: msg = "Item 2 Selected"; break;
						case 2: msg = "Item 3 Selected"; break;
					}
					Log.d("OPEL", msg);
				}
			})
					.show();*/

				return true;

			case com.opel.opel_manager.R.id.action_filter_clear:

				OPELContext.getEventList().eventAllClear();
				EventLoggerActivity.updateDisplay();
				return true;

			default:
				return super.onOptionsItemSelected(item);
		}
	}



	private AdapterView.OnItemClickListener mItemClickListener = new AdapterView.OnItemClickListener() {
		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position,	long l_position) {

			OPELEvent item = (OPELEvent) parent.getAdapter().getItem(position);

			//TODO : open noti view intent

			Log.d("OPEL", item.getEventJsonData());
			Intent notificationIntent = new Intent(EventLoggerActivity.this, RemoteNotiUIActivity.class);
			Bundle extras = new Bundle();
			extras.putString("jsonData", item.getEventJsonData());
			extras.putString("checkNoti", "0");
			notificationIntent.putExtras(extras);
			startActivity(notificationIntent);

		}

	};

	private AdapterView.OnItemLongClickListener mItemLongClickListener = new AdapterView.OnItemLongClickListener() {
		@Override
		public boolean onItemLongClick(AdapterView<?> parent, View arg1, int pos, long id) {

			// Log.v("long clicked","pos: " + pos);
			final OPELEvent item = (OPELEvent) parent.getAdapter().getItem(pos);

			AlertDialog.Builder alt_bld = new AlertDialog.Builder(EventLoggerActivity.this);
			alt_bld.setMessage("Delete this OPELEvent ?")
					.setCancelable(false)
					.setPositiveButton("Yes",
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog,	int id) {
									// Action for 'Yes' Button
									OPELContext.getEventList().deleteEvent(item.getEventID());
									EventLoggerActivity.updateDisplay();
								}
							})
					.setNegativeButton("No",
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog,	int id) {
									// Action for 'NO' Button

									dialog.cancel();
								}
							});

			AlertDialog alert = alt_bld.create();

			alert.setTitle(item.getEventAppName());
			Drawable d = new BitmapDrawable(getResources(), OPELContext.getAppList().getApp(item.getEventAppID()).getImage());
			alert.setIcon(d);
			alert.show();


			return true;
		}
	};

	public static void updateDisplay(){

		if (ca == null){

		}
		else{
			ca.updateDisplay();

		}
	}
}

class EventLoggerAdapter extends BaseAdapter implements OnClickListener{
	private Context mContext;
	private Activity mActivity;
	private ArrayList<OPELEvent> arr;
	private int pos;

	public EventLoggerAdapter(Context mContext, Activity mActivity) {
		this.mContext = mContext;
		this.mActivity = mActivity;
		this.arr = updateItemList();
	}

	@Override
	public int getCount() {
		return arr.size();
	}

	@Override
	public Object getItem(int position) {
		return arr.get(position);
	}
	public long getItemId(int position){
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		if(convertView == null){
			int res = 0;
			res = com.opel.opel_manager.R.layout.template_listview_item_icon;
			LayoutInflater mInflater = (LayoutInflater)mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			convertView = mInflater.inflate(res, parent, false);
		}
		pos = position;
		if(arr.size() != 0){
			TextView textAppName = (TextView)convertView.findViewById(R.id.tv_title);
			textAppName.setText(arr.get(pos).getEventAppName());

			TextView textDescription = (TextView)convertView.findViewById(R.id.tv_subTitle);

			String desc = "";
			desc = arr.get(pos).getEventDescription();
			textDescription.setTextColor(Color.WHITE);
			textDescription.setText(desc);


			TextView textDate = (TextView)convertView.findViewById(R.id.tv_status);
			textDate.setTextColor(Color.WHITE);

			String timeStr = arr.get(pos).getEventTime();
			String[] dateTime = timeStr.split(" ");
			String time="";

			Log.d("OPEL", String.valueOf(dateTime.length) + "  " + dateTime[0] + " " + dateTime[1]+ "length:" + dateTime[1].length() );

			if(dateTime[1].length() == 6){

				time += dateTime[1].substring(0, 2) + ":" + dateTime[1].substring(2, 4) + ":" + dateTime[1].substring(4, 6)  ;
				textDate.setText(dateTime[0]+"\n"+time);
			}
			else{
				textDate.setText(timeStr);
			}

			ImageView iv = (ImageView) convertView.findViewById(R.id.imageView11);
			iv.setImageBitmap( OPELContext.getAppList().getApp(arr.get(pos).getEventAppID()).getImage() );


		}
		return convertView;
	}

	public void onClick(View v){
		final int tag = Integer.parseInt(v.getTag().toString());
		//switch(v.getId()){

		/*case R.id.Exit:
			AlertDialog.Builder alertDlg = new AlertDialog.Builder(mActivity);
            alertDlg.setPositiveButton("占쏙옙占쏙옙", new DialogInterface.OnClickListener(){
                 @Override
                 public void onClick( DialogInterface dialog, int which ) {
                	// ma.deleteArr(tag);
                	 Toast.makeText(mContext, "占쏙옙占쏙옙 占실억옙占쏙옙占싹댐옙.", 0).show();
                 }
            });
            alertDlg.setNegativeButton("占쏙옙占?, null);
            alertDlg.setTitle("OPELApplication 占쏙옙占쏙옙");
            alertDlg.setMessage("占쏙옙占쏙옙 占쏙옙占쏙옙 占싹시겠쏙옙占싹깍옙?");
            alertDlg.show();
			break;*/
	}

	public void updateDisplay(){
		this.arr = updateItemList();
		this.notifyDataSetChanged();
	}

	//get app infomation from global list and transfer to ListApp format
	public ArrayList<OPELEvent> updateItemList(){

		return OPELContext.getEventList().getCurEventArrayList();
	}
}












