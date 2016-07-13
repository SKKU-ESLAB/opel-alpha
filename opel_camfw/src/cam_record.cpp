#include "cam_core.h"
bool Record::init_device()
{
	/*unsigned int min;
	int fd = camProp->getfd();
	struct v4l2_capability* cap = camProp->getCapability();
	struct v4l2_cropcap* cropcap = camProp->getCropcap();
	struct v4l2_crop* crop = camProp->getCrop();
	struct v4l2_format* fmt = camProp->getFormat();
	if(-1 == xioctl(fd, VIDIOC_QUERYCAP, cap))
	{
		if(EINVAL == errno)
		{
			fprintf(stderr, "%s is no v4l2 device \n", deviceName);
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
			}
		
		}
	}*/
	return bool;

}
bool Record::init_userPointer(unsigned int buffer_size)
{

	unsigned reqCounts = REC_REQ_COUNT; 
	camProp->setN_buffers(reqCounts);
	struct v4l2_requestbuffers* req = camPorp->getRequestbuffers();
	struct v4l2_buffer* buf = camProp->getBuffer();
	memset(&reqbufs, 0, sizeof(req));
	req->count = reqCounts;
	req->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req->memory = V4L2_MEMORY_USERPTR;
	buffers = (buffer*)calloc(4, sizeof(*buffers));
	if(!buffers)
	{
		fprintf(stderr, "Out of Memeory\n");
		return false;
	}  
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
	for(unsigned int i=0; i<reqCounts; i++)
	{
		memset(&buf, 0, sizeof(buf));
		memset(planes, 0, sizeof(planes));
		buf->type = V4L2_BUF_TYPE_VIDEIO_CAPTURE;
		buf->memory = V4L2_MEMORY_USERPTR;
		buf->index = i;
		if(-1 == xioctl(fd, VIDIOC_QUERYBUF, buf))
		{
			fprintf(stderr, "VIDIOC_QUERYBUF");
			return false;
		}
		//					  buffers = (buffer*)calloc(4, sizeof(*buffers));
		buffers[i].length = buf.length;
		buffers[i].start = calloc(1, buf.length);
		if(!buffers[i].start)
		{
			fprintf(stderr, "index[%d] : Buffer Calloc Failed\n", i);
			return false;
		}
		buf->m.userptr = (unsigned long)buffers[i].start;
		if(-1 == xioctl(fd, VIDIOC_QBUF, buf))
		{
			fprintf(stderr, "VIDIOC_QBUF");
			return false;
		}

	}
	return true;
}
static int do_handle_cap(CameraProperty* camProp, buffer* buffers)
{
	char ch = '<';
	int ret;	
	struct v4l2_buffer* buf = camProp->getBuffer();
	int fd = camProp->getfd();
	int n_buffers = camProp->getN_buffers();
	unsigned n_buffers = camProp->getCount();
	unsigned i;
	unsigned* count = camProp->getCount();
	CLEAR(*buf);
	buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf->memory = V4L2_MEMORY_USERPTR;
	for(;;)
	{
		if(-1 == xioctl(fd, VIDIOC_DQBUF, buf))
		{
			switch(errno){
				case EAGAIN:
						return 0;
						break;
				case EIO:
						/* Ignore */
				default:
						fprintf(stderr, "VIDIOC_DQBUF\n");
						return false;
			}
		}
		for(i=0; i<n_buffers, ++i)
		{
			if(buf->m.


		}
			
	}


}
static bool recMainLoop(CameraProperty* camProp, buffer* buffers)
{ 
	 
   while((*count)-- > 0)
	{
	 for(;;){
	 fd_set read_fds;
	 fd_set exception_fds;
	 struct timeval tv = {2, 0};
	 int r;
	 FD_ZERO(&exception_fds);
	 FD_SET(fd, &exception_fds);
	 FD_ZERO(&read_fds);
	 FD_SET(fd, &read_fds);
   r = select(fd + 1, $read_fds, NULL, &exception_fds, &tv);
	 if(-1 == r){
		 		if(EINTR == errno)
							continue;
				fprintf(stderr, "Select error %s\n", strerror(errno));
				return false;
	 }
	 if(0 == r){
	 			fprintf(stderr, "Select Timeout\n");
				return false;
	 }
	/* if(FD_ISSET(fd, &exception_fds))
	 {
			

	 }
	*/
	 if(FD_ISSET(fd, &read_fds))
	 {	
		  r = this->do_handle_cap();
			if( r == -1)
				break;
	 }
	 }
	}
	return true;
}
bool Record::start()
{
				unsigned int i;
				int fd = camProp->getfd();
				unsigned int n_buffer = camProp->getN_buffers();
				struct v4l2_buffer* buf = camProp->getBuffer();
				enum v4l2_buf_type type = camProp_getType();
				type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if(-1 == xioctl(fd, VIDIOC_STREAMON, &type))
				{
					fprintf(stderr, "VIDIOC_STREAMON\n");
					return false;
				}
				if(recMainLoop(this->camProp, this->buffers))
				{
					fprintf(stderr, "REC_MAIN_LOOP\n");
					return false;
				}	
				return true;
}
bool Record::stop()
{
				
				
				
				return true;
}
bool Record::close_device()
{

				
				return true;
}











/*void Record::getCameraProperty(const CameraProperty& camProp)
{
				this->camProp = camProp;
}*/


