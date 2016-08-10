#include "abtMisc.h"


int abtFileListInitialize(abtFileList *pFileList) {

	pFileList->pFileInfo = NULL;
	pFileList->max_file_number = 0;
	pFileList->current_file_number = 0;

}


int abtFileListAdd(abtFileList *pFileList) {

	if (pFileList->max_file_number == 0) {

		uint32_t i = 0;

		pFileList->pFileInfo =
		(abtFileInfo *) calloc(pFileList->max_file_number + ABT_FILE_NUMBER_INCREMENT, sizeof(abtFileInfo));
		pFileList->max_file_number += ABT_FILE_NUMBER_INCREMENT;

		for (i = 0; i < pFileList->max_file_number; i++) {
			_abtFileInfoInitialize(&(pFileList->pFileInfo[i]));
		}

		pFileList->current_file_number = 0;

	}

	else if (pFileList->current_file_number >= pFileList->max_file_number) {

		abtFileInfo *tmp = pFileList->pFileInfo;
		uint32_t i = 0;

		pFileList->pFileInfo =
		(abtFileInfo *) calloc(pFileList->max_file_number + ABT_FILE_NUMBER_INCREMENT, sizeof(abtFileInfo));
		pFileList->max_file_number += ABT_FILE_NUMBER_INCREMENT;

		pFileList->current_file_number += 1;

		for (i = 0; i < pFileList->max_file_number; i++) {
			_abtFileInfoInitialize(&(pFileList->pFileInfo[i]));
		}

		for (i = 0; i < (pFileList->max_file_number - ABT_FILE_NUMBER_INCREMENT); i++) {
			_abtFileNameCopy((tmp[i]).file_name, pFileList->pFileInfo[i].file_name);
		}

		free(tmp);

	}

	else {

		pFileList->current_file_number += 1;

	}

	_abtFileInfoAdd( &(pFileList->pFileInfo[pFileList->current_file_number]) );

	return 0;
}


int _abtFileInfoInitialize(abtFileInfo *pFileInfo) {

	pFileInfo->file_name = (char *) calloc(ABT_FILE_NAME_LENGTH, sizeof(char));

}


int _abtFileInfoAdd(abtFileInfo *pFileInfo) {

	time_t current_time;
	struct tm *current_tm;

	current_time = time(NULL);
	current_tm = localtime(&current_time);

	sprintf(pFileInfo->file_name, "video_%04d%02d%02d_%02d%02d%02d", 
			current_tm->tm_year + 1900,
			current_tm->tm_mon + 1,
			current_tm->tm_mday,
			current_tm->tm_hour,
			current_tm->tm_min,
			current_tm->tm_sec
			);

	return 0;

}


int _abtFileNameCopy(char *src, char *dst) {

	uint32_t i = 0;

	for (i = 0; i < ABT_FILE_NAME_LENGTH; i++) {
		dst[i] = src[i];
	}

	return 0;
}


int abtMediaInfoInitialize(abtMediaInfo *pMedInfo, abtFileList *pFileList) {

	pMedInfo->width = ABT_WIDTH;
	pMedInfo->height = ABT_HEIGHT;
	pMedInfo->channel = ABT_CHANNEL;
	pMedInfo->fps = ABT_FPS;
	pMedInfo->time_length = ABT_TIME_LENGTH;

	if (pMedInfo->data == NULL) {	
		pMedInfo->data = (uint8_t *) calloc(sizeof(uint8_t), ABT_WIDTH*ABT_HEIGHT*ABT_CHANNEL);
	}

	pMedInfo->dev_path = (char *) ABT_DEV_PATH;
	pMedInfo->file_path = (char *) ABT_FILE_PATH;
	pMedInfo->file_name = pFileList->pFileInfo[pFileList->current_file_number].file_name;
	pMedInfo->file_ext = (char *) ABT_FILE_EXTENSION;

	return 0;
}


int abtServerInfoInitialize(abtServerInfo *pServInfo, char *addr, char *port) {

	pServInfo->clnt_sid = socket(PF_INET, SOCK_STREAM, 0);

	if (pServInfo->clnt_sid == -1) {
		printf("socket error\n");
		return (int) NULL;
	}

	memset(&(pServInfo->serv_addr), 0, sizeof(pServInfo->serv_addr));
	pServInfo->serv_addr.sin_family = AF_INET;
	pServInfo->serv_addr.sin_addr.s_addr = inet_addr(addr);
	pServInfo->serv_addr.sin_port = htons(atoi(port));

	return 0;

}


// Warning: following func is asynchronous
int abtConnectToServer(abtServerInfo *pServInfo) {

	pthread_t tid = 0;

	pthread_create( &tid,
			NULL,
			( void * (*)(void *) ) _abtConnectToServer,
			(void *) pServInfo );

	return 0;

}


int _abtConnectToServer(abtServerInfo *pServInfo) {

	while (1) {

		if ( connect(pServInfo->clnt_sid, (struct sockaddr *) &(pServInfo->serv_addr), sizeof(pServInfo->serv_addr)) == -1 ) {
			printf("connect() error\n");
			sleep(10);
			//return NULL;
		}
		else { printf("connect success"); break; }

	}

	return 1;
}


int abtOpenVideo(abtMediaInfo *pMedInfo) {

	struct v4l2_capability caps = {0};
	struct v4l2_format fmt = {0};
	struct v4l2_requestbuffers req = {0};
	struct v4l2_buffer buf = {0};

	pMedInfo->fd = open(pMedInfo->dev_path, O_RDWR);

	if (pMedInfo->fd == -1)
	{
		perror("Opening Video Devide");
		return 1;
	}

	// Query the capture

	if ( xioctl(pMedInfo->fd, VIDIOC_QUERYCAP, &caps) == -1 ) {
		perror("Querying Capabilites");
		return 1;
	}

	// Image Format
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = pMedInfo->width;
	fmt.fmt.pix.height = pMedInfo->height;
	//fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	//printf("format: %d\n", fmt.fmt.pix.pixelformat);
	//printf("id: %d\n", V4L2_PIX_FMT_RGB24);
	//getchar();
	fmt.fmt.pix.field = V4L2_FIELD_NONE;

	if ( xioctl(pMedInfo->fd, VIDIOC_S_FMT, &fmt) == -1 ) {
		perror("Setting Pixel Format");
		return 1;
	}

	//printf("format: %x\n", fmt.fmt.pix.pixelformat);
	//getchar();
	// init mmap

	// Request Buffers
	req.count = 1;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	//ioctl(fd, VIDIOC_REQBUFS, &req);

	if ( xioctl(pMedInfo->fd, VIDIOC_REQBUFS, &req) == -1 ) {
		perror("Requesting Buffer");
		return 1;
	}

	// Query Buffer
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;

	if ( xioctl(pMedInfo->fd, VIDIOC_QUERYBUF, &buf) == -1 ) {
		perror("Querying Buffer");
		return 1;
	}

	pMedInfo->data = (uint8_t *) mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, pMedInfo->fd, buf.m.offset);

	return (int) NULL;
}


int abtQueryFrame(abtMediaInfo *pMedInfo) {
	static int cam_flag = 0;
	fd_set fds;

	struct v4l2_buffer buf = {0};
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;

	if ( xioctl(pMedInfo->fd, VIDIOC_QBUF, &buf) == -1 ) {
		perror("Quering Buffer");
		return 1;
	}

	// Capture Image
	if (cam_flag == 0) {
		if ( xioctl(pMedInfo->fd, VIDIOC_STREAMON, &buf.type) == -1)
		{
			perror("Error in Start Capture\n");
			return 0;
		}

		FD_ZERO(&fds);
		FD_SET(pMedInfo->fd, &fds);

		struct timeval tv = {0};

		tv.tv_sec = 2;
		int r = select((pMedInfo->fd) + 1, &fds, NULL, NULL, &tv);

		cam_flag = 1;


		if (r == -1)
		{
			// Waiting for frame
			return 0;
		}
	}

	// Image stored into buffer

	if ( xioctl(pMedInfo->fd, VIDIOC_DQBUF, &buf) == -1 ) {
		perror("Retrieving Frame");
		return 1;
	}

	//printf("Saving Image");

	// (void *) buffer;
	//
	return 0;

}


static int xioctl(int fh, int request, void *arg)
{
	int r = 0;

	do {
		r = ioctl(fh, request, arg);
	} while ( (r == -1) && (errno == EINTR) ) ;

	return r;
}
