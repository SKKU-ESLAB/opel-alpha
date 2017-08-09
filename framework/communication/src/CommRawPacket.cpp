#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "CommLog.h"

#include "CommRawPacket.h"

CommRawPacket* CommRawPacket::makeMessageMetadataPacket(
    char headerId, int totalDataSize, bool isFileAttached) {
  CommPayloadMessageMetadata* payload = new CommPayloadMessageMetadata(
      totalDataSize, isFileAttached);
  CommRawPacketHeader* header = new CommRawPacketHeader(
      headerId, payload->getBytesSize(), 0,
      false, false, false, true);
  return new CommRawPacket(header, payload);
}
CommRawPacket* CommRawPacket::makeFileMetadataPacket(
    char headerId, const char* fileName, int fileSize) {
  CommPayloadFileMetadata* payload = new CommPayloadFileMetadata(
      (int) fileSize, (char) strlen(fileName), fileName);
  CommRawPacketHeader* header = new CommRawPacketHeader(
      headerId, payload->getBytesSize(), 0,
      false, false, false, true);
  return new CommRawPacket(header, payload);
}

CommRawPacket* CommRawPacket::makeDataPacket(
    char headerId, char* data, int currOffset, short size,
    bool isEnd, bool isFile) {
  CommPayloadData* payload = new CommPayloadData(
      data, size);
  CommRawPacketHeader* header = new CommRawPacketHeader(
      headerId, payload->getBytesSize(), 0,
      isFile, !isFile, isEnd, false);
  return new CommRawPacket(header, payload);
}

char* CommRawPacket::toByteArray() {
  int resLength = this->getBytesSize();
  char* resBytes = new char[resLength];
  char* headerBytes = NULL;
  char* payloadBytes = NULL;

  // Payload header
  headerBytes = this->mHeader->toByteArray();
  if(headerBytes == NULL) return NULL;
  memcpy(resBytes + 0,
      headerBytes + 0,
      this->mHeader->getBytesSize());

  // Payload
  payloadBytes = this->mPayload->toByteArray();
  if(payloadBytes == NULL) return NULL;
  memcpy(resBytes + this->mHeader->getBytesSize(),
      payloadBytes + 0,
      this->mPayload->getBytesSize());
  delete[] resBytes;
}

#define READ_SOCKET_C(socketFd, data) \
  if(read(socketFd, &data, sizeof(char)) < 0) return NULL;
#define READ_SOCKET_S(socketFd, data) \
  if(read(socketFd, &data, sizeof(short)) < 0) return NULL; \
  data = ntohs(data);
#define READ_SOCKET_I(socketFd, data) \
  if(read(socketFd, &data, sizeof(int)) < 0) return NULL; \
  data = ntohl(data);
#define READ_SOCKET(socketFd, data, size) \
  if(read(socketFd, data, size) < 0) return NULL;

CommRawPacketHeader* CommRawPacketHeader::readFromSocket(int socketFd) {
  int readRes;
  char headerId;
  char headerFlag;
  short payloadSize;
  int currOffset;
  READ_SOCKET_C(socketFd, headerId);
  READ_SOCKET_C(socketFd, headerFlag);
  READ_SOCKET_S(socketFd, payloadSize);
  READ_SOCKET_I(socketFd, currOffset);
  //CommLog("header: %d / %d / %d / %d", (int)headerId, (int)headerFlag, (int)payloadSize, currOffset);
  CommRawPacketHeader* header = new CommRawPacketHeader(
      headerId, payloadSize, currOffset, headerFlag);
  return header;
}

#define NEW_BYTEARRAY(byteArray, size) \
  char* byteArray = new char[size]; \
  char* writePtr = byteArray;
#define WRITE_BYTEARRAY_C(x) memcpy(writePtr, x, sizeof(char)); \
  writePtr += sizeof(char);
#define WRITE_BYTEARRAY_S(x) *x = htons(*x); \
  memcpy(writePtr, x, sizeof(short)); \
  writePtr += sizeof(short);
#define WRITE_BYTEARRAY_I(x) *x = htonl(*x); \
  memcpy(writePtr, x, sizeof(int)); \
  writePtr += sizeof(int);
#define WRITE_BYTEARRAY_SIZE(x, s) memcpy(writePtr, x, s); \
  writePtr += s;

char* CommRawPacketHeader::toByteArray() {
  NEW_BYTEARRAY(byteArray, this->getBytesSize());
  WRITE_BYTEARRAY_C(&this->mHeaderId);
  WRITE_BYTEARRAY_C(&this->mHeaderFlag);
  WRITE_BYTEARRAY_S(&this->mPayloadSize);
  WRITE_BYTEARRAY_I(&this->mCurrOffset);
  return byteArray;
}

CommPayloadMessageMetadata* CommPayloadMessageMetadata::readFromSocket(
    int socketFd) {
  int messageDataLength;
  int isFileAttached;
  READ_SOCKET_I(socketFd, messageDataLength);
  READ_SOCKET_I(socketFd, isFileAttached);
  CommPayloadMessageMetadata* messageMetadata = new CommPayloadMessageMetadata(
      messageDataLength, (isFileAttached != 0));
  return messageMetadata;
}

char* CommPayloadMessageMetadata::toByteArray() {
  if(this->mMessageDataLength == 0) return NULL;

  NEW_BYTEARRAY(byteArray, this->getBytesSize());
  WRITE_BYTEARRAY_I(&this->mMessageDataLength);
  WRITE_BYTEARRAY_I(&this->mIsFileAttached);
  return byteArray;
}

CommPayloadFileMetadata* CommPayloadFileMetadata::readFromSocket(int socketFd) {
  int fileSize;
  char fileNameLength;
  char* fileName;
  READ_SOCKET_I(socketFd, fileSize);
  READ_SOCKET_C(socketFd, fileNameLength);
  fileName = new char[fileNameLength + 1];
  READ_SOCKET(socketFd, fileName, fileNameLength + 1);
  CommPayloadFileMetadata* fileMetadata = new CommPayloadFileMetadata(
      fileSize, fileNameLength, fileName);
  return fileMetadata;
}

char* CommPayloadFileMetadata::toByteArray() {
  NEW_BYTEARRAY(byteArray, this->getBytesSize());
  WRITE_BYTEARRAY_I(&this->mFileSize);
  WRITE_BYTEARRAY_C(&this->mFileNameLength);
  WRITE_BYTEARRAY_SIZE(this->mFileName, this->mFileNameLength + 1);
  return byteArray;
}

CommPayloadData* CommPayloadData::readFromSocket(int socketFd, short dataSize) {
  char* data = new char[dataSize];
  READ_SOCKET(socketFd, data, dataSize);
  CommPayloadData* dataPayload = new CommPayloadData(data, dataSize);
  return dataPayload;
}

char* CommPayloadData::toByteArray() {
  return this->mData;
}
