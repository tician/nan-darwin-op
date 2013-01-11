/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_UDP_COMMUNICATION__
#define __UGA_UDP_COMMUNICATION__ 1

//#include "Communication.h"
//#include "TCPCommunication.h"

class UDPCommunication : public TCPCommunication
{
 private:

 protected:

 public:

  virtual bool    Connect    (char* ipAddress, int port);
  virtual bool    Connect    (                 int port);
  virtual int     Send       (void* data     , int numBytes);
  virtual int     Receive    (void* data     , int numBytes);
};

#endif//__UGA_UDP_COMMUNICATION__
