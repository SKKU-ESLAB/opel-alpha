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

import android.util.Log;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ObjectNode;

import java.io.IOException;

public class MessageFactory {
    private static final String TAG = "MessageFactory";

    // string -> JSON -> BaseMessage
    static public BaseMessage makeMessageFromJSONString(String rawString) {
        ObjectMapper mapper = new ObjectMapper();
        JsonNode rootNode = null;
        try {
            rootNode = mapper.readTree(rawString);
        } catch (IOException e) {
            e.printStackTrace();
            Log.e(TAG, "Failed to parse JSON string");
        }
        return makeBaseMessageFromJSON(rootNode);
    }

    // Make AppCoreMessage from bottom
    static AppCoreMessage makeAppCoreMessage(int commandType) {
        return new AppCoreMessage(commandType);
    }

    // JSON -> BaseMessage / AppCoreAckMessage / CompanionMessage
    static BaseMessage makeBaseMessageFromJSON(JsonNode messageObj) {
        ObjectNode thisObj = (ObjectNode) messageObj;

        // messageId
        String messageIdStr = thisObj.get(BaseMessage.OPEL_MESSAGE_KEY_MESSAGE_NUM).asText();
        int messageId = Integer.parseInt(messageIdStr);

        // URI
        String uri = thisObj.get(BaseMessage.OPEL_MESSAGE_KEY_URI).asText();

        // Type
        String typeStr = thisObj.get(BaseMessage.OPEL_MESSAGE_KEY_TYPE).asText();
        int type = Integer.parseInt(typeStr);

        // isFileAttached
        String isFileAttachedStr = thisObj.get(BaseMessage.OPEL_MESSAGE_KEY_IS_FILE_ATTACHED)
                .asText();
        boolean isFileAttached = (Integer.parseInt(isFileAttachedStr) == 1);

        // fileName
        String fileName = thisObj.get(BaseMessage.OPEL_MESSAGE_KEY_FILE_NAME).asText();

        // payload
        JsonNode payloadObj = thisObj.get(BaseMessage.OPEL_MESSAGE_KEY_PAYLOAD);

        // Allocate and initialize a new BaseMessage
        BaseMessage newMessage = new BaseMessage(messageId, uri, type, isFileAttached, fileName);
        switch (type) {
            case BaseMessage.Type_AppCoreAck: {
                AppCoreAckMessage messagePayload = makeAppCoreAckMessageFromJSON(payloadObj);
                newMessage.setPayload(messagePayload);
                break;
            }
            case BaseMessage.Type_Companion: {
                CompanionMessage messagePayload = makeCompanionMessageFromJSON(payloadObj);
                newMessage.setPayload(messagePayload);
                break;
            }
            case BaseMessage.Type_AppCore:
            case BaseMessage.Type_App:
            case BaseMessage.Type_NotDetermined:
            default:
                // These types cannot be handled.
                break;
        }

        return newMessage;
    }

    static AppCoreAckMessage makeAppCoreAckMessageFromJSON(JsonNode messagePayloadObj) {
        ObjectNode thisObj = (ObjectNode) messagePayloadObj;

        // commandMessageId
        String commandMessageIdStr = thisObj.get(AppCoreAckMessage
                .APPCORE_ACK_MESSAGE_KEY_COMMAND_MESSAGE_NUM).asText();
        int commandMessageId = Integer.parseInt(commandMessageIdStr);

        // commandType
        String commandTypeStr = thisObj.get(AppCoreAckMessage
                .APPCORE_ACK_MESSAGE_KEY_COMMAND_TYPE).asText();
        int commandType = Integer.parseInt(commandTypeStr);

        // payload (AppCoreAckMessage's)
        ObjectNode payloadObj = (ObjectNode) thisObj.get(AppCoreAckMessage
                .APPCORE_ACK_MESSAGE_KEY_PAYLOAD);

        // Allocate and initialize a new AppCoreAckMessage
        AppCoreAckMessage newMessage = new AppCoreAckMessage(commandMessageId, commandType);
        newMessage.setPayload(payloadObj);

        return newMessage;
    }

    static CompanionMessage makeCompanionMessageFromJSON(JsonNode messagePayloadObj) {
        ObjectNode thisObj = (ObjectNode) messagePayloadObj;

        // commandType
        String commandTypeStr = thisObj.get(CompanionMessage.COMPANION_MESSAGE_KEY_COMMAND_TYPE)
                .asText();
        int commandType = Integer.parseInt(commandTypeStr);

        // payload (CompanionMessage's)
        ObjectNode payloadObj = (ObjectNode) thisObj.get(CompanionMessage
                .COMPANION_MESSAGE_KEY_PAYLOAD);

        // Allocate and initialize a new AppCoreAckMessage
        CompanionMessage newMessage = new CompanionMessage(commandType);
        newMessage.setPayload(payloadObj);

        return newMessage;
    }
}