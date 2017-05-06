package com.opel.opel_manager.view;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
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

import com.opel.opel_manager.R;
import com.opel.opel_manager.controller.OPELContext;
import com.opel.opel_manager.controller.JSONParser;

import java.io.File;
import java.util.ArrayList;

import cn.dxjia.ffmpeg.library.FFmpegNativeHelper;

public class FileManagerActivity extends Activity {

    ListView listView;
    static ListAdapter_fileManager ca;
    ArrayList<ListItem_fileManager> arr;

    String curPath;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(com.opel.opel_manager.R.layout.template_listview);

        listView = (ListView) findViewById(com.opel.opel_manager.R.id
                .listView1);
        JSONParser jp = new JSONParser("{\"type\":\"1014\"}");
        //JSONParser jp = new JSONParser("{\"type\":\"1014\",
        // \"bin\":\"DIR/4096/16-00-15 10:51\",\"boot\":\"DIR/16384/70-00-01
        // 00:00\",\"boot.bak\":\"REG/4096/16-00-15 10:51\"}");

        //arr = getSettingMenuList();

        ca = new ListAdapter_fileManager(getApplicationContext(),
                FileManagerActivity.this, jp);
        listView.setAdapter(ca);
        listView.setOnItemClickListener(mItemClickListener);
        listView.setOnItemLongClickListener(mItemLongClickListener);
        ActionBar actionBar = getActionBar();
        actionBar.setTitle("File Manager");
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setLogo(com.opel.opel_manager.R.drawable.filemanager);

        actionBar.setDisplayUseLogoEnabled(true);

        curPath = OPELContext.getSettings().getOpelDataDir();
        OPELContext.getAppCore().requestUpdateFileManager(curPath);

        FFmpegNativeHelper.init();

    }

    private AdapterView.OnItemLongClickListener mItemLongClickListener = new
            AdapterView.OnItemLongClickListener() {

        @Override
        public boolean onItemLongClick(AdapterView<?> parent, View view, int
                position, long id) {

            ListItem_fileManager item = (ListItem_fileManager) parent
                    .getAdapter().getItem(position);

            if (item.getType().equals("DIR")) {
                curPath = curPath + "/" + item.getFileName();
                OPELContext.getAppCore().requestUpdateFileManager(curPath);
            } else {
                String requestFilePath = curPath + "/" + item.getFileName();

                File files = new File(OPELContext.getSettings()
                        .getRemoteStorageDir(), transformOfFilename
                        (requestFilePath));

                if (files.exists() == true) {

                    JSONParser jp = new JSONParser();
                    jp.makeNewJson();
                    jp.addJsonKeyValue("type", "DuplicatedFile");
                    jp.addJsonKeyValue("filename", transformOfFilename
                            (requestFilePath));
                    jp.addJsonKeyValue("originpath", OPELContext.getSettings
                            ().getRemoteStorageDir() + "/" +
                            transformOfFilename(requestFilePath)); //???????

                    runSharingFile(getApplicationContext(), jp);
                } else {
                    OPELContext.getAppCore().requestFilebyFileManager
                            (requestFilePath, 1);
                }
            }
            return true;
        }

    };

    private AdapterView.OnItemClickListener mItemClickListener = new
            AdapterView.OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int
                position, long l_position) {

            ListItem_fileManager item = (ListItem_fileManager) parent
                    .getAdapter().getItem(position);

            if (item.getType().equals("DIR")) {
                curPath = curPath + "/" + item.getFileName();
                OPELContext.getAppCore().requestUpdateFileManager(curPath);
            } else {
                String requestFilePath = curPath + "/" + item.getFileName();

                File files = new File(OPELContext.getSettings()
                        .getRemoteStorageDir(), transformOfFilename
                        (requestFilePath));

                if (files.exists() == true) {

                    JSONParser jp = new JSONParser();
                    jp.makeNewJson();
                    jp.addJsonKeyValue("type", "DuplicatedFile");
                    jp.addJsonKeyValue("filename", transformOfFilename
                            (requestFilePath));
                    jp.addJsonKeyValue("originpath", OPELContext.getSettings
                            ().getRemoteStorageDir() + "/" +
                            transformOfFilename(requestFilePath)); //??

                    runRequestedFile(getApplicationContext(), jp);
                } else {
                    OPELContext.getAppCore().requestFilebyFileManager
                            (requestFilePath, 0);
                }
            }

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


    public void onItemClick(AdapterView<?> parent, View v, int position, long
            id) {

        ListItem_fileManager data = (ListItem_fileManager) parent
                .getItemAtPosition(position);
        //    Data data = mList.get(position);

/*
        Log.d("OPEL", "Click " + data.getTitle());

        Bundle extras = new Bundle();
        extras.putString("title", data.getTitle());


        extras.putString("description", data.getTitle());

        Intent intent = new Intent(this, SettingSubActivity.class);
        intent.putExtras(extras);
        startActivity(intent);*/
    }

    public static void updateDisplay(JSONParser jp) {

        if (ca == null) {

        } else {
            ca.updateDisplay(jp);
        }
    }

    private static String transformOfFilename(String orig) {

        String tmp = orig;
        String[] originPathArr = tmp.split("/");
        String ret = "";

        for (int i = 0; i < originPathArr.length; i++) {

            if (i == 0) continue;
            else if (i == originPathArr.length - 1) {
                ret += originPathArr[i];
                continue;
            }

            ret += originPathArr[i] + "___";
        }

        return ret;

    }

    public static void runRequestedFile(Context ctx, JSONParser jp) {

        if (ca == null) {

        } else {
            File to;
            String originFileName = "";

            if (jp.getValueByKey("type").equals("DuplicatedFile")) {
                originFileName = jp.getValueByKey("filename");
                to = new File(OPELContext.getSettings().getRemoteStorageDir()
                        , originFileName);
            } else {
                String fileName = jp.getValueByKey("filename");
                String originPath = jp.getValueByKey("originpath");

                originFileName = transformOfFilename(originPath);

                File from = new File(OPELContext.getSettings()
                        .getRemoteStorageDir(), fileName);
                to = new File(OPELContext.getSettings().getRemoteStorageDir()
                        , originFileName);

                from.renameTo(to);
            }


            Intent fileLinkIntent = new Intent(Intent.ACTION_VIEW);
            Uri uri = Uri.fromFile(to);

            String fileExtend = originFileName.substring(originFileName
                    .lastIndexOf(".") + 1, originFileName.length());

            if (fileExtend.equals("mp3")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "audio/*");
            } else if (fileExtend.equals("mp4") || fileExtend.equals("wmv")
                    || fileExtend.equals("avi")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "video/*");
            } else if (fileExtend.equals("jpg") || fileExtend.equals("jpeg")
                    || fileExtend.equals("gif") || fileExtend.equals("png")
                    || fileExtend.equals("bmp")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "image/*");
            } else if (fileExtend.equals("txt")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "text/*");
            } else if (fileExtend.equals("doc") || fileExtend.equals("docx")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to),
                        "OPELApplication/msword");
            } else if (fileExtend.equals("xls") || fileExtend.equals("xlsx")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to),
                        "OPELApplication/vnd.ms-excel");
            } else if (fileExtend.equals("ppt") || fileExtend.equals("pptx")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to),
                        "OPELApplication/vnd.ms-powerpoint");
            } else if (fileExtend.equals("pdf")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to),
                        "OPELApplication/pdf");
            } else if (fileExtend.equals("hwp")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to),
                        "OPELApplication/haansofthwp");
            } else if (fileExtend.equals("mjpg") || fileExtend.equals
                    ("mjpeg")) {

                String destFile = to.getAbsolutePath().substring(0, to
                        .getAbsolutePath().length() - 4) + ".avi";
                File aviFile;
                if (!jp.getValueByKey("type").equals("DuplicatedFile")) {
                    FFmpegNativeHelper.runCommand("ffmpeg -i " + to
                            .getAbsolutePath() + " -vcodec mjpeg " + destFile);
                }

                aviFile = new File(destFile);
                fileLinkIntent.setDataAndType(Uri.fromFile(aviFile), "video/*");
            } else {
                fileLinkIntent.setDataAndType(Uri.fromFile(to),
                        "OPELApplication/*");
            }

            fileLinkIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            ctx.startActivity(fileLinkIntent);

        }
    }

    public static void runSharingFile(Context ctx, JSONParser jp) {
        if (ca == null) {

        } else {
            File to;
            String originFileName = "";
            Log.d("OPEL", "SHARE = " + jp.getJsonData());

            if (jp.getValueByKey("type").equals("DuplicatedFile")) {
                originFileName = jp.getValueByKey("filename");
                to = new File(OPELContext.getSettings().getRemoteStorageDir()
                        , originFileName);
            } else {
                String fileName = jp.getValueByKey("filename");
                String originPath = jp.getValueByKey("originpath");

                originFileName = transformOfFilename(originPath);

                File from = new File(OPELContext.getSettings()
                        .getRemoteStorageDir(), fileName);
                to = new File(OPELContext.getSettings().getRemoteStorageDir()
                        , originFileName);

                from.renameTo(to);
            }

            String fileStr = to.getAbsolutePath();
            String fileExtend = fileStr.substring(fileStr.lastIndexOf(".") +
                    1, fileStr.length());

			/*
            Intent intentShareFile = new Intent(Intent.ACTION_SEND);
			intentShareFile.setType("OPELApplication/*");
			intentShareFile.putExtra(Intent.EXTRA_STREAM, Uri.parse("file://"
			+ fileStr));

			intentShareFile.putExtra(Intent.EXTRA_SUBJECT, "Sharing File...");
			intentShareFile.putExtra(Intent.EXTRA_TEXT, "Sharing File...");

			intentShareFile.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			ctx.startActivity(Intent.createChooser(intentShareFile, "Share
			image using"));*/

            Intent sendIntent = new Intent();
            sendIntent.setAction(Intent.ACTION_SEND);
            sendIntent.putExtra(Intent.EXTRA_STREAM, Uri.parse("file://" +
                    fileStr));
            sendIntent.putExtra(Intent.EXTRA_TEXT, "This is my text to send.");
            sendIntent.setType("*/*");

            sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            ctx.startActivity(sendIntent);

        }
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }
}

class ListAdapter_fileManager extends BaseAdapter implements OnClickListener {
    private Context mContext;
    private Activity mActivity;
    private ArrayList<ListItem_fileManager> arr;
    private int pos;
    private JSONParser jp;

    //	private Typeface myFont;
    public ListAdapter_fileManager(Context mContext, Activity mActivity,
                                   JSONParser jp) {
        this.mContext = mContext;
        this.mActivity = mActivity;
        this.jp = jp;
        arr = getSettingMenuList();

//		myFont = Typeface.createFromAsset(mContext.getAssets(), "BareunDotum
// .ttf");

    }

    public ArrayList<ListItem_fileManager> getSettingMenuList() {
        ArrayList<ListItem_fileManager> arr = new
                ArrayList<ListItem_fileManager>();

        while (jp.hasMoreValue()) {
            String ret[] = new String[2];
            ret = jp.getNextKeyValue();
            String tmp = "";


            if (ret[0].equals("type")) continue;

            String[] typeSizeTime = ret[1].split("/");
            arr.add(new ListItem_fileManager(ret[0], typeSizeTime[0],
                    typeSizeTime[1], typeSizeTime[2]));
            Log.d("OPEL", ret[0] + "  " + ret[1]);
            Log.d("OPEL", typeSizeTime[0] + "  " + typeSizeTime[1] + "  " +
                    typeSizeTime[2] + "  ");
        }

        return arr;
    }

    @Override
    public int getCount() {
        return arr.size();
    }

    @Override
    public Object getItem(int position) {
        return arr.get(position);
    }

    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        if (convertView == null) {
            int res = 0;
            res = com.opel.opel_manager.R.layout.template_listview_item_file;
            LayoutInflater mInflater = (LayoutInflater) mContext
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            convertView = mInflater.inflate(res, parent, false);
        }
        pos = position;
        if (arr.size() != 0) {
            TextView ci_nickname_text = (TextView) convertView.findViewById(R
                    .id.tv_title);
            ci_nickname_text.setText(arr.get(pos).getFileName());
            TextView ci_content_text = (TextView) convertView.findViewById(R
                    .id.tv_subTitle);
            ci_content_text.setText(arr.get(pos).getTime());
            TextView ci_status_text = (TextView) convertView.findViewById(R
                    .id.tv_status);
            ci_status_text.setText(arr.get(pos).getSize() + "Byte");

            ImageView iv = (ImageView) convertView.findViewById(R.id
                    .imageView11);

            if (arr.get(pos).getType().equals("DIR"))
                iv.setImageResource(R.drawable.fm_folder);

            else if (arr.get(pos).getType().equals("REG") && (arr.get(pos)
                    .getFileName().endsWith("jpg") || arr.get(pos)
                    .getFileName().endsWith("png") || arr.get(pos)
                    .getFileName().endsWith("jpeg")))// ADD MORE FILE TYPE
                iv.setImageResource(R.drawable.fm_img);

            else if (arr.get(pos).getType().equals("REG") && (arr.get(pos)
                    .getFileName().endsWith("avi") || arr.get(pos)
                    .getFileName().endsWith("wmv") || arr.get(pos)
                    .getFileName().endsWith("mjpg") || arr.get(pos)
                    .getFileName().endsWith("mjpeg")))// ADD MORE FILE TYPE
                iv.setImageResource(R.drawable.fm_video);

            else if (arr.get(pos).getType().equals("REG"))
                iv.setImageResource(R.drawable.fm_normal);

            else if (arr.get(pos).getType().equals("ETC"))
                iv.setImageResource(R.drawable.filemanager);

            //ADD file type icon

        }
        return convertView;
    }

    public void onClick(View v) {
        final int tag = Integer.parseInt(v.getTag().toString());
        //switch(v.getId()){
        //}

    }

    public void updateDisplay(JSONParser jp) {
        this.jp = jp;
        this.arr = getSettingMenuList(); //[MORE]
        this.notifyDataSetChanged();
    }
}


class ListItem_fileManager {

    public String fileName;
    public String type;
    public String size;
    public String time;

    public ListItem_fileManager() {

    }

    public ListItem_fileManager(String fileName, String type, String size,
                                String time) {
        this.fileName = fileName;
        this.type = type;
        this.size = size;
        this.time = time;
    }

    public String getFileName() {
        return this.fileName;
    }

    public String getType() {
        return this.type;
    }

    public String getSize() {
        return this.size;
    }

    public String getTime() {
        return this.time;
    }


}

