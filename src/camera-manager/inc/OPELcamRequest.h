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

extern DBusHandlerResult msg_dbus_filter(DBusConnection *conn,
    DBusMessage *msg, void *_type_element_vector);


typedef struct _dbusRequest {
  unsigned camera_num;
  const char* file_path;
  unsigned pid;
  unsigned fps;
  unsigned width;
  unsigned height;
  unsigned play_seconds;
  bool is_start;
} dbusRequest;

typedef struct _dbusStreamingRequest {
  unsigned camera_num;
  const char* ip_address;
  unsigned port;
  unsigned delay;
} dbusStreamingRequest;

typedef struct _dbusSensorOverlayRequest {
  unsigned camera_num;
  const char* sensor_name;
  unsigned pid;
} dbusSensorOverlayRequest;

typedef struct _dbusInitDelayRequest {
	unsigned camera_num;
	unsigned delay;
} dbusInitDelayRequest;

typedef struct _dbusEventRecRequest {
	unsigned camera_num;
	const char* file_path;
	unsigned play_seconds;
} dbusEventRecRequest;

void closeFile(FILE *_fout);
unsigned fileWrite(const char *file_path, char *buffer, unsigned size);
static gboolean timeOutCallback(gpointer _request_elements);
static GstPadProbeReturn event_probe_cb(GstPad *pad, GstPadProbeInfo *info,
    gpointer user_data);
void checkRemainRequest(void);
static gboolean unLinkCallBack(gpointer _request_elements);
void sendReply(DBusConnection *_conn, DBusMessage *_reply, bool is_success);

#endif /* OPEL_CAM_REQUEST_H */