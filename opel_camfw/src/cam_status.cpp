#include "cam_status.h"
CameraStatus* CameraStatus::camStats = NULL;


void CameraStatus::getThrMutex(pthread_mutex_t& mutex) 
{
	this->mutex = mutex;
}

bool CameraStatus::uinitSemaphore(void)
{
	sem_close(this->statusMutex);
	sem_unlink("CCDSTATUS");
	return true;
}

bool CameraStatus::initSemaphore(void)
{
	
	this->statusMutex = sem_open("CCDSTATUS", O_CREAT, 0666, 1);
	if(this->statusMutex == SEM_FAILED)
	{
		fprintf(stderr, "[CameraStatus::uinitSemaphore] : Init Semaphore Failed\n");
		sem_unlink("CCDSTATUS");
		return false;
	}
	return true;
}
bool CameraStatus::InitSharedPropertyToTarget(status** st)
{
	if(!(this->shmPtr_for_status)){
		this->shmid_for_status = shmget((key_t)SHM_KEY_FOR_STATUS, sizeof(status), 0666|IPC_CREAT);
		if(this->shmid_for_status == -1)
		{
			fprintf(stderr, "[CameraStatus::InitSharedPropertyToTarget] : shmget Error\n");
			return false;
		}
		shmPtr_for_status = (void*)shmat(this->shmid_for_status, NULL, 0);
		if(shmPtr_for_status == (void*)-1)
		{
			fprintf(stderr, "[CameraStatus::InitSharedPropertyToTarget] : shmat Error\n");
			return false;
		}
		*st = (status*)shmPtr_for_status;
		(*st)->isCCDRunning = this->isCCDRunning;
		(*st)->isOpenCVRunning = this->isOpenCVRunning;
		(*st)->isOpenCVInitialized = this->isOpenCVInitialized;
		(*st)->isRecRunning = this->isRecRunning;
		(*st)->isRecInitialized = this->isRecInitialized;
		return true;
	}
	else{
		return true;
	}
	return true;
}
CameraStatus::CameraStatus()
{

		this->isCCDRunning = false;
		this->isOpenCVRunning = false;
		this->isOpenCVInitialized = false;
		this->isRecRunning = false;
		this->isRecInitialized = false;
		this->statusMutex = sem_open("camStatus", O_CREAT, 0666, 1);
/*		if(this->statusMutex == SEM_FAILED)
		{
			fprintf(stderr, "[CameraStatus::CameraStatus]: Sem_open Error\n");
			sem_unlink("camStatus");	
			this->statusMutex = NULL;
		}	*/
}
CameraStatus::~CameraStatus()
{
/*	sem_close(this->statusMutex);
	sem_unlink("camStatus"); */
}
CameraStatus* CameraStatus::getInstance(void)
{
	if(camStats == (void*)0)
		camStats = new CameraStatus();
	return camStats;
}

