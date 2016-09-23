#include "OPELdbusInterface.h"
#include <string.h> 
#include <stdio.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-protocol.h>
#include <dbus/dbus-glib.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
static void send_rec_init(DBusConnection *conn)
{
  printf("init\n");
  DBusMessage *message;
  message = dbus_message_new_signal("/org/opel/camera/daemon", "org.opel.camera.daemon", "recInit");
	const char *file_path = "/home/ubuntu/hihihi.mp4";
	unsigned pid = getpid();
	std::cout << "pid : " << pid << std::endl;
	unsigned fps = 30;
	unsigned width = 1920;
	unsigned height = 1080;
	unsigned play_seconds = 10;

	dbus_message_append_args(message, 
			DBUS_TYPE_STRING, &file_path,
			DBUS_TYPE_UINT64, &pid,
			DBUS_TYPE_UINT64, &fps, 
			DBUS_TYPE_UINT64, &width,
			DBUS_TYPE_UINT64, &height,
			DBUS_TYPE_UINT64, &play_seconds,
			DBUS_TYPE_INVALID);
  dbus_connection_send (conn, message, NULL);
  dbus_message_unref(message);
}

static void send_rec_start(DBusConnection *conn)
{
  printf("start\n");
  DBusMessage *message;
	message = dbus_message_new_signal(dbus_path, dbus_interface, rec_start_request);
	
	unsigned pid = getpid();
	dbus_message_append_args(message, 
			DBUS_TYPE_UINT64, &pid,
			DBUS_TYPE_INVALID);
	dbus_connection_send (conn, message, NULL);
  dbus_message_unref(message);
}

static void send_rec_term(DBusConnection *conn)
{
  printf("termination\n");
  DBusMessage *message;
  message = dbus_message_new_signal(dbus_path, dbus_interface, rec_init_request);

  dbus_connection_send (conn, message, NULL);
  dbus_message_unref(message);
}

int main(int argc, char** argv)
{
 int num;
 DBusConnection *conn;
 DBusError error;
 dbus_error_init (&error);
 conn = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
 if(dbus_error_is_set(&error))
 {
   printf("fail\n");
   dbus_error_free(&error);
   return 1;
 }
 for(;;)
 {
	 scanf("%d", &num); 
   if(num == 0)
	 	send_rec_init(conn); 
	 else if(num == 1) 
		 send_rec_start(conn);
	 else
		 break;
 }
 return 0;
}
