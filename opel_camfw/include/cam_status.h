#ifndef _CAM_STATUS_H_
#define _CAM_STATUS_H_
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <fcntl.h>
#define OPENCV_PROP_SHM 1588
#define SEM_KEY_FOR_STATUS 49441
#define SHM_KEY_FOR_STATUS 4944
#include "cam_property.h"

//char SEM_NAME_STATUS[] = "status";


typedef struct status
{
		bool isCCDRunning;
		bool isOpenCVRunning;
		bool isOpenCVInitialized;
		bool isRecRunning;
		bool isRecInitialized;

}status;

class CameraStatus
{
	public:
		~CameraStatus();
		void getThrMutex(pthread_mutex_t& mutex);
		static CameraStatus* getInstance(void);
		bool getIsOpenCVRunning() { return this->isOpenCVRunning; }
		void setIsOpenCVRunning(bool isRunning) 
		{ 
			sem_wait(statusMutex);
			this->isOpenCVRunning = isRunning; 
			statusObject->isOpenCVRunning = isRunning; 
			sem_post(statusMutex);
		}
		bool getIsOpenCVInitialized() { return this->isOpenCVInitialized; }
		void setIsOpenCVInitialized(bool isInitialized) 
		{ 
			sem_wait(statusMutex);
			this->isOpenCVInitialized = isInitialized; 
			statusObject->isOpenCVInitialized = isInitialized; 
			sem_post(statusMutex);
		}

		bool getIsRecRunning() { return this->isRecRunning; }
		void setIsRecRunning(bool isRunning) 
		{
			sem_wait(statusMutex);
			this->isRecRunning = isRunning;
			statusObject->isRecRunning = isRunning; 
			sem_post(statusMutex);
		};
		bool getIsRecInitialized() { return this->isRecInitialized; }
		void setIsRecInitialized(bool isInitialized) 
		{
			sem_wait(statusMutex);
			this->isRecInitialized = isInitialized; 
			statusObject->isRecInitialized = isInitialized;
			sem_post(statusMutex);
		}
		
		bool initSemaphore(void); 	
		bool uinitSemaphore(void);
		int getSemid(void){ return this->semid; } 
		bool InitSharedPropertyToTarget(status** st);
		void setStatusObject(status* st){ this->statusObject = st; } 
	
	private:
		static CameraStatus* camStats;
		CameraStatus();
		bool isCCDRunning;
		bool isOpenCVRunning;
		bool isOpenCVInitialized;
		bool isRecRunning;
		bool isRecInitialized;
		pthread_mutex_t mutex;
		sem_t* statusMutex;    
		int semid;
		int shmid_for_status;
		void* shmPtr_for_status;
		status* statusObject;

		//		const char* mutex_path = "camStatusMutex";
};

#endif   /*_CAM_STATUS_H_*/
