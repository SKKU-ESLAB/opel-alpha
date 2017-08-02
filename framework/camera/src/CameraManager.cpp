#include "CameraManager.h"

CameraManager *cam = NULL;

CameraManager::CameraManager()
{
  this->mController = new CameraController();
  this->mCommunicator = new CameraCommunicator();
  this->mConfigParser = new CameraConfigParser();

  this->mTotalCameraNum = 0;
}

CameraManager::~CameraManager()
{
  if (this->mController != NULL)
    delete this->mController;
  if (this->mCommunicator != NULL)
    delete this->mCommunicator;
  if (this->mConfigParser != NULL)
    delete this->mConfigParser;
  if (this->mGMainLoop != NULL)
    g_main_loop_unref(this->mGMainLoop);
}

bool CameraManager::start(int argc, char **argv)
{
  bool ret;
  
  // Read the configuration file
  char* opel_config_dir = getenv("OPEL_CONFIG_DIR");
  if (!opel_config_dir) {
    OPEL_LOG_ERR(CAM, "Cannot read OPEL_CONFIG_DIR");
    return false;
  }
  char str[512] = "";
  strncpy(str, opel_config_dir, strlen(opel_config_dir));
  strncat(str, "/camera-config.json", strlen("/camera-config.json"));
  OPEL_LOG_DBG(CAM, "Camera Config file path : %s", str);

  ret = this->mConfigParser->readCameraConfig(str);
  if (ret == false) {
    OPEL_LOG_ERR(CAM, "Failed to read camera configuration file");
    return false;
  }

  // GStreamer Initialization
  gst_init(&argc, &argv);

  // Make Glib Main Loop
  this->mGMainLoop = g_main_loop_new(NULL, false);

  // Set the configuration for each camera device
  int total_camera_num = this->mConfigParser->readyCameraConfig();
  if (total_camera_num < 1) {
    OPEL_LOG_ERR(CAM, "Camera Manager requires at least one camera");
    return false;
  }

  this->setTotalCameraNum(total_camera_num);
  for (int camera_id = 0; camera_id < total_camera_num; camera_id++) {
    OPEL_LOG_DBG(CAM, "Camera device initialization (%d)", camera_id);
    GstElement *main_bin = this->mConfigParser->getMainBinById(camera_id);
    CameraDevice *device = this->mController->createDevice(camera_id, main_bin,
        this->mGMainLoop);
  }
 
  // D-Bus Registration
  this->mCommunicator->initCommunication(this, msg_dbus_filter);

  // Glib Main Loop Run
  g_main_loop_run(this->mGMainLoop);

  return true;
}

CameraController* CameraManager::getController(void)
{
  if (this->mController == NULL)
    this->mController = new CameraController();

  return this->mController;
}

CameraCommunicator* CameraManager::getCommunicator(void)
{
  if (this->mCommunicator == NULL)
    this->mCommunicator = new CameraCommunicator();

  return this->mCommunicator;
}

CameraConfigParser* CameraManager::getConfigParser(void)
{
  if (this->mConfigParser == NULL)
    this->mConfigParser = new CameraConfigParser();

  return this->mConfigParser;
}

void CameraManager::setTotalCameraNum(int total_camera_num)
{
  this->mTotalCameraNum = total_camera_num;
}

int CameraManager::getTotalCameraNum(void)
{
  return this->mTotalCameraNum;
}

void signalHandler(int signo)
{
  OPEL_LOG_WARN(CAM, "Signal handler invoked");
  assert(cam != NULL);

  CameraController *controller = cam->getController();
  int total_camera_num = cam->getTotalCameraNum();
  bool ret;

  for (int camera_id = 0; camera_id < total_camera_num; camera_id++) {
    CameraDevice *device = controller->getDeviceById(camera_id);
    GstElement *pipeline = device->getPipeline();
    ret = gst_element_set_state(pipeline, GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE)
      OPEL_LOG_ERR(CAM, "Unable to set the pipeline to the NULL state.");
  }

  GMainLoop *loop = cam->getGMainLoop();
  if (loop && g_main_loop_is_running(loop))
    g_main_loop_quit(loop);
}

int main(int argc, char *argv[])
{
  bool ret;

  OPEL_LOG_DBG(CAM, "OPEL Camera Manager Start!");

  signal(SIGINT, signalHandler);
  
  cam = new CameraManager();
  ret = cam->start(argc, argv);
  if (!ret) {
    OPEL_LOG_ERR(CAM, "OPEL Camera Manager Start Fail");
    return -1;
  }

  if (cam != NULL)
    delete cam;

  return 0;
}


DBusHandlerResult msg_dbus_filter(DBusConnection *connection,
    DBusMessage *msg, void *camera_manager)
{
  OPEL_LOG_DBG(CAM, "D-Bus message received");

  CameraManager *cam = (CameraManager *)camera_manager;
  CameraCommunicator *communicator = cam->getCommunicator();
  CameraController *controller = cam->getController();
  CameraConfigParser *config_parser = cam->getConfigParser();

  if (dbus_message_is_signal(msg, "org.opel.camera.daemon","recStart"))
    return communicator->recordingStart(msg, controller, config_parser);
  else if (dbus_message_is_signal(msg, "org.opel.camera.daemon", "recStop"))
    return communicator->recordingStop(msg, controller, config_parser);
  else if (dbus_message_is_signal(msg, "org.opel.camera.daemon", "jpegStart"))
    return communicator->snapshotStart(msg, controller, config_parser);
  else if (dbus_message_is_signal(msg, "org.opel.camera.daemon", "streamingStart"))
    return communicator->streamingStart(msg, controller, config_parser);
  else if (dbus_message_is_signal(msg, "org.opel.camera.daemon", "streamingStop"))
    return communicator->streamingStop(msg, controller, config_parser);
  else if (dbus_message_is_signal(msg, "org.opel.camera.daemon", "preRecordingInit"))
    return communicator->preRecordingInit(msg, controller, config_parser);
  else if (dbus_message_is_signal(msg, "org.opel.camera.daemon", "preRecordingStart"))
    return communicator->preRecordingStart(msg, controller, config_parser);
  else if (dbus_message_is_signal(msg, "org.opel.camera.daemon", "preRecordingStop"))
    return communicator->preRecordingStop(msg, controller, config_parser);
  else if (dbus_message_is_signal(msg, "org.opel.camera.daemon", "openCVStart"))
    return communicator->openCVStart(msg, controller, config_parser);
  else if (dbus_message_is_signal(msg, "org.opel.camera.daemon", "openCVStop"))
    return communicator->openCVStop(msg, controller, config_parser);
  else if (dbus_message_is_signal(msg, "org.opel.camera.daemon", "sensorOverlayStart"))
    return communicator->sensorOverlayStart(msg, controller, config_parser);
  else if (dbus_message_is_signal(msg, "org.opel.camera.daemon", "sensorOverlayStop"))
    return communicator->sensorOverlayStop(msg, controller, config_parser);

  OPEL_LOG_WARN(CAM, "Receive unidentified D-bus message");
  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
