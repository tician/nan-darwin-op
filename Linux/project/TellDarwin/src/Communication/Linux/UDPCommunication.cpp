/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include "Communication.h"

namespace Global{extern int verbosity;}

bool UDPCommunication::Connect(char* ipAddress, int port)
{
  fprintf(stderr, "UDPCommunication.cpp: Connect as host unimplemented\n");
  return false;
}

bool UDPCommunication::Connect(int port)
{
  int error;

  if(fileDescriptor != -1) this->Disconnect();

  error = fileDescriptor = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if((error == -1) && (Global::verbosity >= 1)) perror("UDPCommunication.cpp: unable to create a UDP socket\n"); 

  if(error != -1)
    {
      remotePort = port;
      error = Communication::bindPort(fileDescriptor, (char*)"0.0.0.0", port);
      if((error == -1) && (Global::verbosity >= 1)) perror("UDPCommunication.cpp: unable to bind\n"); 
    }
  return (error == -1) ? false : true;
}  

int UDPCommunication::Send(void* data, int numBytes)
{
  remoteAddress.sin_port = htons(remotePort);
  //remoteAddress.sin_port = htons(7401);
  return sendto(fileDescriptor, data, numBytes, 0, (struct sockaddr *)&remoteAddress, remoteAddressLength);
}

int UDPCommunication::Receive(void* data, int numBytes)
{
  return recvfrom(fileDescriptor, data, numBytes, 0, (struct sockaddr *)&remoteAddress, &remoteAddressLength);
}


