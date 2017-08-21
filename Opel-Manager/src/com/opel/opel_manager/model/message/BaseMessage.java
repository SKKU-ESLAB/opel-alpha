package com.opel.opel_manager.model.message;

/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import java.io.File;

// BaseMessage: the root base message
// - Decoding(makeFromJSON): C++, Java
// - Encoding(make, toJSON): C++, Java
public class BaseMessage {
    static private final int TYPE_NOT_DETERMINED = 0;
    static private final int TYPE_APPCORE = 10;
    static private final int TYPE_APPCOREACK = 11;
    static private final int TYPE_APP = 20;
    static private final int TYPE_COMPANION = 30;

    // Attach file on message to be sent
    public void attachFile(String filePath) {
        File file = new File(filePath);
        this.mData.fileName = file.getName();
    }

    // Set local file path when attached file has come
    public void setStoredFilePath(String storedFilePath) {
        this.mStoredFileName = storedFilePath;
    }

    public String getStoredFileName() {
        return this.mStoredFileName;
    }

    // encoding to JSON
    public String toJSONString() {
        // TODO: implement it
        return "";
    }

    // Get parameters
    public int getMessageId() {
        return Integer.parseInt(this.mData.messageIdStr);
    }

    public String getUri() {
        return this.mData.uri;
    }

    public int getType() {
        return Integer.parseInt(this.mData.type);
    }

    public boolean isFileAttached() {
        return (this.mData.isFileAttachedStr.compareTo("1") == 0);
    }

    public String getFileName() {
        return this.mData.fileName;
    }

    // Payload
    void setPayload(BaseMessagePayload payload) {
        this.mPayload = payload;
    }

    BaseMessagePayload getPayload() {
        return this.mPayload;
    }

    public BaseMessage(int messageId, String uri, int type, boolean isFileAttached, String
            fileName) {
        this.mData.messageIdStr = "" + messageId;
        this.mData.uri = uri;
        this.mData.type = "" + type;
        this.mData.isFileAttachedStr = (isFileAttached) ? "1" : "0";
        this.mData.fileName = fileName;
        this.mStoredFileName = "";
    }

    public BaseMessage(int messageId, String uri, int type) {
        this(messageId, uri, type, false, "");
    }

    protected BaseMessageData mData;
    protected BaseMessagePayload mPayload;
    protected String mStoredFileName;
}

class BaseMessageType {
    public static final int NotDetermined = 0;
    public static final int AppCore = 10;
    public static final int AppCoreAck = 11;
    public static final int App = 20;
    public static final int Companion = 30;
}

class BaseMessageData {
    public String messageIdStr;
    public String uri;
    public String type;
    public String isFileAttachedStr;
    public String fileName;
}

abstract class BaseMessagePayload {
    abstract public String toJSONString();
}

// AppCoreMessage: message sent to AppCore Framework
// - Decoding(makeFromJSON): C++
// - Encoding(make, toJSON): Java
class AppCoreMessage extends BaseMessagePayload {
    // TODO: implement it
    public String toJSONString() {
        // TODO: implement it
        return "";
    }

    public AppCoreMessage(int commandType) {

    }

    protected AppCoreMessageData mData;
}

class AppCoreMessageData {
    public String commandType;
    public String appCorePayload;
}

class AppCoreMessageCommandType {
    public static final int NotDetermined = 0;
    public static final int GetAppList = 1; // params: void (ACK params= ParamAppList)
    public static final int ListenAppState = 2; // params: int appId (ACK params: int appState)
    public static final int InitializeApp = 3; // params: std::string name (ACK params: int appId)
    public static final int InstallApp = 4; // params: int appId
    public static final int LaunchApp = 5; // params: int appId
    public static final int CompleteLaunchingApp = 6; // params: int appId, int pid
    public static final int TerminateApp = 7; // params: int appId
    public static final int RemoveApp = 8; // params: int appId
    public static final int GetFileList = 9; // params: std::string path (ACK params: AckParamFileList)
    public static final int GetFile = 10; // params: std::string path (ACK params: void)
    public static final int GetRootPath = 11; // params: void (ACK params: std::string rootPath)
}

// AppCoreAckMessage: ack message sent from AppCore Framework
// - Decoding(makeFromJSON): Java
// - Encoding(make, toJSON): C++
class AppCoreAckMessage extends BaseMessagePayload {
    // TODO: implement it
    public String toJSONString() {
        // TODO: implement it
        return "";
    }
}

// CompanionMessage: message sent to companion device
// - Decoding(makeFromJSON): Java
// - Encoding(make, toJSON): JavaScript
class CompanionMessage extends BaseMessagePayload {
    // TODO: implement it
    public String toJSONString() {
        // TODO: implement it
        return "";
    }
}