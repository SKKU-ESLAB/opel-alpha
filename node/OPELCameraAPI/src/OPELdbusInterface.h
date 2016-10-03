#ifndef OPEL_DBUS_INTERFACE
#define OPEL_DBUS_INTERFACE
#include <string>

static const char *dbus_path = "/org/opel/camera/daemon";
static const char *dbus_interface = "org.opel.camera.daemon";

static const char *snap_start_request = "jpegStart";
static const char *rec_init_request = "recInit";

static const char *streaming_start_request = "streamingStart";
static const char *streaming_stop_request = "streamingStop";

typedef struct _dbusReques{
  std::string file_path;
  unsigned pid;
  unsigned fps;
  unsigned width;
  unsigned height;
  unsigned play_seconds;
}dbusRequest;

typedef struct _dbusStreamingRequest{
	std::string ip_address;
		  unsigned port;
}dbusStreamingRequest;
#endif /* OPEL_DBUS_INTERFACE */
