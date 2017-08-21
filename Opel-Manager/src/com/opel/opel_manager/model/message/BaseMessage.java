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

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;

import java.io.File;

// BaseMessage: the root base message
// - Decoding(makeFromJSON): C++, Java
// - Encoding(make, toJSON): C++, Java
public class BaseMessage {
    // BaseMessageType
    public static final int Type_NotDetermined = 0;
    public static final int Type_AppCore = 10;
    public static final int Type_AppCoreAck = 11;
    public static final int Type_App = 20;
    public static final int Type_Companion = 30;

    // JSON field name
    static final String OPEL_MESSAGE_KEY_MESSAGE_NUM = "messageId";
    static final String OPEL_MESSAGE_KEY_URI = "uri";
    static final String OPEL_MESSAGE_KEY_TYPE = "type";
    static final String OPEL_MESSAGE_KEY_IS_FILE_ATTACHED = "isFileAttached";
    static final String OPEL_MESSAGE_KEY_FILE_NAME = "fileName";
    static final String OPEL_MESSAGE_KEY_PAYLOAD = "payload";

    // Encoding to JSON
    public String toJSONString() {
        return this.toJSONNode().asText();
    }

    // Encoding to JSON
    public JsonNode toJSONNode() {
        ObjectMapper mapper = new ObjectMapper();
        ObjectNode thisObj = mapper.createObjectNode();
        thisObj.put(OPEL_MESSAGE_KEY_MESSAGE_NUM, "" + this.mMessageId);
        thisObj.put(OPEL_MESSAGE_KEY_URI, "" + this.mUri);
        thisObj.put(OPEL_MESSAGE_KEY_TYPE, "" + this.mUri);
        thisObj.put(OPEL_MESSAGE_KEY_IS_FILE_ATTACHED, "" + this.mUri);
        if (this.mIsFileAttached) thisObj.put(OPEL_MESSAGE_KEY_FILE_NAME, "" + this.mUri);
        thisObj.set(OPEL_MESSAGE_KEY_PAYLOAD, this.mPayload.toJSONNode());
        return thisObj;
    }

    // Get parameters
    public int getMessageId() {
        return this.mMessageId;
    }

    public String getUri() {
        return this.mUri;
    }

    public int getType() {
        return this.mType;
    }

    public boolean isFileAttached() {
        return this.mIsFileAttached;
    }

    public String getFileName() {
        return this.mFileName;
    }

    // Payload
    void setPayload(BaseMessagePayload payload) {
        this.mPayload = payload;
    }

    BaseMessagePayload getPayload() {
        return this.mPayload;
    }


    // Attach file on message to be sent
    public void attachFile(String filePath) {
        File file = new File(filePath);
        this.mFileName = file.getName();
    }

    // Set local file path when attached file has come
    public void setStoredFilePath(String storedFilePath) {
        this.mStoredFileName = storedFilePath;
    }

    public String getStoredFileName() {
        return this.mStoredFileName;
    }

    public BaseMessage(int messageId, String uri, int type, boolean isFileAttached, String
            fileName) {
        this.mMessageId = messageId;
        this.mUri = uri;
        this.mType = type;
        this.mIsFileAttached = isFileAttached;
        this.mFileName = fileName;
        this.mStoredFileName = "";
        this.mPayload = null;
    }

    // Initializer
    public BaseMessage(int messageId, String uri, int type) {
        this(messageId, uri, type, false, "");
    }

    // Exported to JSON
    private int mMessageId;
    private String mUri;
    private int mType;
    private boolean mIsFileAttached;
    private String mFileName;
    private BaseMessagePayload mPayload;

    // Private
    protected String mStoredFileName;
}

abstract class BaseMessagePayload {
    // Encoding to JSON
    abstract public ObjectNode toJSONNode();
}