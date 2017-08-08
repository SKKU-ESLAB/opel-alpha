/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
 *              Dongig Sin<dongig@skku.edu>
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

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <stdbool.h>
#include <pthread.h> 
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "DbusChannel.h"
#include "OPELdbugLog.h"

#define TERMINATION_SIGNAL "termination_event_driven"
#define CONFIGURATION_SIGNAL "config_event_driven"

const char* DbusChannel::kOPELInterface = "org.opel.sysAppManager";
const char* DbusChannel::kOPELSignal = "BaseMessage";

void DbusChannel::run() {
  // Run ListeningLoop on separate thread
  this->runListeningThread();

  // Run RoutedLoop on separate thread
  this->runRoutedLoop(true);
}

void DbusChannel::onRoutedMessage(BaseMessage* message) {
  const char* uriString = message->getUri().c_str();

  // Stir around all the target entry of given URI
  const char* targetDbusPath = NULL;
  const char* targetUriString = NULL;
  std::map<const char*, const char*>::iterator rtIter;
  for(rtIter = this->mDbusSlaveRoutingTable.begin();
      rtIter != this->mDbusSlaveRoutingTable.end();
      ++rtIter) {
    std::string entryUri(rtIter->first);
    const char* entryDbusPath = rtIter->second;

    size_t foundPos = entryUri.find(uriString);
    // Select the best matching target
    if(foundPos == 0) {
      if((targetUriString == NULL)
          || (strlen(targetUriString) < entryUri.size())) {
        targetDbusPath = entryDbusPath;
        targetUriString = entryUri.c_str();
      }
    }
  }

  // If the message did not routed at all, make a warning message
  if(targetDbusPath != NULL) {
    OPEL_DBG_VERB("Route to %s: %s",
        targetUriString, message->toRawString());
    this->sendRawStringToTarget(message->toRawString(), targetDbusPath);
  } else {
    OPEL_DBG_WARN("Message did not routed!: %s", message->toRawString());
  }
}

bool DbusChannel::checkMessageCompatible(BaseMessage* message) {
  if(message->isFileAttached() == true) {
    OPEL_DBG_WARN("DbusChannel does not support file attachment! %s",
        message->toRawString());
    return false;
  }
  return true;
}

void DbusChannel::sendRawStringToTarget(const char* rawString,
    const char* targetDbusPath) {
  if(this->mDbusConnection == NULL) {
    OPEL_DBG_ERR("Send rawString");
    return;
  }

  DBusMessage* dbusMessage;
  DBusError derr;
  dbus_error_init(&derr);

  // Initialize Dbus Signal
  dbusMessage = dbus_message_new_signal(
      targetDbusPath, DbusChannel::kOPELInterface, kOPELSignal);
  dbus_message_append_args(dbusMessage,
      DBUS_TYPE_STRING, &rawString, DBUS_TYPE_INVALID);

  if (dbus_error_is_set(&derr)) {
    OPEL_DBG_ERR("Dbus error on send data: %s", derr.message);
    dbus_error_free(&derr);
    return;
  }

  OPEL_DBG_VERB("Dbus send %s", rawString);	

  dbus_connection_send(this->mDbusConnection, dbusMessage, NULL);
  dbus_message_unref(dbusMessage);
}

void DbusChannel::runListeningThread() {
  pthread_create(&this->mListeningThread, NULL, DbusChannel::listeningLoop,
      (void*)this);
}

void* DbusChannel::listeningLoop(void* data) {
  DBusError error;
  GMainLoop *loop;
  DbusChannel* self = (DbusChannel*)data;

  OPEL_DBG_VERB("DbusChannel's Listening Loop: Start dbus initializing");

  // Initialize glib main loop
  loop = g_main_loop_new(NULL, false);   

  // Initialize dbus connection
  dbus_error_init(&error);
  self->mDbusConnection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
  if(dbus_error_is_set(&error) || self->mDbusConnection == NULL) {
    OPEL_DBG_ERR("Error connecting to the daemon bus: %s", error.message);
    dbus_error_free(&error);
  }

  // Initialize dbus bus
  int retval = dbus_bus_request_name(self->mDbusConnection, DbusChannel::kOPELInterface,
      DBUS_NAME_FLAG_ALLOW_REPLACEMENT, &error);
  dbus_connection_flush(self->mDbusConnection);
  switch(retval) {
    case -1:
      OPEL_DBG_ERR("Couldn't acquire name %s for our connection: %s",
          DbusChannel::kOPELInterface, error.message);
      dbus_error_free(&error);
      break;
    case DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER:
      OPEL_DBG_VERB("We now own the name %s!", DbusChannel::kOPELInterface);
      break;
    case DBUS_REQUEST_NAME_REPLY_IN_QUEUE:
      OPEL_DBG_VERB("We are standing in queue for our name!");
      break;
    case DBUS_REQUEST_NAME_REPLY_EXISTS:
      OPEL_DBG_VERB("The name we asked for already exists!");
      break;
    case DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER:
      OPEL_DBG_VERB("We already own this name!");
      break;
    default:
      OPEL_DBG_ERR("Unknown result = %d", retval);
      break;
  }
  dbus_bus_add_match(self->mDbusConnection,
      "type='signal',interface='org.opel.sysAppManager'", NULL);  
  dbus_connection_flush(self->mDbusConnection);

  // Add dbus callback function
  dbus_connection_add_filter(self->mDbusConnection,
      DbusChannel::onReceivedDbusMessage, self, NULL); 
  dbus_connection_setup_with_g_main(self->mDbusConnection, NULL);
  OPEL_DBG_VERB("Dbus response ready");

  // Start glib main loop
  g_main_loop_run(loop);
}

DBusHandlerResult DbusChannel::onReceivedDbusMessage(DBusConnection* connection,
    DBusMessage* dbusMessage, void* user_data) {
  // Check dbus interface & signal type
  DbusChannel* self = (DbusChannel*)user_data;
  if(dbus_message_is_signal(dbusMessage,
        DbusChannel::kOPELInterface, DbusChannel::kOPELSignal) == 0) {
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  if(self->mMessageRouter == NULL) {
    OPEL_DBG_ERR("MessageRouter is not connected to DbusChannel!");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  // Retrieve rawString from DbusMessage
  char* rawString = NULL;
  DBusError derr;
  dbus_error_init(&derr);
  dbus_message_get_args(dbusMessage, &derr,
      DBUS_TYPE_STRING, &rawString, DBUS_TYPE_INVALID);
  if(dbus_error_is_set(&derr)) {
    OPEL_DBG_ERR("Cannot retrieve rawString from DbusMessage!: %s",
        derr.message);
    dbus_error_free(&derr);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  } else if(rawString == NULL) {
    OPEL_DBG_ERR("Cannot retrieve rawString from DbusMessage!");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  // Parse rawString into OPEL message
  BaseMessage* message = BaseMessage::parse(rawString);
  if(message == NULL) {
    OPEL_DBG_ERR("Received message is not OPEL message!: %s", rawString);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  OPEL_DBG_VERB("Received rawMessage from DbusChannel: %s", rawString);

  // Route message to the target of the OPEL message
  self->mMessageRouter->routeMessage(message);
  dbus_message_unref(dbusMessage);
  return DBUS_HANDLER_RESULT_HANDLED;
}

void DbusChannel::addRoutingEntry(const char* uriString, const char* dbusPath) {
  std::pair<const char*, const char*> newEntry(uriString, dbusPath);
  this->mDbusSlaveRoutingTable.insert(newEntry);
}
