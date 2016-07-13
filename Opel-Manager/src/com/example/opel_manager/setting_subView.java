package com.example.opel_manager;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;

public class setting_subView extends Activity{
	
	ListView listView;
	ListSubAdapter ca;
	
	ArrayList<ListSubItem> arr;
	
	protected void onCreate(Bundle savedInstanceState) {
		
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.listview_activity);
		
		String title = getIntent().getStringExtra("title");
		String jsonData = getIntent().getStringExtra("jsonData");
		
		int iconId = getIntent().getIntExtra("iconID", R.drawable.ic_launcher);
		
		listView = (ListView) findViewById(R.id.listView1);
		 
		
		arr = getSettingMenuList(jsonData);
		
		ca = new ListSubAdapter(getApplicationContext(), setting_subView.this, arr);
		listView.setAdapter(ca);
		listView.setOnItemClickListener(mItemClickListener);
		
		
		
		ActionBar actionBar = getActionBar();
		actionBar.setTitle(title);
		actionBar.setDisplayHomeAsUpEnabled(true);

		actionBar.setLogo(iconId);
	
		actionBar.setDisplayUseLogoEnabled(true);
		
		
		
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
	
	private String selected;
	private AdapterView.OnItemClickListener mItemClickListener = new AdapterView.OnItemClickListener() {
		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position, long l_position) {
			final ListSubItem item = (ListSubItem) parent.getAdapter().getItem(position);
			
			selected = item.getStatus();
			
			
			AlertDialog.Builder ab = new AlertDialog.Builder(setting_subView.this);
			ab.setTitle(item.getTitle());
			
			ab.setSingleChoiceItems(item.getListArray(), item.getListPosition(selected),
					new DialogInterface.OnClickListener() {
						
						
						public void onClick(DialogInterface dialog,
								int whichButton) {
							
							selected = item.getListArray()[whichButton];
						}
					})
					.setPositiveButton(R.string.alert_dialog_ok,
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog,
										int whichButton) {
									
									 if(selected.equals( item.getStatus()) ){
										 //change setting item.getListArray()[selected]
									
									 }
								}
							})
						
					.setNegativeButton(R.string.alert_dialog_cancel,
							new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog,
										int whichButton) {
									selected = item.getStatus();
									
								}
							});
			ab.show();
			
		}
	};
	
	//Make the list to show on display by parsing input Json string
	public ArrayList<ListSubItem> getSettingMenuList(String jsonData){
		
		ArrayList<ListSubItem> arr = new ArrayList<ListSubItem>();
		
		String main="", sub="", status="";
		
		ArrayList<String> tempArr = new ArrayList<String>();
		String temp = "";
		int jsonControl = 0;
		
		jsonParser jp = new jsonParser(jsonData);
		
		 while(jp.hasMoreValue()){
		    	String ret[] = new String[2];
				ret=jp.getNextKeyValue();
				String tmp="";
		        
				if(ret[0].equals("sensor")){
					continue;
				}
				if(ret[0].equals("camera")){
					continue;
				}
				main = ret[0];
				for(int i=0; i<ret[1].length(); i++){
					if (ret[1].charAt(i) == '[') {
						tempArr.add(tmp);
						tmp = "";
					}

					else if (ret[1].charAt(i) == '/') {
						tempArr.add(tmp);
						tmp = "";
					}

					else if (ret[1].charAt(i) == ']') {
						status = tmp;
						tmp = "";
					}

					else if (ret[1].charAt(i) == '"') {
						continue;
					} else {
						tmp += ret[1].charAt(i);
					}
				}
				
				arr.add(new ListSubItem(main,"",status, tempArr));
				main="";
				status="";
				tempArr.clear();
				
		 }
		
		/*
		// Need to parse both json string and make arrList
		StringTokenizer st = new StringTokenizer(jsonData, "{,} ");
		while(st.hasMoreTokens()){
			String perListStr = st.nextToken();
						
			for(int i=0; i<perListStr.length(); i++){
				
				//Main String
				if(jsonControl == 0){
					if(perListStr.charAt(i) == ':'){
						jsonControl++;
						
					}
					else
						main+=perListStr.charAt(i);
				}
				
				//Alternative status
				else if(jsonControl == 1) {
					
					if(perListStr.charAt(i) == '('){
						tempArr.add(temp);
						temp="";
						jsonControl++;
						
					}
					
					else if(perListStr.charAt(i) == '/'){
						tempArr.add(temp);
						temp="";
						
					}
					else						
						temp += perListStr.charAt(i);
				}
				
				//Status String
				else if(jsonControl == 2){
					if(perListStr.charAt(i) == ')'){
						jsonControl++;
						
					}
					else
						status+=perListStr.charAt(i);
				}
			}*/
			
			if(main.equals("") || status.equals("") || tempArr.isEmpty()){
								
				Log.d("OPEL:setting_subView", "Json parsing error, data = " + jsonData);
				
				Toast.makeText(getApplicationContext(), "Json parsing error : "+ jsonData, 0).show();
				
			}
				
			else{
				arr.add(new ListSubItem(main,"",status, tempArr));
				main="";
				status="";
				tempArr.clear();
				jsonControl=0;
			}
		//}
		
		return arr;
	}
}

class ListSubAdapter extends BaseAdapter implements OnClickListener{
	private Context mContext;
	private Activity mActivity;
	private ArrayList<ListSubItem> arr;
	
	private int pos;
	
	private LinearLayout layout;
	
//	private Typeface myFont;
	public ListSubAdapter(Context mContext, Activity mActivity, ArrayList<ListSubItem> arr_item) {
		this.mContext = mContext;
		this.mActivity = mActivity;
		this.arr = arr_item;
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
			res = com.example.opel_manager.R.layout.listview_no_icon;
			LayoutInflater mInflater = (LayoutInflater)mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			convertView = mInflater.inflate(res, parent, false);
		}
		
		pos = position;
		if(arr.size() != 0){
			TextView title = (TextView)convertView.findViewById(R.id.tv_title);
			title.setText(arr.get(pos).getTitle());
			TextView subtitle = (TextView)convertView.findViewById(R.id.tv_subTitle);
			subtitle.setText(arr.get(pos).getSubtitle());
			if(arr.get(pos).getSubtitle().equals(null)){
				
			}
			TextView status = (TextView)convertView.findViewById(R.id.tv_status);
			status.setText(arr.get(pos).getStatus());
				
		}
		return convertView;
	}
	
	public void onClick(View v){
		final int tag = Integer.parseInt(v.getTag().toString());
		switch(v.getId()){
	
		}
	}
	
}


class ListSubItem {
	
	public String mainTitle;
	public String subTitle; 
	public String status;
	public ArrayList<String> alternativeTitle; 
	
	public ListSubItem(){
		this.mainTitle = null;
		this.subTitle = null;
		this.status = null;
		alternativeTitle = new ArrayList<String>();
	}
	
	public ListSubItem(String main, String sub, String status){
		this.mainTitle = main;
		this.subTitle = sub;
		this.status = status;
		//alternativeTitle = null;
	
		alternativeTitle=new ArrayList<String>();
	}
	
	public ListSubItem(String main, String sub, String status, ArrayList<String> arr){
		this.mainTitle = main;
		this.subTitle = sub;
		this.status = status;
		alternativeTitle=new ArrayList<String>();
		listDeepCopy(arr); 
	}
	
	public void listDeepCopy(ArrayList<String> arr){
		for(int i=0; i<arr.size(); i++)
			alternativeTitle.add(arr.get(i));
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
	public String getStatus(){
		return this.status;
	}
	
	public ArrayList<String> getAlterList(){
		return this.alternativeTitle;
	}
	
	//convert ArrayList to string array
	public String[] getListArray(){
		String[] str = new String[alternativeTitle.size()];
				
		str = alternativeTitle.toArray(str);
		return str;
	}
	
	//return position number of target string from arraylist
	public int getListPosition(String targetString){
		for(int i=0; i<alternativeTitle.size(); i++){
			if(this.status.equals(alternativeTitle.get(i)))
				return i;
		}
		return 0;
	}
}

