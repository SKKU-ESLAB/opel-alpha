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
static void send_rec_init(DBusConnection *conn, char* file_name)
{
  printf("init\n");
  DBusMessage *message;
  message = dbus_message_new_signal("/org/opel/camera/daemon", "org.opel.camera.daemon", "recInit");
  char* opel_data_dir = getenv("OPEL_DATA_DIR");
	char str[256] = "";
  strcat(str, opel_data_dir);
  strcat(str, "/");
	strcat(str, file_name);
	const char *file_path = str;
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

static void send_rec_start(DBusConnection *conn, char* file_name)
{
  printf("start\n");
  DBusMessage *message;
		message = dbus_message_new_signal(dbus_path, dbus_interface, snap_start_request);
  char* opel_data_dir = getenv("OPEL_DATA_DIR");
	char str[512] = "";
  strcat(str, opel_data_dir);
  strcat(str, "/");
	char prop[256] = ".jpeg";
	strcat(str, file_name);	
	strcat(str, prop);
	const char *file_path = str;
	printf("file_path : %s\n", str);	
	unsigned width = 1920;
	unsigned height = 1080;
	unsigned pid = getpid();
	printf("pid : %d\n", pid);
	dbus_message_append_args(message, 
			DBUS_TYPE_STRING, &file_path,
			DBUS_TYPE_UINT64, &pid,
			DBUS_TYPE_UINT64, &width,
			DBUS_TYPE_UINT64, &height,
			DBUS_TYPE_INVALID);
	dbus_connection_send (conn, message, NULL);
  dbus_message_unref(message);
}

static void send_rec_term(DBusConnection *conn)
{
  printf("termination\n");
  DBusMessage *message;
  message = dbus_message_new_signal(dbus_path, dbus_interface, opencv_start_request);
	unsigned pid = getpid();
	dbus_message_append_args(message,
			DBUS_TYPE_UINT64, &pid,
			DBUS_TYPE_INVALID);
  dbus_connection_send (conn, message, NULL);
  dbus_message_unref(message);
}

static void send_stream_start(DBusConnection *conn)
{
	printf("streaming start");
	DBusMessage *message;
	message = dbus_message_new_signal(dbus_path, dbus_interface, streaming_start_request);
	const char *ip_addr = "127.0.0.1";
	unsigned port = 5000;
	dbus_message_append_args(message,
			DBUS_TYPE_STRING, &ip_addr,
			DBUS_TYPE_UINT64, &port,
			DBUS_TYPE_INVALID);
	
	dbus_connection_send(conn, message, NULL);
	dbus_message_unref(message);

}

static void send_stream_stop(DBusConnection *conn)
{
	printf("streaming stop");
	DBusMessage *message;
	message = dbus_message_new_signal(dbus_path, dbus_interface, streaming_stop_request);
	dbus_connection_send(conn, message, NULL);
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
 if(argc <= 1)
	 return 0;

 printf("argv[1] = %s\n", argv[1]);

 for(;;)
 {
	 scanf("%d", &num); 
   if(num == 0)
	 	send_rec_init(conn, argv[1]); 
	 else if(num == 1) 
		send_rec_start(conn, argv[1]);	 
	 else if(num == 2)
		send_rec_term(conn);
	 else if(num == 3)
		 send_stream_start(conn);
	 else if(num == 4)
		 send_stream_stop(conn);
	 else
		 break;
 }
 return 0;
}
