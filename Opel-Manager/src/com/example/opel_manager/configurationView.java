package com.example.opel_manager;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.DatePickerDialog;
import android.app.TimePickerDialog;
import android.content.Context;
import android.content.DialogInterface;
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
import android.widget.DatePicker;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.TimePicker;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Calendar;

public class configurationView extends Activity{

	ListView listView;
	static ListConfigAdapter ca;

	ArrayList<ListConfigItem> arr;
	String inputJson;
	jsonParser jp;

	String appID;
	String rqID;
	String pid;

	final String defaultStatus = ">";

	protected void onCreate(Bundle savedInstanceState) {


		super.onCreate(savedInstanceState);
		setContentView(R.layout.listview_activity);

		String title = getIntent().getStringExtra("title");
		Toast.makeText(getApplicationContext(),	title, 0).show();
		inputJson = getIntent().getStringExtra("jsonData");

		jp = new jsonParser(inputJson);
		appID = jp.getValueByKey("appID");
		rqID = jp.getValueByKey("rqID");
		pid = jp.getValueByKey("pid");

		int iconId = getIntent().getIntExtra("iconID", R.drawable.app);

		listView = (ListView) findViewById(R.id.listView1);

		arr = getSettingMenuList(inputJson);

		ca = new ListConfigAdapter(getApplicationContext(), configurationView.this, arr);
		listView.setAdapter(ca);
		listView.setOnItemClickListener(mItemClickListener);

		application targetApp = globalData.getInstance().getAppList().getAppInAllList(appID);

		try {
			ActionBar actionBar = getActionBar();
			actionBar.setTitle(targetApp.getTitle());
			actionBar.setDisplayHomeAsUpEnabled(true);
			actionBar.setDisplayHomeAsUpEnabled(true);
			actionBar.setDisplayShowCustomEnabled(true);
			Drawable dr = new BitmapDrawable(getResources(), globalData.getInstance().getAppList().getAppInAllList(appID).getImage());
			actionBar.setIcon( dr );
			actionBar.setDisplayUseLogoEnabled(true);

		} catch (Exception e) {
			e.printStackTrace();
		}


	}
	public boolean onCreateOptionMenu(Menu menu){
		if(menu != null){
			getMenuInflater().inflate(R.menu.main, menu);
		}
		return true;
	}


	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.config_actionbar, menu);
		return true;
	}

	//back button
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
			case android.R.id.home:

				this.finish();
				break;

			case R.id.action_config_ok:
				if (completeConfigSetting()){
					jsonParser jp = new jsonParser();
					jp.makeNewJson();

					jp.addJsonKeyValue("appID", appID);
					jp.addJsonKeyValue("rqID", rqID);
					jp.addJsonKeyValue("pid", pid);
					for(int i = 0; i < arr.size(); i++){

						jp.addJsonKeyValue(arr.get(i).getTitle(), arr.get(i).getStatus());

					}

					globalData.getInstance().getCommManager().requestConfigSetting(jp);
					this.finish();
				}


				break;
			default:
				return super.onOptionsItemSelected(item);

		}
		return true;
	}



	public static void updateDisplay(){

		if (ca == null){

		}
		else{
			ca.updateDisplay();
		}
	}


	public boolean completeConfigSetting(){

		for(int i = 0; i < arr.size(); i++){

			if (arr.get(i).getStatus().equals(defaultStatus)){
				Toast.makeText(getApplicationContext(),	"Select all of the option!",	0).show();
				return false;
			}
		}

		Toast.makeText(getApplicationContext(),	"Set this configuration",	0).show();
		return true;

	}

	private String selected;
	private AdapterView.OnItemClickListener mItemClickListener = new AdapterView.OnItemClickListener() {
		@Override
		public void onItemClick(AdapterView<?> parent, View view, int position,
								long l_position) {
			final ListConfigItem item = (ListConfigItem) parent.getAdapter()
					.getItem(position);

			selected = item.getStatus();

			int selectedFlag = item.flag;


			if (selectedFlag == 1) {

				// Str textBox - constrain >> length
				if (item.getAlterList().size() == 1) {
					final String length = item.getAlterList().get(0);
					AlertDialog.Builder alert = new AlertDialog.Builder(configurationView.this);
					alert.setTitle(item.getTitle()); // Set Alert dialog title
					// here
					alert.setMessage("Length : " + length); // Message here

					final EditText input = new EditText(configurationView.this);
					alert.setView(input);

					alert.setPositiveButton("OK",new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,	int whichButton) {

							String inputStr = input.getEditableText().toString();
							if (inputStr.length() < Integer.parseInt(length)) {
								item.setStatus(inputStr);
							}

							else {
								Toast.makeText(getApplicationContext(),	"Input str length is too long",	0).show();
							}

						}
					});
					alert.setNegativeButton("CANCEL",new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,
											int whichButton) {
							dialog.cancel();
						}
					});
					AlertDialog alertDialog = alert.create();
					alertDialog.show();

				}


				// Number textBox - constrain >> range
				else if (item.getAlterList().size() == 2) {

					AlertDialog.Builder alert = new AlertDialog.Builder(configurationView.this);
					alert.setTitle(item.getTitle()); // Set Alert dialog title
					// here
					alert.setMessage("Range : " + item.getAlterList().get(0) +" ~ " + item.getAlterList().get(1)); // Message here

					final EditText input = new EditText(configurationView.this);
					alert.setView(input);

					alert.setPositiveButton("OK",new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,	int whichButton) {

							String inputStr = input.getEditableText().toString();
							Double inputNumber=0.0;

							try{
								inputNumber = Double.parseDouble(inputStr);

								if ( Double.parseDouble(item.getAlterList().get(0)) < inputNumber && inputNumber < Double.parseDouble(item.getAlterList().get(1))) {
									item.setStatus(inputStr);
								}

								else {
									Toast.makeText(getApplicationContext(),	"Input Number is out of range",	0).show();
								}

							}
							catch(NumberFormatException e){
								Toast.makeText(getApplicationContext(),	"Input data is not number format",	0).show();

							}
						}
					});
					alert.setNegativeButton("CANCEL",new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog,
											int whichButton) {
							dialog.cancel();
						}
					});


					AlertDialog alertDialog = alert.create();
					alertDialog.show();
				}

			}


			// single choice dialog
			else if (selectedFlag == 2) {

				AlertDialog.Builder ab = new AlertDialog.Builder(configurationView.this);
				ab.setTitle(item.getTitle());

				ab.setSingleChoiceItems(item.getListArray(), item.getListPosition(selected),
						new DialogInterface.OnClickListener() {

							public void onClick(DialogInterface dialog,	int whichButton) {
								selected = item.getListArray()[whichButton];
							}
						})
						.setPositiveButton(R.string.alert_dialog_ok,
								new DialogInterface.OnClickListener() {
									public void onClick(DialogInterface dialog,	int whichButton) {
										if(item.getStatus().equals(defaultStatus)){
											item.setStatus( item.getListArray()[0]);
										}
										else{
											item.setStatus(selected);
										}
										ca.updateDisplay();

									}
								})
						.setNegativeButton(R.string.alert_dialog_cancel,
								new DialogInterface.OnClickListener() {
									public void onClick(DialogInterface dialog,
														int whichButton) {
										dialog.cancel();
									}
								});

				AlertDialog alertDialog = ab.create();
				alertDialog.show();
			}


			//*9****************Not implemented***********************
			else if (selectedFlag == 3) {

				AlertDialog.Builder builder = new AlertDialog.Builder(
						configurationView.this);
				builder.setTitle(item.getTitle())
						.setMultiChoiceItems(
								item.getListArray(),
								null,
								new DialogInterface.OnMultiChoiceClickListener() {
									@Override
									public void onClick(DialogInterface dialog,	int which, boolean isChecked) {
										if (isChecked) {

										}
									}
								})

						.setPositiveButton(R.string.alert_dialog_ok, new DialogInterface.OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog, int id) {
								//Create onlcick method
							}
						})
						.setNegativeButton(R.string.alert_dialog_cancel, new DialogInterface.OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog, int id) {
								//Create onlcick method
							}
						});
				builder.show();

			}

			else if (selectedFlag == 4) {

				final Calendar c = Calendar.getInstance();
				int mYear = c.get(Calendar.YEAR);
				int mMonth = c.get(Calendar.MONTH);
				int mDay = c.get(Calendar.DAY_OF_MONTH);

				DatePickerDialog dpd = new DatePickerDialog(configurationView.this,
						new DatePickerDialog.OnDateSetListener() {

							@Override
							public void onDateSet(DatePicker view, int year, int monthOfYear, int dayOfMonth) {

								String _year, _month, _day;
								_year = String.valueOf(year);

								_month = String.valueOf(monthOfYear+1);
								if(_month.length() ==1 )
									_month = "0"+_month;

								_day = String.valueOf(dayOfMonth);
								if(_day.length() ==1 )
									_day = "0"+_day;

								item.setStatus(_year +"-"+ _month +"-" + _day );
								ca.updateDisplay();
							}
						}, mYear, mMonth, mDay);
				dpd.show();

			}

			else if (selectedFlag == 5) {

				int mHour=10;
				int mMinute=10;

				TimePickerDialog tpd = new TimePickerDialog(configurationView.this,
						new TimePickerDialog.OnTimeSetListener() {

							@Override
							public void onTimeSet(TimePicker view, int hourOfDay, int minute) {

								String hour, min;

								hour = String.valueOf(hourOfDay);
								if(hour.length() ==1 )
									hour = "0"+hour;

								min = String.valueOf(minute);
								if(min.length() ==1 )
									min = "0"+min;

								item.setStatus(hour +":"+ min);
								ca.updateDisplay();
							}
						}, mHour, mMinute, false);
				tpd.show();

			}
		}
	};


	//Make the list to show on display by parsing input Json string
	public ArrayList<ListConfigItem> getSettingMenuList(String jsonData){
		ArrayList<ListConfigItem> arr = new ArrayList<ListConfigItem>();

		while(jp.hasMoreValue()){
			String ret[] = new String[2];
			ret=jp.getNextKeyValue();

			ArrayList<String> strArr = parseConfigItem(ret[1]);
			ArrayList<String> strTemp = new ArrayList<String>();
			String key="", description="";
			for(int i=0; i < strArr.size(); i++){
				if(i==0){
					key = strArr.get(i);
				}
				else if(i==1){
					description = strArr.get(i);
				}
				else{
					strTemp.add(strArr.get(i));
					Log.d("OPEL", strArr.get(i));
				}

			}

			if(ret[0].equals("strTB")){

				arr.add(new ListConfigItem(key, description, defaultStatus,  1, strTemp));
			}
			else if(ret[0].equals("numTB")){

				arr.add(new ListConfigItem(key, description, defaultStatus, 1, strTemp));
			}
			else if(ret[0].equals("sDialog")){
				arr.add(new ListConfigItem(key, description, defaultStatus,  2, strTemp));

			}
			else if(ret[0].equals("mDialog")){
				arr.add(new ListConfigItem(key, description, defaultStatus,  3, strTemp));

			}
			else if(ret[0].equals("dateDialog")){
				arr.add(new ListConfigItem(key, description, defaultStatus,  4, strTemp));
			}
			else if(ret[0].equals("timeDialog")){
				arr.add(new ListConfigItem(key, description, defaultStatus,  5, strTemp));
			}
			else{
				Log.d("OPEL", "Config other item : "+ ret[0]);
			}
		}

		return arr;
	}

	private ArrayList<String> parseConfigItem(String str) {

		ArrayList<String> arr = new ArrayList<String>();
		String tmp = "";
		for (int i = 0; i < str.length(); i++) {

			if (str.charAt(i) == '[') {
				arr.add(tmp);
				tmp = "";
			}

			else if (str.charAt(i) == '/') {
				arr.add(tmp);
				tmp = "";
			}

			else if (str.charAt(i) == ']') {
				arr.add(tmp);
				tmp = "";
			}

			else if (str.charAt(i) == '"') {
				continue;
			} else {
				tmp += str.charAt(i);
			}

		}
		return arr;
	}
}

class ListConfigAdapter extends BaseAdapter implements OnClickListener{
	private Context mContext;
	private Activity mActivity;
	private ArrayList<ListConfigItem> arr;

	private int pos;

	private LinearLayout layout;

	//	private Typeface myFont;
	public ListConfigAdapter(Context mContext, Activity mActivity, ArrayList<ListConfigItem> arr_item) {
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

	public void updateDisplay(){

		this.notifyDataSetChanged();
	}
}

class ListConfigItem {

	public String mainTitle;
	public String subTitle;
	public String status;
	public int flag;
	// 1:textbox, 2:single 3:multiple 4:Date 5:Time

	public ArrayList<String> alternativeTitle;

	public ListConfigItem(){
		this.mainTitle = null;
		this.subTitle = null;
		this.status = null;
		alternativeTitle = new ArrayList<String>();
	}

	public ListConfigItem(String main, String sub, String status){
		this.mainTitle = main;
		this.subTitle = sub;
		this.status = status;
		//alternativeTitle = null;

		alternativeTitle=new ArrayList<String>();
	}
	public ListConfigItem(String main, String sub, String status, int flag){
		this.mainTitle = main;
		this.subTitle = sub;
		this.status = status;
		this.flag = flag;
		//alternativeTitle = null;

		alternativeTitle=new ArrayList<String>();
	}


	public ListConfigItem(String main, String sub, String status, int flag, ArrayList<String> arr){
		this.mainTitle = main;
		this.subTitle = sub;
		this.status = status;
		this.flag = flag;

		alternativeTitle=new ArrayList<String>();
		listDeepCopy(arr);
	}


	public ListConfigItem(String main, String sub, String status, ArrayList<String> arr){
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

	public void setSubtitle(String c){
		this.subTitle = c;
	}
	public void setStatus(String c){
		this.status = c;
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




	
	
	
	

