#ifndef _CAM_PROPERTY_H_
#define _CAM_PROPERTY_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/sem.h>
//#include <semaphore.h>

#include <iostream>
#include <linux/videodev2.h>
//#include <libv4l1.h>
//#include <libv4l2.h>
//#include "cam_core.h"
#include "cam_status.h"
#define DEFAULT_COUNT 100000000

#define OPENCV_DEFAULT_PIXFORMAT V4L2_PIX_FMT_RGB24 //RGB24
#define DEFAULT_WIDTH 640 
#define DEFAULT_HEIGHT 480
#define OPENCV_SHM_KEY 5315 

#define OPENCV_SHM_KEY_FOR_PROPERTY 4941
#define OPENCV_SEMAPHORE_FOR_PROPERTY 49411

#define REC_DEFAULT_WIDTH 1280
#define REC_DEFAULT_HEIGHT 720
#define REC_DEFAULT_PIXFORMAT V4L2_PIX_FMT_H264 // h.264
#define REC_SHM_KEY_SIZE 9448
#define REC_SHM_KEY 9447
#define REC_SHM_KEY_FOR_PROPERTY 9988


//char SEM_NAME[] = "openCVProperty";

union semun
{
	int val;
	struct semid_ds* buf;
	unsigned short int *array;
};
static struct sembuf mutex_post = {0, -1, SEM_UNDO};
static struct sembuf mutex_wait = {0, 1, SEM_UNDO};

typedef struct property
{
	int width;
	int height;
	int buffer_size; 
	int buffer_num; 
	int n_buffer;		
	bool isPropertyChanged;
	bool allowRunning;

//	CameraStatus* camStatus;	

}property;


class CameraProperty
{
				public: 
//							static CameraProperty* getInstance();
								
							
								~CameraProperty();
								/*Default Constructor For OPNECV*/
								CameraProperty();
								CameraProperty(bool);
								key_t getShmKey(void){return this->shmkey;}
								void setfd(int fd) { this->fd = fd; }		
							  int getfd(void) { return this->fd; } 
								struct v4l2_requestbuffers* getRequestbuffers(void) { return this->req; } 	
								void setN_buffers(unsigned int n_buffer) {this->n_buffer = n_buffer; }
						    unsigned int getN_buffers(void) {return this->n_buffer; }	
								struct v4l2_capability* getCapability(void) { return this->cap; }
								struct v4l2_cropcap* getCropcap(void) { return this->cropcap; } 
								struct v4l2_crop* getCrop(void) { return this->crop; }
								struct v4l2_format* getFormat(void) {return this->fmt; }
								struct v4l2_buffer* getBuffer(void) {return this->buf; }
								enum v4l2_buf_type getType(void) {return this->type;}
								int getWidth() { return this->width; }
								int getHeight() { return this->height; }
								unsigned int getPixelformat() { return this->pixelformat; }
								enum v4l2_field getField() { return this->field; }
							  unsigned int* getCount() { return this->count; }
								void setCount(unsigned int count) { *(this->count) = count; }
								void printSetValue(void);
							  void setBufferSize(int buffer_size);
								void setBufferNum(int buffer_num);
								int getBufferSize(void) { return this->buffer_size; }
								int getBufferNum(void) { return this->buffer_num; } 
							  void initProperty(property* prop);
								bool InitSharedPropertyToTarget(property* prop);
								bool uInitSharedPropertyToTarget(property* prop);
								bool initSemaphore(void);
								//	struct stat getStat() { return this->st; }
							  //void setStat(struct stat st) { this->st = st; }
								void setProperty(property* prop){ this->prop = prop; } 	

				private:
								char* processName;
								int fd;
							 //static CameraProperty* camProp;
								//CameraProperty();
								int deviceHandle;
								int bufferIndex;
								int width, height;
								unsigned int pixelformat;
								key_t shmkey;
								key_t shmkey_for_property;
								int shmid_for_property;
								void* shmPtr_for_property;

								enum v4l2_field field;
								int mode;
								unsigned int* count;
								unsigned int n_buffer;
								int buffer_size;
								int buffer_num;

								struct v4l2_capability* cap;
								struct v4l2_input* inp;
								struct v4l2_format* fmt;
								struct v4l2_crop* crop;
								struct v4l2_cropcap* cropcap;
								struct v4l2_requestbuffers* req;
								struct v4l2_jpegcompression* compr;
								struct v4l2_control* control;
								struct v4l2_buffer* buf;
								enum v4l2_buf_type type;
								struct v4l2_queryctrl* queryctrl;
								struct timeval* timestamp;
								property* prop;
								//	sem_t* mutex;
								//			  			struct stat* st;
								int semid;
};


#endif /*_CAM_PROPERTY_H_*/
