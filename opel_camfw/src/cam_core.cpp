#include "cam_core.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
static bool init_SharedMemorySpace(int req_count, int buffer_size, int shmid, void** shmptr);
static bool uinit_SharedMemorySpace(int shmid);
char SEM_NAME[] = "vik";
/*MINI ADD*/
char SEM_FOR_SELECT[]= "vvik";
static int openFifo(const char* path)
{	
	int fifo_fd;
	if(mkfifo(path, 0666) == -1)
	{
		fprintf(stderr, "MKFIFO FAILED\n");
		return -1;
	}
	if((fifo_fd = open(path, O_RDWR))<0){	
		fprintf(stderr, "GET FIFO FD FAILED\n");
		return -1;
	}	
	return fifo_fd;
}
static bool checkFPS(unsigned& cnt, unsigned& last, struct timeval &tv_last, FILE* profile_fp) 
{
	struct timeval cur;
	gettimeofday(&cur, NULL);
	fprintf(profile_fp, "%dSec %dUsec\n", cur.tv_sec ,cur.tv_usec);
	if(cnt == 0)
	{
		gettimeofday(&tv_last, NULL);
	}else
	{
		struct timeval tv_cur, res;
		gettimeofday(&tv_cur, NULL);
		timersub(&tv_cur, &tv_last, &res);
		if(res.tv_sec){
			unsigned fps =(cnt - last);
			last = cnt;
			tv_last = tv_cur;
			//fprintf(stderr, "Test : %d fps\n", fps);
		}
	}
	cnt++;
	return true;
}
bool OpenCVSupport::init_Semaphore()
{
//	fifo_fd = openFifo("/tmp/fifo");
	printf("hihihihihihihihihi");
	mx = sem_open(SEM_NAME, O_CREAT, 0666, 1);
	if(mx == SEM_FAILED)
	{
		fprintf(stderr, "[INIT_SEMAPHORE] : FAILED\n");
		sem_unlink(SEM_NAME);
		return false;
	}
	/*MINI ADD*/
	semForSelect = sem_open(SEM_FOR_SELECT, O_CREAT, 0666, 1);
	if(semForSelect == SEM_FAILED)
	{
		fprintf(stderr, "[INIT_SEMAPHORE] : FAILED\n");
		sem_unlink(SEM_FOR_SELECT);
		return false;
	}
	return true;
}

bool OpenCVSupport::uinit_Semaphore()
{
	sem_close(mx);
	sem_unlink(SEM_NAME);
	/*MINI ADD*/
	sem_close(semForSelect);
	sem_unlink(SEM_FOR_SELECT);
	return true;
}


OPELCamera::OPELCamera()
{
				//		this->camProp = CameraProperty::getInstance();
				//	unsigned int* cnt = camProp->getCount();
				//	printf("count : %d\n ", *cnt);
}

void OPELCamera::setCameraProperty(CameraProperty* camProp)
{
				this->camProp = camProp;
}
void OPELCamera::deleteCameraProperty()
{
				delete this->camProp;
}
				
CameraProperty* OPELCamera::getCameraProperty() const
{
				return this->camProp;
}
bool OPELCamera::open()
{
				return libv4l2_open(this->camProp) ? true : false;
}
bool OpenCVSupport::getEos(void)
{
	return this->eos;
}
void OpenCVSupport::setEos(bool eos)
{
	pthread_mutex_lock(&mutex);
	this->eos = eos;
	pthread_mutex_unlock(&mutex);	
}
bool OpenCVSupport::stop()
{
	int size = camProp->getBufferSize();
	int fd = this->camProp->getfd();
	enum v4l2_buf_type  type;	
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
	{
		fprintf(stderr, "VIDIOC_STREAMOFF");
		return false;
	}
	//MINI
/*	for(int i=0; i<4; i++)
	{
		memset(buffers[i].start, 0x0, size);
		memset(buffers[i].length, 0x0, sizeof(int));
	}*/
	return true;
}
bool OpenCVSupport::close_device()
{
				unsigned int i;
				int fd = camProp->getfd();
				if(!uinit_SharedMemorySpace(this->shmid, &shmPtr))
								return false;
				if(!uinit_Semaphore())
								return false;
				//MINI	
				if(-1 == close(fd))
				{
						fprintf(stderr, "[OpenCVSupport::close_device] : close(fd) failed\n");
						return false;
				}
				//MINI
				for(int j=0; j<4; j++)
				{
					free(buffers[j].start);
					free(buffers[j].length);
				}
				free(this->buffers);
				return true;
}
static bool uinit_SharedMemorySpace(int shmid, void** shmPtr)
{
	//MINI
		/*		if(-1 == shmdt(*shmPtr))
				{
								fprintf(stderr, "Detach Shared Memory Space Failed\n");
								return false;
				}
				
				//MINI
				if(-1 == shmctl(shmid, IPC_RMID, 0))
				{
								fprintf(stderr, "UnInit Shared Memory Space Failed\n");
								return false;
				}*/
				return true;
}
bool OpenCVSupport::init_userPointer(unsigned int buffer_size)
{
				
				unsigned int i; 
				int fd = camProp->getfd();
				struct v4l2_requestbuffers* req = camProp->getRequestbuffers();
				CLEAR(*(req));
				req->count = 4;
				camProp->setN_buffers(req->count);
				req->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				req->memory = V4L2_MEMORY_USERPTR;
				
				if(-1 == xioctl(fd, VIDIOC_REQBUFS, req))
				{
								if(EINVAL == errno)
								{
												fprintf(stderr, "%s does not support " " user pointer i/o\n", deviceName);
												return false;
								}
								else
								{
											  fprintf(stderr, "VIDIOC_REQBUFS");
												return false;
								}
				}

				buffers = (buffer*)calloc(4, sizeof(*buffers)); 
				if(!buffers){
					fprintf(stderr, "Out of Memory\n");
					return false;
				}
				camProp->setBufferSize(buffer_size);
				camProp->setBufferNum(req->count);
				if(!init_SharedMemorySpace(req->count, buffer_size, shmid, &shmPtr, camProp->getShmKey()))
				{
								fprintf(stderr, "Shared Memory Space Initialization Failed\n");
								return false;
				}
				for(i=0; i< req->count; i++)
				{
						//		printf("buffer_size : %d\n", buffer_size);
						//		(*buffers[i].length) = buffer_size;
								/* MINI */
								buffers[i].start = (void*)malloc(buffer_size); //malloc data
									//shmPtr+i*buffer_size;
								if(!buffers[i].start)
								{
												fprintf(stderr, "Link to SharedMemorySpace failed\n");
												return false;
								}
				}
				int offset = req->count*buffer_size;
				for(i=0; i< req->count; i++)
				{
					buffers[i].length = (int*)malloc(sizeof(int));
						// (int*)(shmPtr+offset)+(i*sizeof(int));
					if(!buffers[i].length)
					{
						fprintf(stderr, "Link to SharedMemorySpace failed\n"); 
						return false;
					}
					*buffers[i].length = buffer_size;
				}
				if(!init_Semaphore())
					return false;

				return true;
}
static void processImg(void* p, int* size)
{

	unsigned sz; 
	FILE *fp = fopen("/home/pi/camData/cam.mjpg", "ab");
	sz = fwrite((char*)p, sizeof(char), *size, fp);
//	printf("sz : %d, length : %d", sz, *size);
	if(sz != *size)
	{
		fprintf(stderr, "failed!!!!!\n");
	}
	fflush(fp);
	fclose(fp);
}
static bool readFrame(CameraProperty* camProp, buffer* buffers, unsigned& cnt, unsigned &last, struct timeval &tv_last, int semid, void* shmPtr, sem_t* mx, sem_t* semForSelect, int fifo_fd)
{
		char ch = '<';
		struct v4l2_buffer* buf = camProp->getBuffer();
		int fd = camProp->getfd();
		int n_buffers = camProp->getN_buffers();
		int size = camProp->getBufferSize();
		int offset = size*(n_buffers-1);
		int offset_size = size*n_buffers;
		unsigned int i;
		unsigned int* count = camProp->getCount();
		void* ptr = shmPtr+offset;
		void* _check =(void*)shmPtr+offset_size+sizeof(int);
		void* _length = (void*)shmPtr+offset_size;
		int* length  = (int*)_length;
		int* check = (int*)_check;
		CLEAR(*buf);
		buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf->memory = V4L2_MEMORY_USERPTR;
		if(-1 == xioctl(fd, VIDIOC_DQBUF, buf))
		{
						switch(errno){
							case EAGAIN:
											return false;
											break;
							case EIO: 
											/*Ignore*/
							default: 
											fprintf(stderr, "VIDIOC_DQBUF\n");
											return false;
						}
		}
		for(i=0; i <n_buffers; ++i)
					if(buf->m.userptr == (unsigned long)buffers[i].start && buf->length >= (*buffers[i].length))
						break;
				assert(i < n_buffers); 
				sem_wait(mx);
					*check = 0;
				sem_post(mx);
		
				sem_wait(mx);
					*length = buf->bytesused;
					memcpy((char*)ptr, (char*)buf->m.userptr, *length);		
					*check = 1;
				sem_post(mx);
			//	write(fifo_fd, "DONE", 5);	
				//FIFO Message Send To Application
				usleep(100);
					if(-1 == xioctl(fd, VIDIOC_QBUF, buf))
					{
						fprintf(stderr, "VIDIOC_QBUF\n"); 
						return false;

					}
		fflush(stderr);

		if(cnt == 0)
		{
						gettimeofday(&tv_last, NULL);
		}else
		{
						struct timeval tv_cur, res;
						gettimeofday(&tv_cur, NULL);
						timersub(&tv_cur, &tv_last, &res);
						if(res.tv_sec){
								unsigned fps = (100*(cnt - last)) / (res.tv_sec * 100 + res.tv_usec / 10000);
								last = cnt; 
								tv_last = tv_cur;
								fprintf(stderr, " %d fps\n", fps);
						}
		}
		cnt++;

		return true;
}
bool OpenCVSupport::mainLoop(CameraProperty* camProp, buffer* buffers)
{
		FILE* profile_fp =  fopen("TIMESET.txt", "w");	
	//profile
		 unsigned cnt_fps=0;
		 struct timeval tv_last_fps;
		 unsigned last_fps=0;
		//
			unsigned int* count = camProp->getCount();
			unsigned int volatile_count;
			unsigned int cnt=0;
			
			pthread_mutex_lock(&mutex);
		  bool volatile_eos = this->eos;
			volatile_count = *count;
			pthread_mutex_unlock(&mutex);
			
			unsigned last = 0;
			struct timeval tv_last;
			int fd = camProp->getfd();
			
			int n_buffers = camProp->getN_buffers();
			int size = camProp->getBufferSize();
			int offset = size*(n_buffers-1);
			int offset_size = size*n_buffers;
			void* _check =(void*)shmPtr+offset_size+sizeof(int);
			int* check = (int*)_check;
			
			while((volatile_count > 0) && volatile_eos)
			{
							for(;;)
							{
											fd_set fds;
											struct timeval tv;
											int r;
											FD_ZERO(&fds);
											FD_SET(fd, &fds);
											tv.tv_sec = 2;
											tv.tv_usec = 0;
											r = select(fd+1, &fds, NULL, NULL, &tv);
											if(-1 == r){
															if(EINTR == errno)
																			continue;
															fprintf(stderr, "Select Error\n");
															return false;
											}
											if(0 == r){
															fprintf(stderr, "Select Timeout\n");
															return false;
											}
											if(FD_ISSET(fd, &fds))
											{
														/* Do Nathing */							
											}
											checkFPS(cnt_fps, last_fps, tv_last_fps, profile_fp);
											if(readFrame(camProp, buffers, cnt, last, tv_last, this->semid, this->shmPtr, this->mx, this->semForSelect, this->fifo_fd))
													break;	
							}

							pthread_mutex_lock(&mutex);
							volatile_eos = this->eos;
							if(*count > 0)
								volatile_count=(*count)--;
							else
								volatile_count = (*count);
					  	pthread_mutex_unlock(&mutex);
			
			}
			sem_wait(this->mx);
			*check = 0;
			sem_post(this->mx);
			fclose(profile_fp);	
			return true;
}
bool OpenCVSupport::start()
{
				unsigned int i;
				int fd = camProp->getfd();
				unsigned int n_buffer = camProp->getN_buffers();
				struct v4l2_buffer* buf = camProp->getBuffer();
				enum v4l2_buf_type type = camProp->getType();	
				for(i=0; i< n_buffer; i++)
				{
								CLEAR(*buf);
								buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
								buf->memory = V4L2_MEMORY_USERPTR;
								buf->index = i;
								buf->m.userptr = (unsigned long)buffers[i].start;
								buf->length = *buffers[i].length;
								sem_wait(this->mx);										
						*buffers[1].length = 0;
								sem_post(this->mx);
								if(-1 == xioctl(fd, VIDIOC_QBUF, buf))
								{
												fprintf(stderr, "VIDIOC_QBUF\n");
												return false;
								}
				}
				
				type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if(-1 == xioctl(fd, VIDIOC_STREAMON, &type))
				{
								fprintf(stderr, "VIDIOC_STREAMON\n");
								return false;
				}
			  if(!mainLoop(this->camProp, this->buffers))
				{
								fprintf(stderr, "Main_Loop\n");
								return false;
				}

				return true;
}
bool OpenCVSupport::init_device()
{
				unsigned int min;
				int fd = camProp->getfd();
				struct v4l2_capability* cap = camProp->getCapability();
				struct v4l2_cropcap* cropcap = camProp->getCropcap();
				struct v4l2_crop* crop = camProp->getCrop();
				struct v4l2_format* fmt = camProp->getFormat();
				if(-1 == xioctl(fd, VIDIOC_QUERYCAP, cap))
				{
								if(EINVAL == errno)
								{
												fprintf(stderr, "%s is no V4L2 device \n", deviceName);
												return false;
								}
								else
								{
												errno_exit("VIDIOC_QUERYCAP");
												return false;
								}
				}
				if(!(cap->capabilities & V4L2_CAP_VIDEO_CAPTURE))
				{
								fprintf(stderr, "%s is no video capture device\n", deviceName);
								return false;
				}
				if(!(cap->capabilities & V4L2_CAP_STREAMING))
				{
								fprintf(stderr, "%s is no video capture device\n", deviceName);
								return false;
				}
				CLEAR(*cropcap);
				cropcap->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if(0 == xioctl(fd, VIDIOC_CROPCAP, cropcap))
				{
								crop->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
								crop->c = cropcap->defrect;
								if(-1 == xioctl(fd, VIDIOC_S_CROP, crop))
								{
												switch(errno)
												{
																case EINVAL:
																				fprintf(stderr, "Cropping Not Supported\n");
																				break;
																default:
																				fprintf(stderr, "Errors Ignored\n");
																				break;
												}
								}
				}
				else
				{
								/* Do Nothing */
				}
				CLEAR(*fmt);
				fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				fmt->fmt.pix.width = camProp->getWidth();
				fmt->fmt.pix.height = camProp->getHeight();
				fmt->fmt.pix.pixelformat = camProp->getPixelformat();
				fmt->fmt.pix.field = camProp->getField();
				if(-1 == xioctl(fd, VIDIOC_S_FMT, fmt))
				{
								fprintf(stderr, "VIDIOC_S_FMT Error\n");
								return false;
				}

				min = fmt->fmt.pix.width * 2;
				if(fmt->fmt.pix.bytesperline < min)
					fmt->fmt.pix.bytesperline = min;
				min = fmt->fmt.pix.bytesperline * fmt->fmt.pix.height;
				if(fmt->fmt.pix.sizeimage < min)
					fmt->fmt.pix.sizeimage = min;

				if(!init_userPointer(fmt->fmt.pix.sizeimage))
				{
								fprintf(stderr, "initialize user pointer failed\n");
								return false;
				}
				return true;

				//	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
}
static bool libv4l2_open(CameraProperty* camProp)
{
				int fd;
				struct stat st; 
				if(-1 == stat(deviceName,&st))
				{
						fprintf(stderr, "Cannot identify '%s' : %d, %s\n", deviceName, errno, strerror(errno));
						return false;
				}
				if(!S_ISCHR(st.st_mode))
				{
						fprintf(stderr, "%s is no device\n", deviceName);
						return false;
				}
				//NINI
				fd = open(deviceName, O_RDWR, 0);
				if(-1 == fd)
				{
								fprintf(stderr, "Cannot open '%s' : %d, %s\n", deviceName, errno, strerror(errno));
								return false;
				}
	//			camProp->setStat(st);
				camProp->setfd(fd);
				return true;
}
static bool libv4l2_init(CameraProperty* camProp)
{
				return true;
}



static void errno_exit(const char *s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg)
{
	int r;
	do {
		r = ioctl(fh, request, arg);
	}while(-1 == r && EINTR == errno);
}
static bool init_SharedMemorySpace(int req_count, int buffer_size, int shmid, void** shmPtr, key_t shmkey)
{
				printf("buffer_len = %d, buffer_num = %d\n", buffer_size, req_count);
				
				shmid = shmget((key_t)shmkey, (buffer_size*req_count)+(sizeof(int)*req_count), 0666|IPC_CREAT);
				printf("shmkey : %d\n", shmkey);
				if(shmid == -1)
				{
								perror("shmget failed : ");
								return false;
				}
				*shmPtr = shmat(shmid, NULL, 0);
				
				if(*shmPtr == (void*)-1)
				{
								perror("shmget failed : ");
								return false;
				}
				printf("Shared Memory Region Initialization Success\n");
				return true;
}
static bool uninit_SharedMemorySpace(int shmid)
{
				if(-1 == shmctl(shmid, IPC_RMID, 0))
				{
								fprintf(stderr, "Failed to remove shared data region\n");
								return false;
				}
				return true;
}


