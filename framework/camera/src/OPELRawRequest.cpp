#include "OPELRawRequest.h"

const char *sem_path = "ORG.OPEL.CAMERA";

OPELRawRequest *OPELRawRequest::opel_raw_request = NULL;

OPELRawRequest::OPELRawRequest(RequestType type, GstElement *bin)
    : CameraRequest(type, bin)
{
}

OPELRawRequest::~OPELRawRequest()
{
  uinitSharedMemorySpace(this->shm_id);
  uinitSemaphore(sem_path, this->sem);
  opel_raw_request = NULL;
}

OPELRawRequest *OPELRawRequest::getInstance(RequestType type, GstElement *bin)
{
  if (opel_raw_request == NULL)
    opel_raw_request = new OPELRawRequest(type, bin);
  return opel_raw_request;
}

bool OPELRawRequest::initializeSemAndSharedMemory(void)
{
  int ret = initSharedMemorySpace(1, RAW_DEFAULT_BUF_SIZE,
      &(this->shm_ptr), SHM_KEY_FOR_BUFFER);
  if(this->shm_ptr == NULL)
  {
    OPEL_LOG_ERR(CAM, "Shared memory space has NULL address");
    return false;
  }
  if(ret == -1){
    OPEL_LOG_ERR(CAM, "Shared memory space initialization failed");
    return false;
  }
  this->shm_id = ret;

  if(initSemaphore(sem_path, &(this->sem)) == false)
  {
    OPEL_LOG_ERR(CAM, "Semaphore initailization failed");
    return false;
  }
  
  return true;
}

bool OPELRawRequest::uninitializeSemAndSharedMemory(void)
{
  uinitSharedMemorySpace(this->shm_id);
  uinitSemaphore(sem_path, this->sem);
}

static int initSharedMemorySpace(int _req_count, int _buffer_size,
    void** _shm_ptr, key_t _shmkey)
{
  int shmid;
  shmid = shmget((key_t)_shmkey,
      (_buffer_size*_req_count)+(sizeof(int)*_req_count), 0666|IPC_CREAT);

  if(shmid == -1)
  {
    OPEL_LOG_ERR(CAM, "shmget failed : ");
    return -1;
  }
  *_shm_ptr = shmat(shmid, NULL, 0);

  if(*_shm_ptr == (void*)-1)
  {
    OPEL_LOG_ERR(CAM, "shmget failed : ");
    return -1;
  }
  return shmid;
}

static int uinitSharedMemorySpace(int _shm_id)
{
  if(-1 == shmctl(_shm_id, IPC_RMID, 0))
  {
    OPEL_LOG_ERR(CAM, "Failed to remove shared memory space");
    return 0;
  }
  return 1;
}

static bool initSemaphore(const char *path, sem_t **_sem)
{
  assert(path != NULL);
  *_sem = sem_open(path, O_CREAT, 0666, 1);
  if((*_sem) == SEM_FAILED)
  {
    OPEL_LOG_ERR(CAM, "Semaphore Open Failed");
    sem_unlink(path);
    return false;
  }
  return true;		
}

static void uinitSemaphore(const char *path, sem_t *sem)
{
  sem_close(sem);
  sem_unlink(path);
  sem_destroy(sem);
}

