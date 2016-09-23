#include "OPELdbusInterface.h"
#include <string.h> 
#include <stdio.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-protocol.h>
#include <dbus/dbus-glib.h>
static void send_rec_init(DBusConnection *conn)
{
  printf("init\n");
  DBusMessage *message;
  message = dbus_message_new_signal("/org/opel/camera/daemon", "org.opel.camera.daemon", "recInit");
}
static void send_rec_start(DBusConnection *conn)
{
  printf("start\n");
  DBusMessage *message;
  message = dbus_message_new_signal(dbus_path, dbus_interface, rec_init_request);
}

static void send_rec_term(DBusConnection *conn)
{
  printf("termination\n");
  DBusMessage *message;
  message = dbus_message_new_signal(dbus_path, dbus_interface, rec_init_request);
}

int main(int argc, char** argv)
{
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
 if(argc == 1)
  return 0;
 for(int i=0; i < argc; i++)
 {
   if(!strcmp(argv[i], "-i"))
    send_rec_init(conn); 
   if(!strcmp(argv[i], "-s"))
     send_rec_start(conn);
 }
}
