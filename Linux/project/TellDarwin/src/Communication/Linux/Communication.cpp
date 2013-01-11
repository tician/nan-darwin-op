/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include "Communication.h"
#include <string.h>


Communication::Communication()
{ fileDescriptor = -1; }

Communication::~Communication()
{ this->Disconnect(); }

bool Communication::bindPort(int fd, char* ipAddress, int port)
{
  struct sockaddr_in thisAddress;
  memset(&thisAddress, 0, sizeof(thisAddress));
  thisAddress.sin_family      = AF_INET;
  thisAddress.sin_port        = htons(port);
  thisAddress.sin_addr.s_addr = htonl(inet_addr(ipAddress));
  
  int error = bind(fd, (struct sockaddr*)&thisAddress, sizeof(thisAddress));
return error;
}

int Communication::Send    (void* data    , int numBytes)
{  return write(fileDescriptor, data, numBytes);  }

int Communication::Receive (void* data    , int numBytes)
{  return read(fileDescriptor, data, numBytes);  }

void Communication::Disconnect()
{ close(fileDescriptor); fileDescriptor = -1; }

