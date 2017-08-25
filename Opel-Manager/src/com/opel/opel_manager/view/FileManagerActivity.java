package com.opel.opel_manager.view;

import android.app.ActionBar;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.opel.opel_manager.R;
import com.opel.opel_manager.controller.LegacyJSONParser;
import com.opel.opel_manager.controller.OPELControllerBroadcastReceiver;
import com.opel.opel_manager.controller.OPELControllerService;

import java.io.File;
import java.util.ArrayList;

import cn.dxjia.ffmpeg.library.FFmpegNativeHelper;

import static android.content.ContentValues.TAG;

public class FileManagerActivity extends Activity {
    // OPELControllerService
    private OPELControllerService mControllerServiceStub = null;
    private PrivateControllerBroadcastReceiver mControllerBroadcastReceiver;
    private FileManagerActivity self = this;

    private ListView mFileListView;
    private static FileListAdapter mFileListAdapter;
    private ArrayList<FileListItem> mFileList;

    private String mCurrentPath = "";

    // TODO: remake it

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(com.opel.opel_manager.R.layout.template_listview);

        // Initialize UI
        this.initializeUI();

        // Initialize FFmpeg
        FFmpegNativeHelper.init();

        // Connect controller service
        this.connectControllerService();
    }

    private void initializeUI() {
        mFileListView = (ListView) findViewById(com.opel.opel_manager.R.id.listView1);
        LegacyJSONParser jp = new LegacyJSONParser("{\"type\":\"1014\"}");

        mFileListAdapter = new FileListAdapter(jp);
        mFileListView.setAdapter(mFileListAdapter);
        mFileListView.setOnItemClickListener(mItemClickListener);
        mFileListView.setOnItemLongClickListener(mItemLongClickListener);

        ActionBar actionBar = getActionBar();
        assert actionBar != null;
        actionBar.setTitle("File Manager");
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setLogo(com.opel.opel_manager.R.drawable.filemanager);
        actionBar.setDisplayUseLogoEnabled(true);
    }

    private AdapterView.OnItemLongClickListener mItemLongClickListener = new AdapterView
            .OnItemLongClickListener() {
        @Override
        public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
            if (mControllerServiceStub == null) return true;

            FileListItem item = (FileListItem) parent.getAdapter().getItem(position);
            if (!item.getFileType().equals("DIR")) {
                File remoteStorageDir = mControllerServiceStub.getSettings().getRemoteStorageDir();
                String requestFilePath = mCurrentPath + "/" + item.getFileName();
                File cachedFile = new File(remoteStorageDir, transformOfFilename(requestFilePath));
                if (cachedFile.exists()) {
                    LegacyJSONParser jp = new LegacyJSONParser();
                    jp.makeNewJson();
                    jp.addJsonKeyValue("type", "DuplicatedFile");
                    String fileName = transformOfFilename(requestFilePath);
                    String originPath = remoteStorageDir.getAbsolutePath() + "/" +
                            transformOfFilename(requestFilePath);
                    runSharingFile(fileName, originPath);
                } else {
                    mControllerServiceStub.getFileAsync(requestFilePath);
                }
            }
            return true;
        }
    };

    private AdapterView.OnItemClickListener mItemClickListener = new AdapterView
            .OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long l_position) {

            FileListItem item = (FileListItem) parent.getAdapter().getItem(position);
            if (item.getFileType().equals("DIR")) {
                // Redirect to Directory
                String newPath = mCurrentPath + "/" + item.getFileName();
                mRedirectToPathProcedure.start(newPath);
            } else {
                String requestFilePath = mCurrentPath + "/" + item.getFileName();

                File remoteStorageDir = mControllerServiceStub.getSettings().getRemoteStorageDir();
                File files = new File(remoteStorageDir, transformOfFilename(requestFilePath));

                if (files.exists() == true) {
                    LegacyJSONParser jp = new LegacyJSONParser();
                    jp.makeNewJson();
                    jp.addJsonKeyValue("type", "DuplicatedFile");
                    jp.addJsonKeyValue("mFileName", transformOfFilename(requestFilePath));
                    jp.addJsonKeyValue("originpath", OPELContext.getSettings()
                            .getRemoteStorageDir() + "/" + transformOfFilename(requestFilePath));

                    runRequestedFile(getApplicationContext(), jp);
                } else {
                    OPELContext.getAppCore().requestFilebyFileManager(requestFilePath, 0);
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

    public static void updateDisplay(LegacyJSONParser jp) {

        if (mFileListAdapter == null) {

        } else {
            mFileListAdapter.updateUI(jp);
        }
    }

    private static String transformOfFilename(String fileName) {
        String[] originPathArr = fileName.split("/");
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

    public void runRequestedFile(LegacyJSONParser jp) {
        if (mFileListAdapter != null) {
            File to;
            String originFileName = "";

            if (jp.getValueByKey("type").equals("DuplicatedFile")) {
                originFileName = jp.getValueByKey("mFileName");
                to = new File(OPELContext.getSettings().getRemoteStorageDir(), originFileName);
            } else {
                String fileName = jp.getValueByKey("mFileName");
                String originPath = jp.getValueByKey("originpath");

                originFileName = transformOfFilename(originPath);

                File from = new File(OPELContext.getSettings().getRemoteStorageDir(), fileName);
                to = new File(OPELContext.getSettings().getRemoteStorageDir(), originFileName);

                from.renameTo(to);
            }


            Intent fileLinkIntent = new Intent(Intent.ACTION_VIEW);
            Uri uri = Uri.fromFile(to);

            String fileExtend = originFileName.substring(originFileName.lastIndexOf(".") + 1,
                    originFileName.length());

            if (fileExtend.compareTo("mp3") == 0) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "audio/*");
            } else if (fileExtend.compareTo("mp4") == 0 || fileExtend.compareTo("wmv") == 0 ||
                    fileExtend.compareTo("avi") == 0) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "video/*");
            } else if (fileExtend.compareTo("jpg") == 0 || fileExtend.compareTo("jpeg") == 0 ||
                    fileExtend.compareTo("gif") == 0 || fileExtend.compareTo("png") == 0 ||
                    fileExtend.compareTo("bmp") == 0) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "image/*");
            } else if (fileExtend.equals("txt")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "text/*");
            } else if (fileExtend.equals("doc") || fileExtend.equals("docx")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "OPELApp/msword");
            } else if (fileExtend.equals("xls") || fileExtend.equals("xlsx")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "OPELApp/vnd.ms-excel");
            } else if (fileExtend.equals("ppt") || fileExtend.equals("pptx")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "OPELApp/vnd.ms-powerpoint");
            } else if (fileExtend.equals("pdf")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "OPELApp/pdf");
            } else if (fileExtend.equals("hwp")) {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "OPELApp/haansofthwp");
            } else if (fileExtend.equals("mjpg") || fileExtend.equals("mjpeg")) {

                String destFile = to.getAbsolutePath().substring(0, to.getAbsolutePath().length()
                        - 4) + ".avi";
                File aviFile;
                if (!jp.getValueByKey("type").equals("DuplicatedFile")) {
                    FFmpegNativeHelper.runCommand("ffmpeg -i " + to.getAbsolutePath() + " " +
                            "-vcodec" + " mjpeg " + destFile);
                }

                aviFile = new File(destFile);
                fileLinkIntent.setDataAndType(Uri.fromFile(aviFile), "video/*");
            } else {
                fileLinkIntent.setDataAndType(Uri.fromFile(to), "OPELApp/*");
            }
            fileLinkIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            this.startActivity(fileLinkIntent);
        }
    }

    public void runSharingFile(String filePath) {
        if (mFileListAdapter != null) {
            Intent sendIntent = new Intent();
            sendIntent.setAction(Intent.ACTION_SEND);
            sendIntent.putExtra(Intent.EXTRA_STREAM, Uri.parse("file://" + filePath));
            sendIntent.putExtra(Intent.EXTRA_TEXT, "This is my text to send.");
            sendIntent.setType("*/*");

            sendIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            this.startActivity(sendIntent);
        }
    }


    private class FileListAdapter extends BaseAdapter {
        private int pos;
        private LegacyJSONParser jp;

        FileListAdapter(LegacyJSONParser jp) {
            this.jp = jp;
            mFileList = getSettingMenuList();
        }

        ArrayList<FileListItem> getSettingMenuList() {
            ArrayList<FileListItem> arr = new ArrayList<FileListItem>();

            while (jp.hasMoreValue()) {
                String ret[] = new String[2];
                ret = jp.getNextKeyValue();
                String tmp = "";


                if (ret[0].equals("type")) continue;

                String[] typeSizeTime = ret[1].split("/");
                arr.add(new FileListItem(ret[0], typeSizeTime[0], typeSizeTime[1],
                        typeSizeTime[2]));
                Log.d("OPEL", ret[0] + "  " + ret[1]);
                Log.d("OPEL", typeSizeTime[0] + "  " + typeSizeTime[1] + "  " + typeSizeTime[2] +
                        "  ");
            }

            return arr;
        }

        @Override
        public int getCount() {
            return mFileList.size();
        }

        @Override
        public Object getItem(int position) {
            return mFileList.get(position);
        }

        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                int res = 0;
                res = com.opel.opel_manager.R.layout.template_listview_item_file;
                LayoutInflater mInflater = (LayoutInflater) self.getSystemService(Context
                        .LAYOUT_INFLATER_SERVICE);
                convertView = mInflater.inflate(res, parent, false);
            }
            pos = position;
            if (mFileList.size() != 0) {
                TextView ci_nickname_text = (TextView) convertView.findViewById(R.id.tv_title);
                ci_nickname_text.setText(mFileList.get(pos).getFileName());
                TextView ci_content_text = (TextView) convertView.findViewById(R.id.tv_subTitle);
                ci_content_text.setText(mFileList.get(pos).getFileTime());
                TextView ci_status_text = (TextView) convertView.findViewById(R.id.tv_status);
                ci_status_text.setText(mFileList.get(pos).getFileSize() + "Byte");

                ImageView iv = (ImageView) convertView.findViewById(R.id.imageView11);

                if (mFileList.get(pos).getFileType().equals("DIR")) {
                    iv.setImageResource(R.drawable.fm_folder);
                } else if (mFileList.get(pos).getFileType().equals("REG") && (mFileList.get(pos)
                        .getFileName().endsWith("jpg") || mFileList.get(pos).getFileName()
                        .endsWith("png") || mFileList.get(pos).getFileName().endsWith("jpeg"))) {
                    iv.setImageResource(R.drawable.fm_img);
                } else if (mFileList.get(pos).getFileType().equals("REG") && (mFileList.get(pos)
                        .getFileName().endsWith("avi") || mFileList.get(pos).getFileName()
                        .endsWith("wmv") || mFileList.get(pos).getFileName().endsWith("mjpg") ||
                        mFileList.get(pos).getFileName().endsWith("mjpeg"))) {
                    iv.setImageResource(R.drawable.fm_video);
                } else if (mFileList.get(pos).getFileType().equals("REG")) {
                    iv.setImageResource(R.drawable.fm_normal);
                } else if (mFileList.get(pos).getFileType().equals("ETC")) {
                    iv.setImageResource(R.drawable.filemanager);
                }
            }
            return convertView;
        }

        public void updateUI(LegacyJSONParser jp) {
            this.jp = jp;
            this.mFileList = getSettingMenuList(); //[MORE]
            this.notifyDataSetChanged();
        }
    }


    private class FileListItem {
        private String mFileName;
        private int mFileType;
        private int mFileSizeBytes;
        private String mFileTime;

        FileListItem(String fileName, int fileType, int fileSizeBytes, String fileTime) {
            this.mFileName = fileName;
            this.mFileType = fileType;
            this.mFileSizeBytes = fileSizeBytes;
            this.mFileTime = fileTime;
        }

        public String getFileName() {
            return this.mFileName;
        }

        public int getFileType() {
            return this.mFileType;
        }

        public int getFileSizeBytes() {
            return this.mFileSizeBytes;
        }

        public String getFileTime() {
            return this.mFileTime;
        }
    }

    private void connectControllerService() {
        Intent serviceIntent = new Intent(this, OPELControllerService.class);
        this.bindService(serviceIntent, this.mControllerServiceConnection, Context
                .BIND_AUTO_CREATE);
    }

    private RedirectToPathProcedure mRedirectToPathProcedure = new RedirectToPathProcedure();
    private OpenFileProcedure mOpenFileProcedure = new OpenFileProcedure();
    private ShareFileProcedure mShareFileProcedure = new ShareFileProcedure();

    class RedirectToPathProcedure {
        private ArrayList<Integer> mTransactions = new ArrayList<>();

        public void start(String path) {
            int messageId = mControllerServiceStub.getFileListAsync(path);
            this.mTransactions.add(messageId);
        }

        public void onGetFileList(int commandMessageId, String path, String[] fileList) {
            self.mCurrentPath = path;

            // TODO: clear and addAll FileList
        }
    }

    class OpenFileProcedure {
        public void start(String filePath) {

        }
    }

    class ShareFileProcedure {
        public void start(String filePath) {

        }
    }

    private ServiceConnection mControllerServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder inputBinder) {
            OPELControllerService.ControllerBinder serviceBinder = (OPELControllerService
                    .ControllerBinder) inputBinder;
            mControllerServiceStub = serviceBinder.getService();

            // Set BroadcastReceiver
            IntentFilter broadcastIntentFilter = new IntentFilter();
            broadcastIntentFilter.addAction(OPELControllerBroadcastReceiver.ACTION);
            mControllerBroadcastReceiver = new PrivateControllerBroadcastReceiver();
            registerReceiver(mControllerBroadcastReceiver, broadcastIntentFilter);

            // Get root path
            mControllerServiceStub.getTargetRootPathAsync();
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            Log.d(TAG, "onServiceDisconnected()");
            mControllerServiceStub = null;
        }
    };

    class PrivateControllerBroadcastReceiver extends OPELControllerBroadcastReceiver {
        PrivateControllerBroadcastReceiver() {
            this.setOnResultGetTargetRootPathListener(new OnResultGetTargetRootPathListener() {
                @Override
                public void onResultGetTargetRootPath(int commandMessageId, String path) {
                    // Redirect to the root path
                    mRedirectToPathProcedure.start(path);
                }
            });
            this.setOnResultGetFileListListener(new OnResultGetFileListListener() {
                @Override
                public void onResultGetFileList(int commandMessageId, String path, String[]
                        fileList) {
                    mRedirectToPathProcedure.onGetFileList(commandMessageId, path, fileList);
                }
            });
        }
    }
}