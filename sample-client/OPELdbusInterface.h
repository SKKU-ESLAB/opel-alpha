#ifndef OPEL_DBUS_INTERFACE
#define OPEL_DBUS_INTERFACE
#include <glib.h>
#include <string>

static const gchar *dbus_path = "/org/opel/camera/daemon";
static const gchar *dbus_interface = "org.opel.camera.daemon";

static const gchar *snap_start_request = "jpegStart";
static const gchar *rec_init_request = "recInit";
static const gchar *rec_start_request = "recStart";
static const gchar *rec_stop_request = "recStop";

typedef struct _dbusReques{
  std::string file_path;
  unsigned pid;
  unsigned fps;
  unsigned width;
  unsigned height;
  unsigned play_seconds;
}dbusRequest;

#endif /* OPEL_DBUS_INTERFACE */
