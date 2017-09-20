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
#include <sys/types.h>
#include <unistd.h>

#include "DbusChannel.h"
#include "OPELdbugLog.h"
#include "MessageFactory.h"
#include "BaseMessage.h"

const char* DbusChannel::kOPELInterface = "org.opel.dbuschannel";
const char* DbusChannel::kOPELSignal = "BaseMessage";
const char* DbusChannel::kOPELPath ="/org/opel/dbuschannel";

void DbusChannel::initializeDbus() {
  DBusError error;
  
  OPEL_DBG_VERB("DbusChannel: Start dbus initializing");

  // Initialize dbus connection
  dbus_error_init(&error);
  this->mDbusConnection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
  if(dbus_error_is_set(&error) || this->mDbusConnection == NULL) {
    OPEL_DBG_ERR("Error connecting to the daemon bus: %s", error.message);
    dbus_error_free(&error);
  }

  // Initialize dbus bus
  int retval = dbus_bus_request_name(this->mDbusConnection,
      DbusChannel::kOPELInterface,
      DBUS_NAME_FLAG_ALLOW_REPLACEMENT, &error);
  dbus_connection_flush(this->mDbusConnection);
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
  dbus_bus_add_match(this->mDbusConnection,
      "type='signal',interface='org.opel.dbuschannel'", NULL);  
  dbus_connection_flush(this->mDbusConnection);
}

void DbusChannel::run() {
  // Initialize d-bus
  this->initializeDbus();

  // Run ListeningLoop on separate thread
  this->runListeningThread();

  // Run RoutedLoop on separate thread
  this->runRoutedLoop(true);
}

void DbusChannel::onRoutedMessage(BaseMessage* message) {
  const char* uriString = message->getUri().c_str();

  this->sendRawStringToTarget(message->toJSONString());
}

void DbusChannel::sendRawStringToTarget(const char* rawString) {
  if(this->mDbusConnection == NULL) {
    OPEL_DBG_VERB("Cannot send rawString since dbus is disconnected");
    return;
  }

  DBusMessage* dbusMessage;
  DBusError derr;
  dbus_error_init(&derr);
  int senderPid = getpid();

  // Initialize Dbus Signal
  dbusMessage = dbus_message_new_signal(
      kOPELPath, DbusChannel::kOPELInterface, DbusChannel::kOPELSignal);
  dbus_message_append_args(dbusMessage,
      DBUS_TYPE_STRING, &rawString,
      DBUS_TYPE_INT32, &senderPid,
      DBUS_TYPE_INVALID);

  if (dbus_error_is_set(&derr)) {
    OPEL_DBG_ERR("Dbus error on send data: %s", derr.message);
    dbus_error_free(&derr);
    dbus_message_unref(dbusMessage);
    return;
  }

  OPEL_DBG_VERB("(%d) Dbus send %s", getpid(), rawString);	

  dbus_connection_send(this->mDbusConnection, dbusMessage, NULL);
  dbus_message_unref(dbusMessage);
}

void DbusChannel::runListeningThread() {
  pthread_create(&this->mListeningThread, NULL, DbusChannel::listeningLoop,
      (void*)this);
}

void* DbusChannel::listeningLoop(void* data) {
  GMainLoop *loop;
  DbusChannel* self = (DbusChannel*)data;

  OPEL_DBG_VERB("DbusChannel's Listening Loop: Start dbus-enabled glib main loop");

  // Initialize glib main loop
  loop = g_main_loop_new(NULL, false);   

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
  int senderPid = -1;
  char* rawString = NULL;
  DBusError derr;
  dbus_error_init(&derr);
  dbus_message_get_args(dbusMessage, &derr,
      DBUS_TYPE_STRING, &rawString,
      DBUS_TYPE_INT32, &senderPid,
      DBUS_TYPE_INVALID);
  if(dbus_error_is_set(&derr)) {
    OPEL_DBG_ERR("Cannot retrieve rawString from DbusMessage!: %s",
        derr.message);
    dbus_error_free(&derr);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  } else if(rawString == NULL) {
    OPEL_DBG_ERR("Cannot retrieve rawString from DbusMessage!");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  // Drop message sent from this process
  if(getpid() == senderPid) {
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  // Parse rawString into OPEL message
  BaseMessage* message = MessageFactory::makeMessageFromJSONString(rawString);
  if(message == NULL) {
    OPEL_DBG_ERR("Received message is not OPEL message!: %s", rawString);
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  OPEL_DBG_VERB("(pid=%d) Received rawMessage from DbusChannel: %s", getpid(), rawString);

  // Route message to the target of the OPEL message
  self->mMessageRouter->routeMessage(message);
  return DBUS_HANDLER_RESULT_HANDLED;
}
