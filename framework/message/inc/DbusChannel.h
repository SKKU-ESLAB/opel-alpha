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

#ifndef __DBUS_CHANNEL_H__
#define __DBUS_CHANNEL_H__

#include <iostream>
#include <map>
#include <pthread.h>

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "MessageRouter.h"
#include "BaseMessage.h"

class DbusChannel: public Channel {
	public:
		DbusChannel(MessageRouter* messageRouter)
    : Channel(messageRouter, "DbusChannel"), mDbusConnection(NULL) {
    }

    // Channel function
    virtual void run();

    // DbusChannel-specific function (Dbus Slave Routing Table)
    void addRoutingEntry(const char* uriString, const char* dbusPath);

	protected:
    // Channel function (RoutedThread)
    virtual void onRoutedMessage(BaseMessage* message);

    // Send raw string to target process on RoutedThread
    void sendRawStringToTarget(const char* rawString,
        const char* targetDbusPath);

    // Listening Thread
    void runListeningThread();
    static void* listeningLoop(void* data);
    static DBusHandlerResult onReceivedDbusMessage(
        DBusConnection *connection, DBusMessage* dbusMessage, void* user_data);

    // ListeningThread
    pthread_t mListeningThread;

    static const char* kOPELInterface;
    static const char* kOPELSignal;
    DBusConnection *mDbusConnection;

    // Dbus Slave Routing Table
    //   entry = std::pair<const char* uriString, const char* dbusPath>
    std::map<const char*, const char*> mDbusSlaveRoutingTable;
};
#endif // !defined(__DBUS_CHANNEL_H__)
