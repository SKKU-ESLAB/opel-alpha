#include "CameraConfigParser.h"

CameraConfigParser::CameraConfigParser()
{
}

CameraConfigParser::~CameraConfigParser()
{
  if (this->mCameraConfigCjson)
    cJSON_Delete(this->mCameraConfigCjson);
}

bool CameraConfigParser::readCameraConfig(char *filepath)
{
  FILE *fp;
  char *buffer;
  int num_bytes;

  fp = fopen(filepath, "r");
  if (fp == NULL) {
    OPEL_LOG_ERR(CAM, "%s is not exist", filepath);
    return false;
  }

  fseek(fp, 0L, SEEK_END);
  num_bytes = ftell(fp);
  fseek(fp, 0L, SEEK_SET);

  buffer = (char *) calloc(num_bytes, sizeof(char));
  if (buffer == NULL) {
    OPEL_LOG_ERR(CAM, "Failed to allocate buffer");
    return false;
  }

  fread(buffer, sizeof(char), num_bytes, fp);
  this->mCameraConfigCjson = cJSON_Parse(buffer);
  if (this->mCameraConfigCjson == NULL) {
    OPEL_LOG_ERR(CAM, "Failed to parse camera configuration file");
    return false;
  }

  free(buffer);
  fclose(fp);
  return true;
}

int CameraConfigParser::readyCameraConfig()
{
  cJSON *root = this->mCameraConfigCjson;
  int camera_num = cJSON_GetObjectItem(root, "camera_num")->valueint;
  this->mRecordingConfig = cJSON_GetObjectItem(root, "recording")->valuestring;
  this->mSnapshotConfig = cJSON_GetObjectItem(root, "snapshot")->valuestring;
  this->mStreamingConfig = cJSON_GetObjectItem(root, "streaming")->valuestring;
  this->mPreRecordingInitConfig = cJSON_GetObjectItem(root, "pre_recording_init")->valuestring;
  this->mPreRecordingConfig = cJSON_GetObjectItem(root, "pre_recording")->valuestring;
  this->mOpenCVConfig = cJSON_GetObjectItem(root, "opencv")->valuestring;

  OPEL_LOG_DBG(CAM, "Number of camera = %d", camera_num);
  OPEL_LOG_DBG(CAM, "Recording = %s", this->mRecordingConfig);
  OPEL_LOG_DBG(CAM, "Snapshot = %s", this->mSnapshotConfig);
  OPEL_LOG_DBG(CAM, "Streaming = %s", this->mStreamingConfig);
  OPEL_LOG_DBG(CAM, "Pre-Recording Init = %s", this->mPreRecordingInitConfig);
  OPEL_LOG_DBG(CAM, "Pre-Recording = %s", this->mPreRecordingConfig);
  OPEL_LOG_DBG(CAM, "OpenCV = %s", this->mOpenCVConfig);

  return camera_num;
}

GstElement* CameraConfigParser::getMainBinById(int camera_id)
{
  cJSON *main = cJSON_GetObjectItem(this->mCameraConfigCjson, "main");
  char s_camera_id[32];
  sprintf(s_camera_id, "%d", camera_id);

  char *main_bin = cJSON_GetObjectItem(main, s_camera_id)->valuestring;
  OPEL_LOG_DBG(CAM, "main_%d = %s", camera_id, main_bin);
  return gst_parse_bin_from_description(main_bin, TRUE, NULL);
}

GstElement* CameraConfigParser::getRecordingBin()
{
  return gst_parse_bin_from_description(this->mRecordingConfig, TRUE, NULL);
}

GstElement* CameraConfigParser::getSnapshotBin()
{
  return gst_parse_bin_from_description(this->mSnapshotConfig, TRUE, NULL);
}

GstElement* CameraConfigParser::getStreamingBin()
{
  return gst_parse_bin_from_description(this->mStreamingConfig, TRUE, NULL);
}

GstElement* CameraConfigParser::getPreRecordingInitBin()
{
  return gst_parse_bin_from_description(this->mPreRecordingInitConfig, TRUE, NULL);
}

GstElement* CameraConfigParser::getPreRecordingBin()
{
  return gst_parse_bin_from_description(this->mPreRecordingConfig, TRUE, NULL);
}

GstElement* CameraConfigParser::getOpenCVBin()
{
  return gst_parse_bin_from_description(this->mOpenCVConfig, TRUE, NULL);
}
