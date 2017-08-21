package com.opel.opel_manager.model.message;

/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: 
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

public class MessageFactory {
    // string -> JSON -> BaseMessage
    static public BaseMessage makeMessageFromJSONString(String rawString) {

    }

    // Make AppCoreAckMessage from bottom
    static AppCoreAckMessage makeAppCoreAckMessage(int commandMessageId, AppcoreMessageCommandType commandType) {

    }

    // Make AppMessage from bottom

    // Make CompanionMessage from bottom


    // JSON -> BaseMessage / AppCoreMessage / AppMessage
}
