#ifndef OPEL_DBUS_INTERFACE
#define OPEL_DBUS_INTERFACE
#include <string>

static const char *dbus_path = "/org/opel/camera/daemon";
static const char *dbus_interface = "org.opel.camera.daemon";

//static const char *rec_init_request = "recInit";
static const char *rec_start_request = "recStart";
static const char *rec_stop_request = "recStop";

static const char *snap_start_request = "jpegStart";

static const char *opencv_start_request = "openCVStart";
static const char *opencv_stop_request = "openCVStop";

static const char *streaming_start_request = "streamingStart";
static const char *streaming_stop_request = "streamingStop";

static const char *sensor_overlay_start_request = "sensorOverlayStart";
static const char *sensor_overlay_stop_request = "sensorOverlayStop";

static const char *delay_streaming_start_request = "delayStreamingStart";
static const char *delay_streaming_stop_request = "delayStreamingStop";
static const char *event_rec_start_request = "eventRecStart";
static const char *event_rec_stop_request = "eventRecStop";


typedef struct _dbusRequest {
  unsigned camera_num;
  std::string file_path;
  unsigned pid;
  unsigned fps;
  unsigned width;
  unsigned height;
  unsigned play_seconds;
} dbusRequest;

typedef struct _dbusStreamingRequest {
  unsigned camera_num;
  std::string ip_address;
  unsigned port;
} dbusStreamingRequest;

typedef struct _dbusSensorOverlayRequest {
  unsigned camera_num;
  std::string sensor_name;
  unsigned pid;
} dbusSensorOverlayRequest;

typedef struct _dbusInitDelayRequest {
	unsigned camera_num;
	unsigned delay;
} dbusInitDelayRequest;

typedef struct _dbusEventRecRequest {
	unsigned camera_num;
	std::string file_path;
	unsigned play_seconds;
} dbusEventRecRequest;


#endif /* OPEL_DBUS_INTERFACE */
