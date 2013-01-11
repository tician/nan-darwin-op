/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_COMMUNICATION__
#define __UGA_COMMUNICATION__ 1

#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

class Communication
{
 private:

 protected:
  int fileDescriptor;
  bool bindPort(int fd, char* ipAddress, int port);

 
 public:
  Communication();
  virtual ~Communication();
  //returns true on success, else false
  virtual bool    Connect   (char*, int) = 0;
  virtual bool    Connect   (int)        = 0;
  virtual char*   GetRemoteAddress()     = 0;
  virtual int     Send      (void* data    , int numBytes);
  virtual int     Receive   (void* data    , int numBytes);
  virtual void    Disconnect(                            );
};

//must include TCP before UDP
#include "TCPCommunication.h"
#include "UDPCommunication.h"
//#include "SerialCommunication.h"

#endif//__UGA_COMMUNICATION__ 
