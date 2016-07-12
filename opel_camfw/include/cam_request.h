#ifndef _CAM_REQUEST_H_
#define _CAM_REQUEST_H_
#include <stdio.h>

enum kinds {openCV=0, REC};
enum operations {init=0, start, stop, close_device};

typedef struct CameraRequest
{
 	pid_t pid;
//char* name;
	enum kinds kind;
	enum operations operation;

}CameraRequest;


	


#endif /*_CAM_REQUEST_H_*/
