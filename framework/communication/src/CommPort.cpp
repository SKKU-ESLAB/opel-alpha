/* Copyright (c) 2015-2017 CISS, and contributors. All rights reserved.
 *
 * Contributor: Gyeonghwan Hong<redcarrottt@gmail.com>
 *              Eunsoo Park<esevan.park@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <mutex>
#include <fcntl.h>
#include <sys/time.h>

#include "CommLog.h"
#include "CommRawPacket.h"

#include "CommPort.h"

int getFileSize(const char* filePath);

void CommPortState::set(Value newValue) {
  Value oldValue = this->mValue;
  this->mValue = newValue;
  if((this->mStateListener != NULL) && (oldValue != newValue)) {
    this->mStateListener->onCommPortStateChanged(this->mOwner,
        this->mValue);
  }
}

void CommPort::runListeningThread(CommPortListener* listener,
    std::string downloadFilePath) {
  __ENTER__;
  this->mListener = listener;
  this->mDownloadFilePath = downloadFilePath;
  this->mIsListeningThreadOn = true;

  pthread_create(&this->mListeningThread, NULL, CommPort::listeningLoop,
      (void*)this);
  __EXIT__;
}

void CommPort::stopListeningThread() {
  __ENTER__;
  this->mIsListeningThreadOn = false;
  __EXIT__;
}

#define IF_NULL_(x) if(x == NULL) {\
  CommLog("%s: %s is NULL", __FUNCTION__, #x); \
  self->stopListeningThread();

#define _RETURN_FALSE() __EXIT__;\
  return false; }

#define _BREAK() break; }

void* CommPort::listeningLoop(void* data) {
  __ENTER__;
  CommPort* self = (CommPort*)data;

  // Accept client
  while(!self->acceptConnection()) { }

  const int kMessageMetadata = 1;
  const int kMessageData = 2;
  const int kFileMetadata = 3;
  const int kFileData = 4;

  bool isMessageCompleted = false;
  int expectedPayloadType = kMessageMetadata;
  CommPayloadMessageMetadata* messageMetadata = NULL;
  CommPayloadFileMetadata* fileMetadata = NULL;
  char* totalMessageData = NULL;
  int loadedBytesSize = 0;
  FILE* fdToWrite = NULL;

  // Listening loop
  while(self->mIsListeningThreadOn) {
    // Get message from communication framework
    CommLog("%s: Waiting to listen message", self->mPortName.c_str());

    // Read header
    CommRawPacketHeader* header
      = CommRawPacketHeader::readFromSocket(self->getSocket());
    IF_NULL_(header) { } _BREAK()
    
    CommLog("Expected type: %d", expectedPayloadType);
    // Read payload
    switch(expectedPayloadType) {
      case kMessageMetadata:
        {
          // Message metadata is expected
          if(!header->getFlagIsMetadata()) {
            CommLog("Expected MessageMetadata, but flag is: %d", header->getHeaderFlag());
            break;
          }
          messageMetadata
            = CommPayloadMessageMetadata::readFromSocket(self->getSocket());
          IF_NULL_(messageMetadata) { } _BREAK()

          totalMessageData = new char[messageMetadata->getMessageDataLength()];
          IF_NULL_(totalMessageData) { } _BREAK()
          expectedPayloadType = kMessageData;
          CommLog("Read complete: message metadata");
          break;
        }

      case kMessageData:
        {
          // Message data is expected
          if(!header->getFlagIsData()) {
            CommLog("Expected Messagedata, but flag is: %d", header->getHeaderFlag());
            break;
          }
          CommPayloadData* messageData
            = CommPayloadData::readFromSocket(self->getSocket(), header->getPayloadSize());
          IF_NULL_(messageData) { } _BREAK()
            char* messageDataBytes = messageData->toByteArray();
          IF_NULL_(messageDataBytes) {
            delete messageData;
            messageData = NULL;
          } _BREAK()
          memcpy(totalMessageData + loadedBytesSize,
              messageDataBytes + 0,
              messageData->getBytesSize());
          loadedBytesSize += messageData->getBytesSize();
          if(header->getFlagIsEndData()) {
            if(messageMetadata->getIsFileAttached() != 0) {
              expectedPayloadType = kFileMetadata;
            } else {
              // Message without file completed
              expectedPayloadType = kMessageMetadata;
              isMessageCompleted = true;
            }
          } else {
            expectedPayloadType = kMessageData;
          }
          CommLog("Read complete: message data");
          break;
        }

      case kFileMetadata:
        {
          // File metadata is expected
          if(!header->getFlagIsMetadata()) {
            CommLog("Expected File Metadata, but flag is: %d", header->getHeaderFlag());
            break;
          }
          fileMetadata
            = CommPayloadFileMetadata::readFromSocket(self->getSocket());
          IF_NULL_(fileMetadata) { } _BREAK()

          // Open file
          char* fileName = fileMetadata->getSrcFileName();
          IF_NULL_(fileName) { } _BREAK()
            char filePath[2048];
          strncpy(filePath, self->mDownloadFilePath.c_str(),
              strlen(self->mDownloadFilePath.c_str()));
          strncat(filePath, fileName, strlen(fileName));
          fdToWrite = fopen(filePath, "a");
          IF_NULL_(fdToWrite) {
            delete fileMetadata;
            fileMetadata = NULL;
          } _BREAK()

          // Expect next packet
          expectedPayloadType = kFileData;
          CommLog("Read complete: file metadata");
          break;
        }

      case kFileData:
        {
          // File data is expected
          if(!header->getFlagIsFile()) {
            CommLog("Expected File Data, but flag is: %d", header->getHeaderFlag());
            break;
          }
          CommPayloadData* fileData
            = CommPayloadData::readFromSocket(self->getSocket(), header->getPayloadSize());
          IF_NULL_(fileData) { } _BREAK()

          // Write
          char* fileDataBytes = fileData->toByteArray();
          IF_NULL_(fileDataBytes) {
            delete fileData;
          } _BREAK()
          fwrite(fileDataBytes, sizeof(char), fileData->getBytesSize(),
              fdToWrite);

          // Expect next packet
          if(header->getFlagIsEndData()) {
            // Message with file completed
            expectedPayloadType = kMessageMetadata;
            isMessageCompleted = true;
          } else {
            expectedPayloadType = kFileData;
          }
          CommLog("Read complete: file data");
          break;
        }
    }

    if(isMessageCompleted) {
      // Notify message to CommPortListener
      std::string messageData(totalMessageData);
      if(self->mListener != NULL) {
        if(fileMetadata != NULL) {
          std::string filePath(fileMetadata->getSrcFileName());
          self->mListener->onReceivedRawMessage(messageData, filePath);
        } else {
          self->mListener->onReceivedRawMessage(messageData);
        }
      } else {
        CommLog("There is no CommPortListener!");
      }

      // Initialize variables
      expectedPayloadType = kMessageMetadata;
      if(messageMetadata != NULL) {
        delete messageMetadata;
        messageMetadata = NULL;
      }
      if(fileMetadata != NULL) {
        delete fileMetadata;
        fileMetadata = NULL;
      }
      if(totalMessageData != NULL) {
        delete[] totalMessageData;
        totalMessageData = NULL;
      }
      if(fdToWrite != NULL) {
        fclose(fdToWrite);
        fdToWrite = NULL;
      }
      loadedBytesSize = 0;
      isMessageCompleted = false;
    }

    // Finalize (while)
    if(header != NULL) {
      delete header;
      header = NULL;
    }
  }

  // Finalize
  if(messageMetadata != NULL) {
    delete messageMetadata;
    messageMetadata = NULL;
  }
  if(fileMetadata != NULL) {
    delete fileMetadata;
    fileMetadata = NULL;
  }
  if(totalMessageData != NULL) {
    delete[] totalMessageData;
    totalMessageData = NULL;
  }
  if(fdToWrite != NULL) {
    fclose(fdToWrite);
    fdToWrite = NULL;
  }
  self->closeConnection();
  __EXIT__;
}

bool CommPort::sendRawMessage(std::string messageData) {
  return this->sendRawMessage(messageData, "");
}

#define WRITE_SOCKET(socketFd, data, size) \
  do { \
    if(write(socketFd, data, size) < 0) { \
      CommLog("write socket error: %s", strerror(errno)); \
      __EXIT__; \
      return false; \
    } \
  } while(0);


bool CommPort::sendRawMessage(std::string messageData, std::string filePath) {
  __ENTER__;
  CommPort* self = (CommPort*)this;
  if(!this->isConnected()) {
    __EXIT__;
    return -1;
  }
  int messageDataLength = messageData.length();

  // Send message metadata and message data
  {
    this->mPresentHeaderId++;

    CommLog("Send message metadata (%d)", this->mPresentHeaderId);

    // Send message metadata
    {
      CommRawPacket* messageMetadataPacket
        = CommRawPacket::makeMessageMetadataPacket(
            this->mPresentHeaderId, messageDataLength,
            (filePath.length() > 0)); 
      IF_NULL_(messageMetadataPacket) { } _RETURN_FALSE()
      char* messageMetadataBytes = messageMetadataPacket->toByteArray();
      IF_NULL_(messageMetadataBytes) {
        delete messageMetadataPacket;
      } _RETURN_FALSE()
      WRITE_SOCKET(this->getSocket(),
          messageMetadataBytes, messageMetadataPacket->getBytesSize());
      delete[] messageMetadataBytes;
      delete messageMetadataPacket;
    }

    CommLog("Send message data (%d)", this->mPresentHeaderId);

    // Send message data (split by limited-size packets)
    int sentBytes = 0;
    while(sentBytes < messageDataLength) {
      // Make message data packet
      short messageDataPayloadSize;
      bool isEnd;
      if(CommRawPacketHeader::kMaxPacketPayloadSize
          < messageDataLength - sentBytes) {
        // Not end data
        messageDataPayloadSize = CommRawPacketHeader::kMaxPacketPayloadSize;
        isEnd = false;
      } else {
        // End data
        messageDataPayloadSize = (short) (messageDataLength - sentBytes);
        isEnd = true;
      }
      char* messageDataPayloadBytes = new char[messageDataPayloadSize];
      IF_NULL_(messageDataPayloadBytes) { } _RETURN_FALSE()
      memcpy(messageDataPayloadBytes + 0,
          messageData.c_str() + sentBytes,
          messageDataPayloadSize);
      CommRawPacket* messageDataPacket = CommRawPacket::makeDataPacket(
          this->mPresentHeaderId, messageDataPayloadBytes, sentBytes,
          messageDataPayloadSize, isEnd, false);
      IF_NULL_(messageDataPacket) {
        delete messageDataPayloadBytes;
      } _RETURN_FALSE()
      CommLog("sendRawMessage(message): mPayloadSize = %d",
          messageDataPayloadSize);

      // Send
      char* messageDataPacketBytes = messageDataPacket->toByteArray();
      IF_NULL_(messageDataPacketBytes) {
        delete messageDataPacket;
      } _RETURN_FALSE()
      WRITE_SOCKET(this->getSocket(),
          messageDataPacketBytes, messageDataPacket->getBytesSize());
      sentBytes += messageDataPayloadSize;

      // Finalize (while)
      delete messageDataPacket;
      delete messageDataPacketBytes;
    }
  }
  // If file is attached, send file metadata and file data
  if(filePath.length() > 0) {
    CommLog("Send attached file (%d)", this->mPresentHeaderId);

    // Send file metadata
    int fileSize = getFileSize(filePath.c_str());
    CommRawPacket* fileMetadataPacket = CommRawPacket::makeFileMetadataPacket(
        this->mPresentHeaderId, filePath.c_str(), fileSize);
    IF_NULL_(fileMetadataPacket) {
    } _RETURN_FALSE()
    char* fileMetadataBytes = fileMetadataPacket->toByteArray();
    IF_NULL_(fileMetadataBytes) {
      delete fileMetadataPacket;
    } _RETURN_FALSE()
    WRITE_SOCKET(this->getSocket(),
        fileMetadataBytes, fileMetadataPacket->getBytesSize());

    // Send file data (split by limited-size packets)
    int sentBytes = 0;
    FILE* fdToRead = NULL;
    fdToRead = fopen(filePath.c_str(), "r");
    IF_NULL_(fdToRead) {
      delete fileMetadataBytes;
      delete fileMetadataPacket;
    } _RETURN_FALSE()
    while(sentBytes < fileSize) {
      // Make file data packet
      CommRawPacket* fileDataPacket;
      int fileDataPayloadSize;
      bool isEnd;
      if(CommRawPacketHeader::kMaxPacketPayloadSize
          < fileSize - sentBytes) {
        // Not end data
        fileDataPayloadSize = CommRawPacketHeader::kMaxPacketPayloadSize;
        isEnd = false;
      } else {
        // End data
        fileDataPayloadSize = fileSize - sentBytes;
        isEnd = true;
      }
      char* fileDataPayloadBytes = new char[fileDataPayloadSize];
      int freadRes = fread(fileDataPayloadBytes + sentBytes,
          sizeof(char),
          fileDataPayloadSize,
          fdToRead);
      if(freadRes <= 0) break;
      fileDataPacket = CommRawPacket::makeDataPacket(
          this->mPresentHeaderId, fileDataPayloadBytes, sentBytes,
          CommRawPacketHeader::kMaxPacketPayloadSize, isEnd, true);
      IF_NULL_(fileDataPacket) {
        fclose(fdToRead);
      } _RETURN_FALSE()
      CommLog("sendRawMessage(file): mPayloadSize = %d", fileDataPayloadSize);

      // Send
      char* fileDataPacketBytes = fileDataPacket->toByteArray();
      IF_NULL_(fileDataPacketBytes) {
        delete fileMetadataPacket;
        fclose(fdToRead);
      } _RETURN_FALSE()
      WRITE_SOCKET(this->getSocket(),
          fileDataPacketBytes, fileDataPacket->getBytesSize());
      sentBytes += fileDataPayloadSize;

      // Finalize (while)
      delete fileDataPacket;
    }

    // Finalize (if)
    delete fileMetadataPacket;
    delete fileMetadataBytes;

    if(fdToRead != NULL) {
      fclose(fdToRead);
    }
  }

  CommLog("Send message done (%d)", this->mPresentHeaderId);
  __EXIT__;
  return true;
}

int getFileSize(const char* filePath) {
  FILE* fd = fopen(filePath, "r");
  if(fd < 0) return -1;
  int fileSize = fseek(fd, 0, SEEK_END);
  fclose(fd);
  return fileSize;
}
