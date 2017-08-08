#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "CommLog.h"
#include "TmpControl.h"

#include "CommPort.h"

#define OPEL_WIFI_DIRECT_IP "192.168.49.1"

bool WifiDirectCommPort::openConnection() {
  __ENTER__;
  if(this->isOpened()) {
    CommLog("Wifi direct port has already opened");
    __EXIT__;
    return true;
  }

  // Open socket
  int newSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(newSocket < 0){
    CommLog("Wifi direct port open error: socket open fail");
    __EXIT__;
    return false;
	}

  // Set socket option
	struct sockaddr_in socketAddress;
	memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_addr.s_addr = inet_addr(OPEL_WIFI_DIRECT_IP);
	socketAddress.sin_port = htons(this->mTcpPortNum);
	int reuse = 1;
	if(setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR,
        (char *)&reuse, sizeof(int)) == -1){
    CommLog("Wifi direct port open error: socket option fail");
    __EXIT__;
    return false;
	}

  // Bind
  int bindRes = bind(newSocket, (struct sockaddr *)&socketAddress,
      sizeof(socketAddress));
	if(bindRes < 0){
		CommLog("Wifi direct port open error: bind fail (%s)",
        strerror(errno));
    __EXIT__;
    return false;
	}

  // Listen
#define NUM_PENDING_CONNECTIONS 5
  int listenRes = listen(newSocket, NUM_PENDING_CONNECTIONS);
	if(listenRes < 0){
    CommLog("Wifi direct port open error: listen fail");
    __EXIT__;
    return false;
	}
	
  // Set socket
	CommLog("Wifi direct port listening success");
  this->setSocket(newSocket);

  this->CommPort::openConnection();
  __EXIT__;
  return true;
}

bool WifiDirectCommPort::acceptConnection() {
  __ENTER__;
  // Accept connection
	struct sockaddr_in clientAddress;
	int clientAddressLength = sizeof(clientAddress);
	CommLog("Wifi direct port is now accepting: port = %d", this->mTcpPortNum);
  int acceptRes = accept(this->getSocket(), (struct sockaddr *)&clientAddress,
      (socklen_t *)&clientAddressLength);
	if(acceptRes < 0) {
		CommLog("Wifi direct port open error: accept fail (%s)", strerror(errno));
    this->closeConnection();
    __EXIT__;
		return false;
	}

	CommLog("Wifi direct port opening complete: client address = %s \n",\
			inet_ntoa(*(struct in_addr *)&clientAddress.sin_addr));
  
  this->CommPort::acceptConnection();
  __EXIT__;
  return true;
}

void WifiDirectCommPort::closeConnection() {
  __ENTER__;

  if(this->isOpened())
    close(this->getSocket());
  this->setSocket(-1);

  this->CommPort::closeConnection();
  __EXIT__;
}
