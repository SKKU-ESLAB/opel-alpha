#ifndef OPEL_CONFIG_H
#define OPEL_CONFIG_H

#define OPEL_NUM_DEFAULT_RECORDING_ELE 4
#define OPEL_NUM_DEFAULT_SNAPSHOT_ELE 3
#define OPEL_NUM_DEFAULT_RAW_ELE 3
#define OPEL_NUM_DEFAULT_STREAMING_ELE 7

#define STREAMING_720P_WIDTH 320
#define STREAMING_720P_HEIGHT 240

#define SHM_KEY_FOR_BUFFER 5315

#define RAW_DEFAULT_BUF_SIZE RAW_BGRX_480P_BUF_SIZE * 2
//#define RAW_DEFAULT_BUF_SIZE 4194304 

#define RAW_FORMAT_WIDTH RAW_480P_WIDTH
#define RAW_FORMAT_HEIGHT RAW_480P_HEIGHT

#define RAW_480P_WIDTH  640
#define RAW_480P_HEIGHT  480
#define RAW_BGRX_480P_BUF_SIZE 1228800

#define RAW_720P_WIDTH  1280
#define RAW_720P_HEIGHT  720
#define RAW_BGRX_720P_BUF_SIZE 3686400

#define RAW_1080P_WIDTH  1920
#define RAW_1080P_HEIGHT  1080
#define RAW_BGRX_1080P_BUF_SIZE 8294400

#define TARGET_SRC_IS_CAM 1

#define OPEL_LOG_VERBOSE 0

#endif /*OPEL_CONFIG_H*/
