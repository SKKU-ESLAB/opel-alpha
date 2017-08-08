#include <cstring>
#include <pthread.h>

#include "Channel.h"
#include "OPELdbugLog.h"

void Channel::runRoutedLoop(bool isCreateRoutedThread) {
  if(isCreateRoutedThread == true) {
    // Run on a separate thread
    pthread_create(&this->mRoutedThread, NULL, Channel::routedLoop,
        (void *)this);
  } else {
    // Run on this thread
    this->routedLoop(this);
  }
}

void* Channel::routedLoop(void* data) {
  Channel* self = (Channel*)data;

  while(1) {
    pthread_mutex_lock(&self->mWaitMutex);

    // Wait until someone enqueues a message to the RoutedMessageQueue
    int queueSize = self->getQueueSizeLocked();
    while(!(queueSize > 0)) {
      pthread_cond_wait(&self->mWaitCond, &self->mWaitMutex);
    }

    // If any message is in RoutedMessageQueue, handle one message
    BaseMessage* message = self->dequeueRoutedMessageLocked();

    // Handle the message only if the message is compatible with this Channel
    if(self->checkMessageCompatible(message)) {
      self->onRoutedMessage(message);
    } else{
      OPEL_DBG_WARN("Given message is not compatible with this Channel!");
    }

    pthread_mutex_unlock(&self->mWaitMutex);
  }
}

void Channel::routeMessage(BaseMessage* message) {
  pthread_mutex_lock(&this->mWaitMutex);

  // Push to mesage queue
  pthread_mutex_lock(&this->mRoutedMessageQueueMutex);
  this->mRoutedMessageQueue.push_back(message);
  pthread_mutex_unlock(&this->mRoutedMessageQueueMutex);

  // Wake up RoutedLoop so that the loop can handle the message
  pthread_cond_signal(&this->mWaitCond);

  pthread_mutex_unlock(&this->mWaitMutex);
}

int Channel::getQueueSizeLocked() {
  int queueSize; 
  pthread_mutex_lock(&this->mRoutedMessageQueueMutex);
  queueSize = this->mRoutedMessageQueue.size();
  pthread_mutex_unlock(&this->mRoutedMessageQueueMutex);
  return queueSize;
}

BaseMessage* Channel::dequeueRoutedMessageLocked() {
  pthread_mutex_lock(&this->mRoutedMessageQueueMutex);
  std::vector<BaseMessage*>::iterator it = this->mRoutedMessageQueue.begin();
  BaseMessage* message = *it;
  this->mRoutedMessageQueue.erase(it);
  pthread_mutex_unlock(&this->mRoutedMessageQueueMutex);
  return message;
}
