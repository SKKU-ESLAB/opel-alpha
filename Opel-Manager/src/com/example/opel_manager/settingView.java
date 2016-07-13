package com.example.opel_manager;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
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

public class settingView extends Activity{
	
	ListView listView;
	static ListAdapter ca;
	
	ArrayList<ListItem> arr;
	
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.listview_activity);		
		
		listView = (ListView) findViewById(R.id.listView1);
		 
		arr = getSettingMenuList();
		
		ca = new ListAdapter(getApplicationContext(), settingView.this, arr);
		listView.setAdapter(ca);
		listView.setOnItemClickListener(mItemClickListener);

		
		ActionBar actionBar = getActionBar();
		actionBar.setTitle("Setting");
		actionBar.setDisplayHomeAsUpEnabled(true);
		actionBar.setLogo(R.drawable.setting);
		
		actionBar.setDisplayUseLogoEnabled(true);
	
	}
	
	private AdapterView.OnItemClickListener mItemClickListener = new AdapterView.OnItemClickListener() {
		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position,
				long l_position) {
			ListItem item = (ListItem) parent.getAdapter().getItem(position);
			Log.d("OPEL", "Select list item : " + item.getTitle());

			  Bundle extras = new Bundle();
		      extras.putString("title", item.getTitle());
		      
		    // Set next menu Icon
		      
			if (item.getTitle().equals("Connection")) {
				extras.putInt("iconID", R.drawable.connect);
			} else if (item.getTitle().equals("Camera")) {
				extras.putInt("iconID", R.drawable.cam);
				extras.putString("jsonData", globalData.getInstance().getCameraInfo());
			} else if (item.getTitle().equals("Sensor")) {
				extras.putInt("iconID", R.drawable.sensor);
				extras.putString("jsonData", globalData.getInstance().getSensorInfo());
			} else if (item.getTitle().equals("Notification")) {
				extras.putInt("iconID", R.drawable.noti);
			} else if (item.getTitle().equals("About OPEL")) {
				extras.putInt("iconID", R.drawable.about);
			}

			Intent intent = new Intent(settingView.this, setting_subView.class);
			intent.putExtras(extras);

			startActivity(intent);
		}

	};

	public boolean onOptionsItemSelected(MenuItem item) {
	    switch (item.getItemId()) {
	        case android.R.id.home:
	        	this.finish();
	        	
	            return true;
	        default:
	            return super.onOptionsItemSelected(item);
	    }
	}
	
	public ArrayList<ListItem> getSettingMenuList(){
		ArrayList<ListItem> arr = new ArrayList<ListItem>();
		
		jsonParser sensorJP = new jsonParser(globalData.getInstance().getSensorInfo());
		jsonParser cameraJP = new jsonParser(globalData.getInstance().getCameraInfo());
		
		arr.add(new ListItem("Connection","Wi-Fi, Bluetooth, ... "));
		arr.add(new ListItem("Camera",cameraJP.getValueByKey("camera")));
		arr.add(new ListItem("Sensor",sensorJP.getValueByKey("sensor")));
		arr.add(new ListItem("Notification",""));
		arr.add(new ListItem("About OPEL","Version 1.0.0"));
	/*	arr.add(new ListItem("Light","On"));
		arr.add(new ListItem("Magnetic field","On"));
		arr.add(new ListItem("Orientation","Off"));
		arr.add(new ListItem("Pressure","Off"));
		arr.add(new ListItem("Proximity","N/A"));
		arr.add(new ListItem("Rotation","N/A"));
		arr.add(new ListItem("Temperature","N/A"));
		arr.add(new ListItem("Humidity","On"));*/
		return arr;
	}
	
		
	public void onItemClick(AdapterView<?> parent, View v, int position, long id)
    {
        
		ListItem data = (ListItem) parent.getItemAtPosition(position);
    //    Data data = mList.get(position);
         
		
		Log.d("OPEL", "Click " + data.getTitle());
		
        Bundle extras = new Bundle();
        extras.putString("title", data.getTitle());
        
        
        extras.putString("description", data.getTitle());
        
        Intent intent = new Intent(this, setting_subView.class);
        intent.putExtras(extras);
        startActivity(intent);
    }
	
	public static void updateDisplay(){
		
		if (ca == null){
			
		}
		else{
			ca.updateDisplay();
			
		}
	}


}

class ListAdapter extends BaseAdapter implements OnClickListener{
	private Context mContext;
	private Activity mActivity;
	private ArrayList<ListItem> arr;
	private int pos;
	
//	private Typeface myFont;
	public ListAdapter(Context mContext, Activity mActivity, ArrayList<ListItem> arr_item) {
		this.mContext = mContext;
		this.mActivity = mActivity;
		this.arr = arr_item;
		
//		myFont = Typeface.createFromAsset(mContext.getAssets(), "BareunDotum.ttf");
		
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
			res = com.example.opel_manager.R.layout.listview_icon;
			LayoutInflater mInflater = (LayoutInflater)mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			convertView = mInflater.inflate(res, parent, false);
		}
		pos = position;
		if(arr.size() != 0){
			TextView ci_nickname_text = (TextView)convertView.findViewById(R.id.tv_title);
			ci_nickname_text.setText(arr.get(pos).getTitle());
			TextView ci_content_text = (TextView)convertView.findViewById(R.id.tv_subTitle);
			ci_content_text.setText(arr.get(pos).getSubtitle());
			
			ImageView iv = (ImageView) convertView.findViewById(R.id.imageView11);
			if (arr.get(pos).getTitle().equals("Camera"))
				iv.setImageResource(R.drawable.cam);
			else if (arr.get(pos).getTitle().equals("Connection"))
				iv.setImageResource(R.drawable.connect);

			else if (arr.get(pos).getTitle().equals("Sensor"))
				iv.setImageResource(R.drawable.sensor);

			else if (arr.get(pos).getTitle().equals("About OPEL"))
				iv.setImageResource(R.drawable.about);

			else if (arr.get(pos).getTitle().equals("Notification"))
				iv.setImageResource(R.drawable.noti);

		}
		return convertView;
	}
	
	public void onClick(View v){
		final int tag = Integer.parseInt(v.getTag().toString());
		//switch(v.getId()){

		/*case R.id.Exit:
			AlertDialog.Builder alertDlg = new AlertDialog.Builder(mActivity);
            alertDlg.setPositiveButton("����", new DialogInterface.OnClickListener(){
                 @Override
                 public void onClick( DialogInterface dialog, int which ) {
                	// ma.deleteArr(tag);
                	 Toast.makeText(mContext, "���� �Ǿ����ϴ�.", 0).show();
                 }
            });
            alertDlg.setNegativeButton("���", null);
            alertDlg.setTitle("Application ����");
            alertDlg.setMessage("���� ���� �Ͻðڽ��ϱ�?");
            alertDlg.show();
			break;*/
		}	
	
		public void updateDisplay(){
			//this.arr = updateItemList(); //[MORE]
			//this.notifyDataSetChanged();
			
		}
}



class ListItem {
	
	public String mainTitle;
	public String subTitle; 
	public ArrayList<String> alternativeTitle; 
	
	
	public ListItem(){
		this.mainTitle = null;
		this.subTitle = null;
		alternativeTitle = new ArrayList<String>();
	}
	
	public ListItem(String main, String sub){
		this.mainTitle = main;
		this.subTitle = sub;
		alternativeTitle = null;
	}
	
	public ListItem(String main, String sub, ArrayList<String> arr){
		this.mainTitle = main;
		this.subTitle = sub;
		alternativeTitle = arr;
	}
	
	public void setAppName(String c){
		this.mainTitle = c;
	}
	
	public String getTitle(){
		return this.mainTitle;
	}
	public String getSubtitle(){
		return this.subTitle;
	}

}

