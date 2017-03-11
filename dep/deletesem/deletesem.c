#include <semaphore.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
char SEM_NAME[]= "vik";
char SEM_FOR_SELECT[]="vvik";
int main()
{
  char ch;
  int shmid;
  key_t key;
  char *shm,*s;
  sem_t *mutex;
  sem_t *semForSelect;
  mutex = sem_open(SEM_NAME,O_CREAT,0644,1);
  if(mutex == SEM_FAILED)
  {
    perror("unable to create semaphore");
    sem_unlink(SEM_NAME);
    exit(-1);
  }
  semForSelect = sem_open(SEM_FOR_SELECT, O_CREAT, 0664, 1);
  if(semForSelect == SEM_FAILED)
  {
    perror("unable to create semaphore");
    sem_unlink(SEM_FOR_SELECT);
    exit(-1);
  }
  sem_close(mutex);
  sem_close(semForSelect);
  sem_unlink(SEM_FOR_SELECT);
  sem_unlink(SEM_NAME);
  mutex = sem_open("CCDSTATUS", O_CREAT, 0666, 1);
  if(mutex == SEM_FAILED)
  {
    perror("unable to create Semaphore");
    sem_unlink("CCDSTATUS");
    exit(-1);
  }
  sem_close(mutex);
  sem_unlink("CCDSTATUS");
  return 0;

}
