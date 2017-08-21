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

import com.fasterxml.jackson.databind.node.ObjectNode;
import com.opel.opel_manager.model.message.params.ParamsSendConfigPage;
import com.opel.opel_manager.model.message.params.ParamsSendEventPage;

// CompanionMessage: message sent to companion device
// - Decoding(makeFromJSON): Java
// - Encoding(make, toJSON): JavaScript
public class CompanionMessage extends BaseMessagePayload {
    // CompanionMessageCommandType
    public static final int Type_NotDetermined = 0;
    public static final int Type_SendEventPage = 1; // params: string legacyData
    public static final int Type_SendConfigPage = 2; // params: string legacyData

    // JSON field name
    static final String COMPANION_MESSAGE_KEY_COMMAND_TYPE = "commandType";
    static final String COMPANION_MESSAGE_KEY_PAYLOAD = "payload";

    // Encoding to JSON: not implemented for Java
    @Override
    public ObjectNode toJSONNode() {
        return null;
    }

    // Get parameters
    public ParamsSendEventPage getParamsSendEventPage() {
        ObjectNode paramsObj = (ObjectNode) this.mCompanionPayloadObj;

        String legacyData = paramsObj.get("legacyData").asText();

        return new ParamsSendEventPage(legacyData);
    }

    public ParamsSendConfigPage getParamsSendConfigPage() {
        ObjectNode paramsObj = (ObjectNode) this.mCompanionPayloadObj;

        String legacyData = paramsObj.get("legacyData").asText();

        return new ParamsSendConfigPage(legacyData);
    }

    public int getCommandType() {
        return this.mCommandType;
    }

    // Set parameters
    public void setPayload(ObjectNode companionPayloadObj) {
        this.mCompanionPayloadObj = companionPayloadObj;
    }

    public CompanionMessage(int commandType) {
        this.mCommandType = commandType;
    }

    private int mCommandType;
    private ObjectNode mCompanionPayloadObj;
}