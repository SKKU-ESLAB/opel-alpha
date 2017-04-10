#ifndef OPEL_DBUS_INTERFACE
#define OPEL_DBUS_INTERFACE
#include <glib.h>
#include <string>

static const gchar *dbus_path = "/org/opel/camera/daemon";
static const gchar *dbus_interface = "org.opel.camera.daemon";

//static const gchar *rec_init_request = "recInit";
static const gchar *rec_start_request = "recStart";
static const gchar *rec_stop_request = "recStop";

static const gchar *snap_start_request = "jpegStart";

static const gchar *opencv_start_request = "openCVStart";
static const gchar *opencv_stop_request = "openCVStop";

static const gchar *streaming_start_request = "streamingStart";
static const gchar *streaming_stop_request = "streamingStop";

static const gchar *sensor_overlay_start_request = "sensorOverlayStart";
static const gchar *sensor_overlay_stop_request = "sensorOverlayStop";

static const gchar *delay_streaming_start_request = "delayStreamingStart";
static const gchar *delay_streaming_stop_request = "delayStreamingStop";
static const gchar *event_rec_start_request = "eventRecStart";
static const gchar *event_rec_stop_request = "eventRecStop";


typedef struct _dbusReques{
  unsigned camera_num;
  std::string file_path;
  unsigned pid;
  unsigned fps;
  unsigned width;
  unsigned height;
  unsigned play_seconds;
}dbusRequest;

#endif /* OPEL_DBUS_INTERFACE */
