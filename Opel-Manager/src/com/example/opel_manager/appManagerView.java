package com.example.opel_manager;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
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
import java.util.Comparator;

public class appManagerView extends Activity{


	ListView listView;
	static appListAdapter ca;
	ArrayList<ListApp> arr;

	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.listview_activity);

		ActionBar actionBar = getActionBar();
		actionBar.setTitle("App Manager");
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setLogo(R.drawable.appmanager);
		actionBar.setDisplayUseLogoEnabled(true);

		listView = (ListView) findViewById(R.id.listView1);

		arr = new ArrayList<ListApp>();

		ca = new appListAdapter(getApplicationContext(), appManagerView.this );
		//ca.sortingArrayByState();	
		listView.setAdapter(ca);

		//listView.setOnItemClickListener(mItemClickListener);
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


	//back button
	public boolean onOptionsItemSelected(MenuItem item) {


		switch (item.getItemId()) {
			case android.R.id.home:
				this.finish();
				return true;

			default:
				return super.onOptionsItemSelected(item);
		}
	}

	private AdapterView.OnItemClickListener mItemClickListener = new AdapterView.OnItemClickListener() {
		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position,	long l_position) {

			ListApp item = (ListApp) parent.getAdapter().getItem(position);
			Log.d("OPEL", "Select list item : " + item.getTitle());
		}

	};

	private AdapterView.OnItemLongClickListener mItemLongClickListener = new AdapterView.OnItemLongClickListener() {
		@Override
		public boolean onItemLongClick(AdapterView<?> parent, View arg1, int pos, long id) {

			// Log.v("long clicked","pos: " + pos);
			final ListApp item = (ListApp) parent.getAdapter().getItem(pos);

			AlertDialog.Builder alt_bld = new AlertDialog.Builder(appManagerView.this);
			alt_bld.setMessage("Delete this app ?")
					.setCancelable(false)
					.setPositiveButton("Yes",
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog,	int id) {
									// Action for 'Yes' Button									
									globalData.getInstance().getCommManager().requestUninstall(item.getAppID());
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

			alert.setTitle(item.getTitle());
			Drawable d = new BitmapDrawable(getResources(), item.getIcon());
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




class appListAdapter extends BaseAdapter implements OnClickListener{
	private Context mContext;
	private Activity mActivity;
	private ArrayList<ListApp> arr;
	private int pos;
	Comparator<ListApp> comperator;

	//	private Typeface myFont;
	public appListAdapter(Context mContext, Activity mActivity) {
		this.mContext = mContext;
		this.mActivity = mActivity;
		this.arr = updateItemList();

//		myFont = Typeface.createFromAsset(mContext.getAssets(), "BareunDotum.ttf");

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
			res = com.example.opel_manager.R.layout.listview_icon;
			LayoutInflater mInflater = (LayoutInflater)mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			convertView = mInflater.inflate(res, parent, false);
		}
		pos = position;
		if(arr.size() != 0){
			TextView ci_nickname_text = (TextView)convertView.findViewById(R.id.tv_title);
			ci_nickname_text.setText(arr.get(pos).getTitle());
			TextView ci_content_text = (TextView)convertView.findViewById(R.id.tv_subTitle);


			String state = "";
			if(arr.get(pos).getRunningState() == 0){
				state = "Installed";
				ci_content_text.setTextColor(Color.WHITE);
			}
			else if (arr.get(pos).getRunningState() ==1){
				state = "Running";
				ci_content_text.setTextColor(Color.RED);
			}
			ci_content_text.setText(state);

			ImageView iv = (ImageView) convertView.findViewById(R.id.imageView11);
			iv.setImageBitmap(arr.get(pos).getIcon());

		}
		return convertView;
	}

	public void onClick(View v){
		final int tag = Integer.parseInt(v.getTag().toString());
		//switch(v.getId()){

		/*case R.id.Exit:

			break;*/
	}

	public void updateDisplay(){
		this.arr = updateItemList();
		this.notifyDataSetChanged();
	}



	//get app infomation from global list and transfer to ListApp format
	public ArrayList<ListApp> updateItemList(){
		ArrayList<ListApp> arr = new ArrayList<ListApp>();

		ArrayList<application> appArray = globalData.getInstance().getAppList().getAllApplicationList();

		for (int i = 0; i < appArray.size(); i++) {
			application tmpApp = appArray.get(i);
			if (tmpApp.getType() == -1) {
				continue;
			}
			else{
				arr.add(new ListApp( "" + tmpApp.getAppId(), tmpApp.getTitle(),
						tmpApp.getImage(),tmpApp.getType())  );
			}
		}

		return arr;
	}
}





class ListApp {

	public String appID;
	public String mainTitle;
	public int runningState;
	public String memoryUsage;
	public Bitmap icon;

	public ListApp(String appID, String main, Bitmap icon,  int state){
		this.appID = appID;
		this.mainTitle = main;
		this.runningState = state;
		this.icon = icon;
	}

	public void setAppID(String id){
		this.appID = id;
	}

	public void setAppName(String c){
		this.mainTitle = c;
	}

	public void setRunningState(int state){
		this.runningState = state;
	}
	public void setIcon(Bitmap icon){
		this.icon = icon;
	}

	public String getAppID(){
		return this.appID;
	}
	public String getTitle(){
		return this.mainTitle;
	}
	public int getRunningState(){
		return this.runningState;
	}

	public Bitmap getIcon(){
		return icon;
	}

}
