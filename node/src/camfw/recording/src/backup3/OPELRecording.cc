#include "OPELRecording.h"
//const char* dev_name = "/dev/video0";
bool RecordingWorker::openFileCap()
{
	fout = fopen(file_path, "w+");	
	if(!fout)
		return false;
	return true;
}
bool RecordingWorker::readFrame()
{
	unsigned sz; 
	unsigned offset;
	offset = (buffer_index-1)*buffer_size;
	sz = fwrite((char*)shmPtr+offset, 1, buffer_size, fout);	
	if(sz != (unsigned)buffer_size)
	{
		return false;
	}
	return true;
}

void RecordingWorker::Execute()
{
	if(openFileCap()){
		while(count-- > 0)
		{
			for(;;)
			{
				fd_set fds;
				struct timeval tv;
				int r; 
				FD_ZERO(&fds);
				FD_SET(fd, &fds);
				tv.tv_sec = 10;
				tv.tv_usec = 0;
				r = select(fd+1, &fds, NULL, NULL, &tv);
				if(-1 == r)
				{
					if(EINTR == errno)
						continue;
				}
				if(0 == r)
				{
					break;
				}
				if(readFrame())
					break;
			}
		}
	}
	fclose(fout);
}  // end extern "C"
