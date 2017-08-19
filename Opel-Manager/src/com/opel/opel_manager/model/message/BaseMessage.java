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

import com.google.gson.GsonBuilder;

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
        // TODO: implement it
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
        return new GsonBuilder().create().toJson(this.mData);
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

    protected Data mData;
    public String mStoredFileName;

    class Data {
        public String messageIdStr;
        public String uri;
        public String type;
        public String isFileAttachedStr;
        public String fileName;
    }
}

// AppCoreMessage: message sent to AppCore Framework
// - Decoding(makeFromJSON): C++
// - Encoding(make, toJSON): Java
class AppCoreMessage {
    // TODO: implement it
}

// AppCoreAckMessage: ack message sent from AppCore Framework
// - Decoding(makeFromJSON): Java
// - Encoding(make, toJSON): C++
class AppCoreAckMessage {
    // TODO: implement it
}

// CompanionMessage: message sent to companion device
// - Decoding(makeFromJSON): Java
// - Encoding(make, toJSON): JavaScript
class CompanionMessage {
    // TODO: implement it
}