/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_TCP_COMMUNICATION__
#define __UGA_TCP_COMMUNICATION__ 1


//#include "Communication.h"

class TCPCommunication : public Communication
{
 private: 
   int localFileDescriptor;

 protected:
  struct sockaddr_in remoteAddress;
  socklen_t          remoteAddressLength;
  unsigned short     remotePort;

 public:
  TCPCommunication();
  
  virtual bool    Connect   (char* ipAddress, int port);
  virtual bool    Connect   (                 int port);
  virtual void    Disconnect(                         );
  virtual char*   GetRemoteAddress(                   );
  virtual int     GetRemotePort(                      );
};

#endif//__UGA_TCP_COMMUNICATION__
