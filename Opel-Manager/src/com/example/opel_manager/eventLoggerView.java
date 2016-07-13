package com.example.opel_manager;

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

import java.util.ArrayList;

public class eventLoggerView extends Activity{

	ListView listView;
	static eventLoggerAdapter ca;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.listview_activity);

		ActionBar actionBar = getActionBar();
		actionBar.setTitle("Event Logger");
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setLogo(R.drawable.eventlogger);
		actionBar.setDisplayUseLogoEnabled(true);

		listView = (ListView) findViewById(R.id.listView1);

		ca = new eventLoggerAdapter(getApplicationContext(), eventLoggerView.this );
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
		getMenuInflater().inflate(R.menu.eventlogger_actionbar, menu);
		return true;
	}

	//back button
	public boolean onOptionsItemSelected(MenuItem item) {

		int id = item.getItemId();

		switch (id) {
			case android.R.id.home:
				this.finish();
				return true;
			case R.id.action_filter:

				AlertDialog.Builder ab = new AlertDialog.Builder(eventLoggerView.this);
				ab.setTitle("Application Filtering");

				ArrayList <application> apList = globalData.getInstance().getAppList().getAllApplicationList();
				
		/*		String[] strItems = { "Alert Item 1", "Alert Item 2", "Alert Item 3" };

				AlertDialog alert = new AlertDialog.Builder( this )
				    .setIcon( R.drawable.appmanager )
				    .setTitle( "Application Filtering" )
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

			case R.id.action_filter_clear:

				globalData.getInstance().getEventList().eventAllClear();
				eventLoggerView.updateDisplay();
				return true;

			default:
				return super.onOptionsItemSelected(item);
		}
	}



	private AdapterView.OnItemClickListener mItemClickListener = new AdapterView.OnItemClickListener() {
		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position,	long l_position) {

			event item = (event) parent.getAdapter().getItem(position);

			//TODO : open noti view intent

			Log.d("OPEL", item.getEventJsonData());
			Intent notificationIntent = new Intent(eventLoggerView.this, notificationView.class);
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
			final event item = (event) parent.getAdapter().getItem(pos);

			AlertDialog.Builder alt_bld = new AlertDialog.Builder(eventLoggerView.this);
			alt_bld.setMessage("Delete this event ?")
					.setCancelable(false)
					.setPositiveButton("Yes",
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog,	int id) {
									// Action for 'Yes' Button
									globalData.getInstance().getEventList().deleteEvent(item.getEventID());
									eventLoggerView.updateDisplay();
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
			Drawable d = new BitmapDrawable(getResources(), globalData.getInstance().getAppList().getAppInAllList(item.getEventAppID()).getImage());
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

class Item{
	public final String text;
	public final int icon;
	public Item(String text, Integer icon) {
		this.text = text;
		this.icon = icon;
	}
	@Override
	public String toString() {
		return text;
	}
}

class eventLoggerAdapter extends BaseAdapter implements OnClickListener{
	private Context mContext;
	private Activity mActivity;
	private ArrayList<event> arr;
	private int pos;

	public eventLoggerAdapter(Context mContext, Activity mActivity) {
		this.mContext = mContext;
		this.mActivity = mActivity;
		this.arr = updateItemList();
	}

	/*public void sortingArrayByState(){
		Log.d("OPEL", "SORTING");
		final Comparator<ListApp> comparator = new Comparator<ListApp>(){
			public int compare(ListApp app1, ListApp app2){
				return (app1.getRunningState()) < app2.getRunningState() ? 1:-1;
			}
		};
		if(arr.isEmpty())
			Log.d("OPEL", "ARRAY is empty");

		Collections.sort(arr, comparator);

	}*/

	@Override
	public int getCount() {
		return arr.size();
	}

	@Override
	public Object getItem(int position) {https://www.google.co.kr/url?sa=t&rct=j&q=&esrc=s&source=web&cd=2&sqi=2&ved=0ahUKEwjhvcLIpZjNAhVj5KYKHemrCBkQFggiMAE&url=http%3A%2F%2Fwww.androidpub.com%2F2395660&usg=AFQjCNExRKzAAOOjPyHA0D-L7A3ercRNiw&sig2=SXs94msqQdvkoTvwMvL0Dg&bvm=bv.124088155,d.dGY
	return arr.get(position);
	}
	public long getItemId(int position){
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		if(convertView == null){
			int res = 0;
			res = com.example.opel_manager.R.layout.listview_icon;
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
			iv.setImageBitmap( globalData.getInstance().getAppList().getAppInAllList(arr.get(pos).getEventAppID()).getImage() );


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
            alertDlg.setTitle("Application 占쏙옙占쏙옙");
            alertDlg.setMessage("占쏙옙占쏙옙 占쏙옙占쏙옙 占싹시겠쏙옙占싹깍옙?");
            alertDlg.show();
			break;*/
	}

	public void updateDisplay(){
		this.arr = updateItemList();
		this.notifyDataSetChanged();
	}

	//get app infomation from global list and transfer to ListApp format
	public ArrayList<event> updateItemList(){

		return globalData.getInstance().getEventList().getCurEventArrayList();
	}
}












