#ifndef OPEL_CAM_REQUEST_H
#define OPEL_CAM_REQUEST_H
#include "OPELcamProperty.h"
#include <dbus/dbus.h>
#include <glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-protocol.h>


static const gchar *dbus_interface = "org.opel.camera.daemon";
static const gchar *init_request = "init_request";
static const gchar *start_request = "start_request";
static const gchar *stop_request = "stop_request";

static ElementProperty* initElementProperty(elementType type, 
    const std::string &name, const std::string &nickname, 
    unsigned fps, unsigned width, unsigned height, ...);

static DBusHandlerResult msg_dbus_filter(DBusConnection *conn, 
    DBusMessage *msg, void *user_data);

typedef enum _msgType{
  kINIT=0,
  kDYNAMIC,
}msgType;

typedef enum _dynamicType{
  kREC=0,
  kSNAPSHOT,
  kSTREAMING,
}dynamicType;


typedef enum _initType{
  kDEFAULT_TX1=0,
  kMODIFIED,
}initType;


typedef struct _dbusRequest{
  msgType msg_type; 
  union{
    initType init_type;
    dynamicType dynamic_type;
  }u;
  unsigned fps;
  unsigned width;
  unsigned height;
  unsigned num_frames;
}dbusRequest;

#endif /* OPEL_CAM_REQUEST_H */
