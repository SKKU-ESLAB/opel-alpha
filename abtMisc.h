#ifndef _ABT_MISC_H
#define _ABT_MISC_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <arpa/inet.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <linux/videodev2.h>

#include <pthread.h>

//#define ABT_WIDTH 1920
//#define ABT_HEIGHT 1080
#define ABT_WIDTH 640
#define ABT_HEIGHT 480
#define ABT_CHANNEL 2
#define ABT_FPS 20
#define ABT_TIME_LENGTH 20
#define ABT_DEV_PATH "/dev/video0"
#define ABT_FILE_PATH "/home/ubuntu/video/"
#define ABT_FILE_EXTENSION ".mp4"

#define ABT_FILE_PATH_LENGTH 128
#define ABT_FILE_NAME_LENGTH 64
#define ABT_FILE_NUMBER_INCREMENT 100


struct abtFileInfo {

	char *file_name;

};


struct abtFileList {

	abtFileInfo *pFileInfo;
	uint32_t max_file_number;
	uint32_t current_file_number;

};


struct abtMediaInfo {

	uint32_t width;
	uint32_t height;
	uint32_t channel;
	uint32_t fps;
	uint32_t time_length;

	uint8_t *data;

	char *dev_path;
	char *file_path;
	char *file_name;
	char *file_ext;

	int fd;

};


struct abtServerInfo {

	int clnt_sid;
	struct sockaddr_in serv_addr;	

};

int abtFileListInitialize(abtFileList *);
int abtFileListAdd(abtFileList *);

int _abtFileInfoInitialize(abtFileInfo *);
int _abtFileInfoAdd(abtFileInfo *);
int _abtFileNameCopy(char *, char *);

int abtMediaInfoInitialize(abtMediaInfo *, abtFileList *);

int abtServerInfoInitialize(abtServerInfo *, char *, char *);

int abtConnectToServer(abtServerInfo *);
int _abtConnectToServer(abtServerInfo *);

int abtOpenVideo(abtMediaInfo *);
int abtQueryFrame(abtMediaInfo *);

static int xioctl(int, int, void *);

#endif
