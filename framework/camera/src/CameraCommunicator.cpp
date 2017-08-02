#include "CameraCommunicator.h"

CameraCommunicator::CameraCommunicator()
{
}

CameraCommunicator::~CameraCommunicator()
{
  if (this->mDBusConnection != NULL)
    dbus_connection_unref(this->mDBusConnection);
}

bool CameraCommunicator::initCommunication(void *cam,
    DBusHandlerResult (*filter)(DBusConnection*, DBusMessage*, void*))
{
  OPEL_LOG_DBG(CAM, "D-Bus initialization");

  dbus_error_init(&this->mDBusError);
  this->mDBusConnection = dbus_bus_get(DBUS_BUS_SYSTEM, &this->mDBusError);
  if (dbus_error_is_set(&this->mDBusError)) {
    OPEL_LOG_ERR(CAM, "D-Bus bus system registration failed");
    dbus_error_free(&this->mDBusError);
    return false;
  }

  dbus_bus_add_match(this->mDBusConnection,
      "type='signal', interface='org.opel.camera.daemon'", &this->mDBusError);
  if (dbus_error_is_set(&this->mDBusError)) {
    OPEL_LOG_ERR(CAM, "D-Bus 'adds a match rule' failed");
    dbus_error_free(&this->mDBusError);
    return false;
  }

  if (!dbus_connection_add_filter(this->mDBusConnection, (*filter), cam, NULL)) {
    OPEL_LOG_ERR(CAM, "D-Bus add filter failed");
    return false;
  }

  dbus_connection_setup_with_g_main(this->mDBusConnection, NULL);

  return true;
}

DBusHandlerResult CameraCommunicator::recordingStart(DBusMessage *msg,
    CameraController *controller, CameraConfigParser *config_parser)
{
  OPEL_LOG_DBG(CAM, "Get recording start request");
  bool ret;

  dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->camera_id),
      DBUS_TYPE_UINT64, &(msg_handle->play_seconds),
      DBUS_TYPE_STRING, &(msg_handle->file_path),
      DBUS_TYPE_UINT64, &(msg_handle->width),
      DBUS_TYPE_UINT64, &(msg_handle->height),
      DBUS_TYPE_UINT64, &(msg_handle->fps),
      DBUS_TYPE_INVALID);

  OPEL_LOG_DBG(CAM, "PID : %u", msg_handle->pid);
  OPEL_LOG_DBG(CAM, "Camera ID : %u", msg_handle->camera_id);
  OPEL_LOG_DBG(CAM, "Playing Time : %usec", msg_handle->play_seconds);
  OPEL_LOG_DBG(CAM, "File Path : %s", msg_handle->file_path);
  OPEL_LOG_DBG(CAM, "Width : %u", msg_handle->width);
  OPEL_LOG_DBG(CAM, "Height : %u", msg_handle->height);
  OPEL_LOG_DBG(CAM, "FPS : %u", msg_handle->fps);

  GstElement *recording_bin = config_parser->getRecordingBin();
  CameraRequest *request = new CameraRequest(kRecordingStart, recording_bin);
  request->setMsgHandle(msg_handle);

  CameraDevice *device = controller->getDeviceById(msg_handle->camera_id);
  request->setCameraDevice(device);
  ret = device->processRequest(request);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "Recording start failed");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult CameraCommunicator::recordingStop(DBusMessage *msg,
    CameraController *controller, CameraConfigParser *config_parser)
{
  OPEL_LOG_DBG(CAM, "Get recording stop request");
  bool ret;

  dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->camera_id),
      DBUS_TYPE_STRING, &(msg_handle->file_path),
      DBUS_TYPE_INVALID);

  OPEL_LOG_DBG(CAM, "PID : %u", msg_handle->pid);
  OPEL_LOG_DBG(CAM, "Camera ID : %u", msg_handle->camera_id);
  OPEL_LOG_DBG(CAM, "File Path : %s", msg_handle->file_path);

  CameraRequest *request = new CameraRequest(kRecordingStop);
  request->setMsgHandle(msg_handle);

  CameraDevice *device = controller->getDeviceById(msg_handle->camera_id);
  request->setCameraDevice(device);
  ret = device->processRequest(request);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "Recording stop failed");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult CameraCommunicator::snapshotStart(DBusMessage *msg,
    CameraController *controller, CameraConfigParser *config_parser)
{
  OPEL_LOG_DBG(CAM, "Get snapshot start request");
  bool ret;

  dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->camera_id),
      DBUS_TYPE_UINT64, &(msg_handle->play_seconds),
      DBUS_TYPE_STRING, &(msg_handle->file_path),
      DBUS_TYPE_UINT64, &(msg_handle->width),
      DBUS_TYPE_UINT64, &(msg_handle->height),
      DBUS_TYPE_INVALID);

  OPEL_LOG_DBG(CAM, "PID : %u", msg_handle->pid);
  OPEL_LOG_DBG(CAM, "Camera ID : %u", msg_handle->camera_id);
  OPEL_LOG_DBG(CAM, "Delayed Time : %u", msg_handle->play_seconds);
  OPEL_LOG_DBG(CAM, "File Path : %s", msg_handle->file_path);
  OPEL_LOG_DBG(CAM, "Width : %u", msg_handle->width);
  OPEL_LOG_DBG(CAM, "Height : %u", msg_handle->height);

  GstElement *snapshot_bin = config_parser->getSnapshotBin();
  CameraRequest *request = new CameraRequest(kSnapshot, snapshot_bin);
  request->setMsgHandle(msg_handle);

  CameraDevice *device = controller->getDeviceById(msg_handle->camera_id);
  request->setCameraDevice(device);
  ret = device->processRequest(request);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "Snapshot failed");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult CameraCommunicator::streamingStart(DBusMessage *msg,
    CameraController *controller, CameraConfigParser *config_parser)
{
  OPEL_LOG_DBG(CAM, "Get streaming start request");
  bool ret;

  dbusStreamingRequest *msg_handle = (dbusStreamingRequest*)malloc(sizeof(dbusStreamingRequest));

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->camera_id),
      DBUS_TYPE_STRING, &(msg_handle->ip_address),
      DBUS_TYPE_UINT64, &(msg_handle->port),
      DBUS_TYPE_INVALID);

  OPEL_LOG_DBG(CAM, "PID : %u", msg_handle->pid);
  OPEL_LOG_DBG(CAM, "Camera ID : %u", msg_handle->camera_id);
  OPEL_LOG_DBG(CAM, "IP Address : %s", msg_handle->ip_address);
  OPEL_LOG_DBG(CAM, "Port : %u", msg_handle->port);

  GstElement *streaming_bin = config_parser->getStreamingBin();
  CameraRequest *request = new CameraRequest(kStreamingStart, streaming_bin);
  request->setMsgHandle(msg_handle);

  CameraDevice *device = controller->getDeviceById(msg_handle->camera_id);
  request->setCameraDevice(device);
  ret = device->processRequest(request);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "Streaming start failed");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult CameraCommunicator::streamingStop(DBusMessage *msg,
    CameraController *controller, CameraConfigParser *config_parser)
{
  OPEL_LOG_DBG(CAM, "Get streaming stop request");
  bool ret;

  dbusStreamingRequest *msg_handle = (dbusStreamingRequest*)malloc(sizeof(dbusStreamingRequest));

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->camera_id),
      DBUS_TYPE_STRING, &(msg_handle->ip_address),
      DBUS_TYPE_UINT64, &(msg_handle->port),
      DBUS_TYPE_INVALID);

  OPEL_LOG_DBG(CAM, "PID : %u", msg_handle->pid);
  OPEL_LOG_DBG(CAM, "Camera ID : %u", msg_handle->camera_id);
  OPEL_LOG_DBG(CAM, "IP Address : %s", msg_handle->ip_address);
  OPEL_LOG_DBG(CAM, "Port : %u", msg_handle->port);

  CameraRequest *request = new CameraRequest(kStreamingStop);
  request->setMsgHandle(msg_handle);

  CameraDevice *device = controller->getDeviceById(msg_handle->camera_id);
  request->setCameraDevice(device);
  ret = device->processRequest(request);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "Streaming stop failed");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult CameraCommunicator::preRecordingInit(DBusMessage *msg,
    CameraController *controller, CameraConfigParser *config_parser)
{
  OPEL_LOG_DBG(CAM, "Get pre-recording init request");
  bool ret;

  dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->camera_id),
      DBUS_TYPE_UINT64, &(msg_handle->play_seconds),
      DBUS_TYPE_INVALID);

  OPEL_LOG_DBG(CAM, "PID : %u", msg_handle->pid);
  OPEL_LOG_DBG(CAM, "Camera ID : %u", msg_handle->camera_id);
  OPEL_LOG_DBG(CAM, "Pre-Recording Time : %usec", msg_handle->play_seconds);

  GstElement *pre_recording_init_bin = config_parser->getPreRecordingInitBin();
  CameraRequest *request = new CameraRequest(kPreRecordingInit, pre_recording_init_bin);
  request->setMsgHandle(msg_handle);

  CameraDevice *device = controller->getDeviceById(msg_handle->camera_id);
  request->setCameraDevice(device);
  ret = device->processRequest(request);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "Pre-recording init failed");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult CameraCommunicator::preRecordingStart(DBusMessage *msg,
    CameraController *controller, CameraConfigParser *config_parser)
{
  OPEL_LOG_DBG(CAM, "Get pre-recording start request");
  bool ret;

  dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->camera_id),
      DBUS_TYPE_UINT64, &(msg_handle->play_seconds),
      DBUS_TYPE_STRING, &(msg_handle->file_path),
      DBUS_TYPE_UINT64, &(msg_handle->width),
      DBUS_TYPE_UINT64, &(msg_handle->height),
      DBUS_TYPE_UINT64, &(msg_handle->fps),
      DBUS_TYPE_INVALID);

  OPEL_LOG_DBG(CAM, "PID : %u", msg_handle->pid);
  OPEL_LOG_DBG(CAM, "Camera ID : %u", msg_handle->camera_id);
  OPEL_LOG_DBG(CAM, "Playing Time : %usec", msg_handle->play_seconds);
  OPEL_LOG_DBG(CAM, "File Path : %s", msg_handle->file_path);
  OPEL_LOG_DBG(CAM, "Width : %u", msg_handle->width);
  OPEL_LOG_DBG(CAM, "Height : %u", msg_handle->height);
  OPEL_LOG_DBG(CAM, "FPS : %u", msg_handle->fps);

  GstElement *pre_recording_bin = config_parser->getPreRecordingBin();
  CameraRequest *request = new CameraRequest(kPreRecordingStart, pre_recording_bin);
  request->setMsgHandle(msg_handle);

  CameraDevice *device = controller->getDeviceById(msg_handle->camera_id);
  request->setCameraDevice(device);
  ret = device->processRequest(request);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "Pre-recording start failed");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult CameraCommunicator::preRecordingStop(DBusMessage *msg,
    CameraController *controller, CameraConfigParser *config_parser)
{
  OPEL_LOG_DBG(CAM, "Get pre recording stop request");
  bool ret;

  dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->camera_id),
      DBUS_TYPE_STRING, &(msg_handle->file_path),
      DBUS_TYPE_INVALID);

  OPEL_LOG_DBG(CAM, "PID : %u", msg_handle->pid);
  OPEL_LOG_DBG(CAM, "Camera ID : %u", msg_handle->camera_id);
  OPEL_LOG_DBG(CAM, "File Path : %s", msg_handle->file_path);

  CameraRequest *request = new CameraRequest(kPreRecordingStop);
  request->setMsgHandle(msg_handle);

  CameraDevice *device = controller->getDeviceById(msg_handle->camera_id);
  request->setCameraDevice(device);
  ret = device->processRequest(request);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "Pre-recording stop failed");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult CameraCommunicator::openCVStart(DBusMessage *msg,
    CameraController *controller, CameraConfigParser *config_parser)
{
  OPEL_LOG_DBG(CAM, "Get OpenCV start request");
  bool ret;

  dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->camera_id),
      DBUS_TYPE_INVALID);

  OPEL_LOG_DBG(CAM, "PID : %u", msg_handle->pid);
  OPEL_LOG_DBG(CAM, "Camera ID : %u", msg_handle->camera_id);

  if (msg_handle->camera_id != 0) {
    OPEL_LOG_WARN(CAM, "Current version supports only using camera id 0");
    return DBUS_HANDLER_RESULT_HANDLED;
  }

  CameraDevice *device = controller->getDeviceById(msg_handle->camera_id);
  if (device->getOpenCVNumUsers() > 0) {
    OPEL_LOG_DBG(CAM, "OpenCV already started");
    return DBUS_HANDLER_RESULT_HANDLED;
  }
  
  GstElement *opencv_bin = config_parser->getOpenCVBin();
  OPELRawRequest *raw_request = OPELRawRequest::getInstance(kOpenCVStart, opencv_bin);
  device->increaseOpenCVNumUsers();
  
  CameraRequest *request = (CameraRequest *)raw_request;
  request->setMsgHandle(msg_handle);

  request->setCameraDevice(device);
  ret = device->processRequest(request);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "OpenCV start failed");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult CameraCommunicator::openCVStop(DBusMessage *msg,
    CameraController *controller, CameraConfigParser *config_parser)
{
  OPEL_LOG_DBG(CAM, "Get OpenCV stop request");
  bool ret;

  dbusRequest *msg_handle = (dbusRequest*)malloc(sizeof(dbusRequest));

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->camera_id),
      DBUS_TYPE_INVALID);

  OPEL_LOG_DBG(CAM, "PID : %u", msg_handle->pid);
  OPEL_LOG_DBG(CAM, "Camera ID : %u", msg_handle->camera_id);

  if (msg_handle->camera_id != 0) {
    OPEL_LOG_WARN(CAM, "Current version supports only using camera id 0");
    return DBUS_HANDLER_RESULT_HANDLED;
  }

  CameraRequest *request = new CameraRequest(kOpenCVStop);
  request->setMsgHandle(msg_handle);

  CameraDevice *device = controller->getDeviceById(msg_handle->camera_id);
  request->setCameraDevice(device);
  ret = device->processRequest(request);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "OpenCV stop failed");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult CameraCommunicator::sensorOverlayStart(DBusMessage *msg,
    CameraController *controller, CameraConfigParser *config_parser)
{
  OPEL_LOG_DBG(CAM, "Get sensor start request");
  bool ret;

  dbusSensorRequest *msg_handle = (dbusSensorRequest*)malloc(sizeof(dbusSensorRequest));

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->camera_id),
      DBUS_TYPE_STRING, &(msg_handle->sensor_name),
      DBUS_TYPE_INVALID);

  OPEL_LOG_DBG(CAM, "PID : %u", msg_handle->pid);
  OPEL_LOG_DBG(CAM, "Camera ID : %u", msg_handle->camera_id);
  OPEL_LOG_DBG(CAM, "Sensor Name : %s", msg_handle->sensor_name);

  CameraRequest *request = new CameraRequest(kSensorOverlayStart);
  request->setMsgHandle(msg_handle);

  CameraDevice *device = controller->getDeviceById(msg_handle->camera_id);
  request->setCameraDevice(device);
  ret = device->processRequest(request);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "Sensor overlay start failed");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult CameraCommunicator::sensorOverlayStop(DBusMessage *msg,
    CameraController *controller, CameraConfigParser *config_parser)
{
  OPEL_LOG_DBG(CAM, "Get sensor stop request");
  bool ret;

  dbusSensorRequest *msg_handle = (dbusSensorRequest*)malloc(sizeof(dbusSensorRequest));

  dbus_message_get_args(msg, NULL,
      DBUS_TYPE_UINT64, &(msg_handle->pid),
      DBUS_TYPE_UINT64, &(msg_handle->camera_id),
      DBUS_TYPE_STRING, &(msg_handle->sensor_name),
      DBUS_TYPE_INVALID);

  OPEL_LOG_DBG(CAM, "PID : %u", msg_handle->pid);
  OPEL_LOG_DBG(CAM, "Camera ID : %u", msg_handle->camera_id);
  OPEL_LOG_DBG(CAM, "Sensor Name : %s", msg_handle->sensor_name);

  CameraRequest *request = new CameraRequest(kSensorOverlayStop);
  request->setMsgHandle(msg_handle);

  CameraDevice *device = controller->getDeviceById(msg_handle->camera_id);
  request->setCameraDevice(device);
  ret = device->processRequest(request);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "Sensor overlay start failed");
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }

  return DBUS_HANDLER_RESULT_HANDLED;
}
