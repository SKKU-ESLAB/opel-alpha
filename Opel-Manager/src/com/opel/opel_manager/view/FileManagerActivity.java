package com.opel.opel_manager.view;

import android.annotation.SuppressLint;
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
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.opel.opel_manager.R;
import com.opel.opel_manager.controller.OPELControllerBroadcastReceiver;
import com.opel.opel_manager.controller.OPELControllerService;
import com.opel.opel_manager.model.message.params.ParamFileListEntry;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import cn.dxjia.ffmpeg.library.FFmpegNativeHelper;

import static android.content.ContentValues.TAG;

public class FileManagerActivity extends Activity {
    // OPELControllerService
    private OPELControllerService mControllerServiceStub = null;
    private PrivateControllerBroadcastReceiver mControllerBroadcastReceiver;
    private FileManagerActivity self = this;

    private FileListAdapter mFileListAdapter;
    private ArrayList<FileListItem> mFileList = new ArrayList<>();

    private String mCurrentPath = "";

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
        ListView fileListView = (ListView) findViewById(R.id.listView1);

        mFileListAdapter = new FileListAdapter();
        fileListView.setAdapter(mFileListAdapter);
        fileListView.setOnItemClickListener(mItemClickListener);
        fileListView.setOnItemLongClickListener(mItemLongClickListener);

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
            if (mControllerServiceStub == null) {
                return true;
            }
            FileListItem fileItem = (FileListItem) parent.getAdapter().getItem(position);
            mShareFileProcedure.start(fileItem);
            return true;
        }
    };

    private AdapterView.OnItemClickListener mItemClickListener = new AdapterView
            .OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long l_position) {
            if (mControllerServiceStub == null) {
                return;
            }
            FileListItem fileItem = (FileListItem) parent.getAdapter().getItem(position);
            if (fileItem.getFileType() == FileListItem.FILE_TYPE_DIRECTORY) {
                // Redirect to Directory
                String newPath = mCurrentPath + "/" + fileItem.getFileName();
                mRedirectToPathProcedure.start(newPath);
            } else if (fileItem.getFileType() == FileListItem.FILE_TYPE_REGULAR_FILE) {
                // Open file
                mOpenFileProcedure.start(fileItem);
            }
        }
    };

    public void updateUI() {
        if (mFileListAdapter != null) {
            mFileListAdapter.updateUI();
        }
    }

    private String getFileCachedName(String fileRPath) {
        // transform file's remote path to cached name
        String[] originPathArr = fileRPath.split("/");
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

    private File getCachedFile(String fileRemotePath) {
        // get cached file from remote path
        File remoteStorageDir = mControllerServiceStub.getSettings().getRemoteStorageDir();
        return new File(remoteStorageDir, getFileCachedName(fileRemotePath));
    }

    public void updateFileList(ParamFileListEntry[] fileList) {
        this.mFileList.clear();
        for (ParamFileListEntry file : fileList) {
            String fileName = file.fileName;
            int fileType = file.fileType;
            int fileSizeBytes = file.fileSizeBytes;
            String fileTime = file.fileTime;
            this.mFileList.add(new FileListItem(fileName, fileType, fileSizeBytes, fileTime));
        }
    }

    private class FileListAdapter extends BaseAdapter {
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

        @SuppressLint("SetTextI18n")
        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                int res = 0;
                res = com.opel.opel_manager.R.layout.template_listview_item_file;
                LayoutInflater mInflater = (LayoutInflater) self.getSystemService(Context
                        .LAYOUT_INFLATER_SERVICE);
                convertView = mInflater.inflate(res, parent, false);
            }

            if (mFileList.size() != 0) {
                FileListItem fileItem = mFileList.get(position);
                String fileName = fileItem.getFileName();
                String fileTime = fileItem.getFileTime();
                int fileSizeBytes = fileItem.getFileSizeBytes();
                int fileType = fileItem.getFileType();

                TextView titleTextView = (TextView) convertView.findViewById(R.id.tv_title);
                titleTextView.setText(fileName);

                TextView subtitleTextView = (TextView) convertView.findViewById(R.id.tv_subTitle);
                subtitleTextView.setText(fileTime);

                TextView statusTextView = (TextView) convertView.findViewById(R.id.tv_status);
                statusTextView.setText(fileSizeBytes + "B");

                String extension = fileName.substring(fileName.lastIndexOf(".") + 1, fileName
                        .length());
                ImageView iconImageView = (ImageView) convertView.findViewById(R.id.imageView11);
                switch (fileType) {
                    case FileListItem.FILE_TYPE_DIRECTORY:
                        iconImageView.setImageResource(R.drawable.fm_folder);
                        break;
                    case FileListItem.FILE_TYPE_REGULAR_FILE:
                        if (extension.compareTo("jpg") == 0 || extension.compareTo("png") == 0 ||
                                extension.compareTo("jpeg") == 0) {
                            iconImageView.setImageResource(R.drawable.fm_img);
                        } else if (extension.compareTo("avi") == 0 || extension.compareTo("wmv")
                                == 0 || extension.compareTo("mjpg") == 0 || extension.compareTo
                                ("mjpeg") == 0) {
                            iconImageView.setImageResource(R.drawable.fm_video);
                        } else {
                            iconImageView.setImageResource(R.drawable.fm_normal);
                        }
                        break;
                    case FileListItem.FILE_TYPE_OTHERS:
                    default:
                        iconImageView.setImageResource(R.drawable.filemanager);
                        break;
                }
            }
            return convertView;
        }

        void updateUI() {
            this.notifyDataSetChanged();
        }
    }


    private class FileListItem {
        public static final int FILE_TYPE_DIRECTORY = 1;
        public static final int FILE_TYPE_REGULAR_FILE = 2;
        public static final int FILE_TYPE_OTHERS = 0;

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

    private boolean isFileCached(String fileRemotePath) {
        File remoteStorageDir = mControllerServiceStub.getSettings().getRemoteStorageDir();
        File cachedFile = new File(remoteStorageDir, getFileCachedName(fileRemotePath));
        return cachedFile.exists();
    }

    private RedirectToPathProcedure mRedirectToPathProcedure = new RedirectToPathProcedure();
    private OpenFileProcedure mOpenFileProcedure = new OpenFileProcedure();
    private ShareFileProcedure mShareFileProcedure = new ShareFileProcedure();

    private class RedirectToPathProcedure {
        private ArrayList<Integer> mTransactions = new ArrayList<>();

        public void start(String path) {
            int messageId = mControllerServiceStub.getFileListAsync(path);
            this.mTransactions.add(messageId);
        }

        void onGetFileList(int commandMessageId, String path, ParamFileListEntry[] fileList) {
            for (Integer messageId : this.mTransactions) {
                if (messageId == commandMessageId) {
                    self.mCurrentPath = path;

                    updateFileList(fileList);
                    updateUI();
                    return;
                }
            }
        }
    }

    private class OpenFileProcedure {
        // item: <int messageId, String fileRemotePath>
        private HashMap<Integer, String> mGetFileRequests = new HashMap();

        public void start(FileListItem fileItem) {
            if (fileItem.getFileType() != FileListItem.FILE_TYPE_REGULAR_FILE) {
                String fileName = fileItem.getFileName();
                Log.d(TAG, "Since " + fileName + " is not regular file, it cannot be opened.");
                return;
            } else {
                String fileRemotePath = mCurrentPath + "/" + fileItem.getFileName();
                if (isFileCached(fileRemotePath)) {
                    // If file has already cached, share it.
                    File cachedFile = getCachedFile(fileRemotePath);
                    open(cachedFile.getAbsolutePath());
                } else {
                    // If file is not cached, get it from remote device.
                    requestFile(fileRemotePath);
                }
            }
        }

        private void requestFile(String fileRemotePath) {
            int messageId = mControllerServiceStub.getFileAsync(fileRemotePath);
            this.mGetFileRequests.put(messageId, fileRemotePath);
        }

        void onResultGetFileListener(int commandMessageId, String storedFilePath) {
            String requestFileRemotePath = this.mGetFileRequests.get(commandMessageId);
            if (requestFileRemotePath != null) {
                this.mGetFileRequests.remove(commandMessageId);

                File storedFile = new File(storedFilePath);
                File cachedFile = getCachedFile(requestFileRemotePath);

                // Rename the stored file with cached file naming convention
                storedFile.renameTo(cachedFile);

                // Share it
                this.open(cachedFile.getAbsolutePath());
            }
        }

        private void open(String fileCachedPath) {
            File fileCached = new File(fileCachedPath);
            String extension = fileCachedPath.substring(fileCachedPath.lastIndexOf(".") + 1,
                    fileCachedPath.length());

            Intent intent = new Intent();
            intent.setAction(Intent.ACTION_SEND);
            if (extension.compareTo("mp3") == 0) {
                intent.setDataAndType(Uri.fromFile(fileCached), "audio/*");
            } else if (extension.compareTo("mp4") == 0 || extension.compareTo("wmv") == 0 ||
                    extension.compareTo("avi") == 0) {
                intent.setDataAndType(Uri.fromFile(fileCached), "video/*");
            } else if (extension.compareTo("jpg") == 0 || extension.compareTo("jpeg") == 0 ||
                    extension.compareTo("gif") == 0 || extension.compareTo("png") == 0 ||
                    extension.compareTo("bmp") == 0) {
                intent.setDataAndType(Uri.fromFile(fileCached), "image/*");
            } else if (extension.equals("txt")) {
                intent.setDataAndType(Uri.fromFile(fileCached), "text/*");
            } else if (extension.equals("doc") || extension.equals("docx")) {
                intent.setDataAndType(Uri.fromFile(fileCached), "OPELApp/msword");
            } else if (extension.equals("xls") || extension.equals("xlsx")) {
                intent.setDataAndType(Uri.fromFile(fileCached), "OPELApp/vnd.ms-excel");
            } else if (extension.equals("ppt") || extension.equals("pptx")) {
                intent.setDataAndType(Uri.fromFile(fileCached), "OPELApp/vnd.ms-powerpoint");
            } else if (extension.equals("pdf")) {
                intent.setDataAndType(Uri.fromFile(fileCached), "OPELApp/pdf");
            } else if (extension.equals("hwp")) {
                intent.setDataAndType(Uri.fromFile(fileCached), "OPELApp/haansofthwp");
            } else if (extension.equals("mjpg") || extension.equals("mjpeg")) {
                String destFile = fileCachedPath.substring(0, fileCachedPath.length() - 4) + ".avi";
                File aviFile;
                FFmpegNativeHelper.runCommand("ffmpeg -i " + fileCached.getAbsolutePath() + " " +
                        "" + "-vcodec" + " mjpeg " + destFile);
                aviFile = new File(destFile);
                intent.setDataAndType(Uri.fromFile(aviFile), "video/*");
            } else {
                intent.setDataAndType(Uri.fromFile(fileCached), "OPELApp/*");
            }
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
        }
    }

    private class ShareFileProcedure {
        // item: <int messageId, String fileRemotePath>
        @SuppressLint("UseSparseArrays")
        private HashMap<Integer, String> mGetFileRequests = new HashMap<>();

        public void start(FileListItem fileItem) {
            if (fileItem.getFileType() != FileListItem.FILE_TYPE_REGULAR_FILE) {
                String fileName = fileItem.getFileName();
                Log.d(TAG, "Since " + fileName + " is not regular file, it cannot be shared.");
            } else {
                String fileRemotePath = mCurrentPath + "/" + fileItem.getFileName();
                if (isFileCached(fileRemotePath)) {
                    // If file has already cached, share it.
                    File cachedFile = getCachedFile(fileRemotePath);
                    share(cachedFile.getAbsolutePath());
                } else {
                    // If file is not cached, get it from remote device.
                    requestFile(fileRemotePath);
                }
            }
        }

        private void requestFile(String fileRemotePath) {
            int messageId = mControllerServiceStub.getFileAsync(fileRemotePath);
            this.mGetFileRequests.put(messageId, fileRemotePath);
        }

        void onResultGetFileListener(int commandMessageId, String storedFilePath) {
            String requestFileRemotePath = this.mGetFileRequests.get(commandMessageId);
            if (requestFileRemotePath != null) {
                this.mGetFileRequests.remove(commandMessageId);

                File storedFile = new File(storedFilePath);
                File cachedFile = getCachedFile(requestFileRemotePath);

                // Rename the stored file with cached file naming convention
                storedFile.renameTo(cachedFile);

                // Share it
                this.share(cachedFile.getAbsolutePath());
            }
        }

        private void share(String fileCachedPath) {
            Uri fileCachedURI = Uri.parse("file://" + fileCachedPath);
            Intent intent = new Intent();
            intent.setAction(Intent.ACTION_SEND);
            intent.putExtra(Intent.EXTRA_STREAM, fileCachedURI);
            intent.putExtra(Intent.EXTRA_TEXT, fileCachedURI.toString());
            intent.setType("*/*");

            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(intent);
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
                public void onResultGetFileList(int commandMessageId, String path,
                                                ParamFileListEntry[] fileList) {
                    mRedirectToPathProcedure.onGetFileList(commandMessageId, path, fileList);
                }
            });

            this.setOnResultGetFileListener(new OnResultGetFileListener() {
                @Override
                public void onResultGetFile(int commandMessageId, String storedFilePath) {
                    // Broadcast the ack message
                    mShareFileProcedure.onResultGetFileListener(commandMessageId, storedFilePath);
                    mOpenFileProcedure.onResultGetFileListener(commandMessageId, storedFilePath);
                }
            });
        }
    }
}