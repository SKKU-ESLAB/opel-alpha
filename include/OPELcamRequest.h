#ifndef OPEL_CAM_REQUEST_H
#define OPEL_CAM_REQUEST_H
#include <dbus/dbus.h>
#include <glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-protocol.h>
#include <dbus/dbus-glib.h>
#include <string>
#include <gst/gst.h>


static const gchar *dbus_interface = "org.opel.camera.daemon";

static const gchar *rec_init_request = "recInit";
static const gchar *rec_start_request = "recStart";
static const gchar *rec_stop_request = "recStop";

static const gchar *opencv_init_request = "openCVInit";
static const gchar *opencv_start_request = "openCVStart";
static const gchar *opencv_stop_request = "openCVStop";

extern DBusHandlerResult msg_dbus_filter(DBusConnection *conn, 
    DBusMessage *msg, void *_type_element_vector);


typedef struct _dbusReques{
  std::string file_path;
  unsigned pid;
  unsigned fps;
  unsigned width;
  unsigned height;
  unsigned play_seconds;
}dbusRequest;


static gboolean timeOutCallback(gpointer _request_elements);
static GstPadProbeReturn event_probe_cb(GstPad *pad, GstPadProbeInfo *info,
        gpointer user_data);

#endif /* OPEL_CAM_REQUEST_H */
