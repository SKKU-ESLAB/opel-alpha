#ifndef _CAM_CORE_H_
#define _CAM_CORE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>

#include <inttypes.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <semaphore.h>

#include <asm/types.h>

#include <linux/videodev2.h>

//#include <libv4l1.h>
//#include <libv4l2.h>


#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
/*User-Defined Header*/
#include "cam_property.h"
#include "cam_status.h"


#define SHM_KEY 1234

#define V4L2_DEFAULT_WIDTH 640
#define V4L2_DEFAULT_HEIGHT 480


#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define MAX_V4L2_BUFFERS 10

#define OptLast 256

#define SEM_FOR_PAYLOAD_SIZE 9948

static int xioctl(int fh, int request, void* arg);
static void errno_exit(const char *s);
static void streaming_set_cap(int fd);


const static char* deviceName = "/dev/video0";



typedef struct buffer{
	void *start;
	/* MINI */
	int* length;
}buffer;

struct ctl_parameters{
	bool terminate_decoding;
	char options[OptLast];
	char fd_name[80];
	bool filename_active;
	double freq;
	uint32_t wait_limit;
	uint8_t tuner_index;
//	struct v4l2_hw_freq_seek freq_seek;
};

static struct ctl_parameters params;


static bool libv4l2_open(CameraProperty* camProp);
static bool libv4l2_init(CameraProperty* camProp);
static bool init_SharedMemorySpace(int req_count, int buffer_size, int shmid, void** shmptr, key_t shmkey);
static bool uinit_SharedMemorySpace(int shmid, void** shmPtr);
static bool recMainLoop(CameraProperty* camProp, buffer* buffers);
static bool readFrame(CameraProperty* camProp, buffer* buffers, unsigned& cnt, unsigned &last, struct timeval &tv_last, int semid, void* shmPtr);
static void processImg(const void* p , int size);
static int do_handle_cap(CameraProperty* camProp, buffer* buffers);
//static bool init_Semaphore(key_t semKey, int& semid);
//static bool uinit_Semaphore(int& semid);
class OPELCamera
{
				public:
								void setCameraProperty(CameraProperty* camProp);
								CameraProperty* getCameraProperty() const;
								OPELCamera();
								OPELCamera(CameraProperty* camProp);
								bool open();
			          virtual bool init_device() = 0;
								virtual bool start() = 0;
								virtual bool stop() = 0;
								virtual bool close_device() = 0;
								void deleteCameraProperty();		
								bool init_Semaphore();
								bool uinit_Semaphore();
				protected:
								virtual bool init_userPointer(unsigned int) = 0;
				//		    sem_t* mutex;	
								CameraProperty* camProp;			
																		
};

class Record : public OPELCamera
{
				public:
					virtual bool init_device();
					virtual bool start();
					virtual bool stop();
					virtual bool close_device();

				private:
				   buffer* buffers;
					 	
					 virtual bool init_userPointer(unsigned int);
					 char* output_path;
};
class OpenCVSupport : public OPELCamera
{
	public:
		virtual	bool init_device();
	 	virtual bool start();
		virtual bool stop();
		virtual bool close_device();
		bool getEos(void); 
		void setEos(bool eos);
	  void setThrMutex(pthread_mutex_t& mutex){ this->mutex = mutex; }
		//virtual bool init_userPointer(unsigned int);
	
		bool init_Semaphore();
		bool uinit_Semaphore();
	private:
	 	  bool mainLoop(CameraProperty* camProp, buffer* buffers);
		  virtual bool init_userPointer(unsigned int);
			buffer* buffers;
			int semid;
			int shmid;
			void* shmPtr;
			struct shmid_ds shm_info;
			bool eos;
			pthread_mutex_t mutex;
			sem_t* mx;
			sem_t* semForSelect;
			int fifo_fd;
};








#endif /* CAM_CORE_H_ */
