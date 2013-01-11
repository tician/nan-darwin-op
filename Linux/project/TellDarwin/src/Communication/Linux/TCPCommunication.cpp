/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include "Communication.h"
#include <string.h>

namespace Global{extern int verbosity;}

TCPCommunication::TCPCommunication()
{
  localFileDescriptor = -1;
  remoteAddressLength = sizeof(remoteAddress);
  memset(&remoteAddress, 0, remoteAddressLength);
}

bool TCPCommunication::Connect(char* ipAddress, int port)
{
  int error;

  if(fileDescriptor != -1) this->Disconnect();

  error = localFileDescriptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if((error == -1) && (Global::verbosity >= 1)) perror("TCPCommunication.cpp: unable to create a TCP socket"); 
  if(error != -1)
    {
      remotePort = port;
      remoteAddress.sin_family      = AF_INET;
      remoteAddress.sin_port        = htons(port);
      inet_pton(AF_INET, ipAddress, &remoteAddress.sin_addr);
      error = ::connect(localFileDescriptor, (struct sockaddr* )&remoteAddress, remoteAddressLength);
      fileDescriptor = localFileDescriptor;
      if((error == -1) && (Global::verbosity >= 1)) perror("TCPCommunication.cpp: Unable to connect to a remote host");
    } 
  return (error ==- 1) ? 0 : 1;
}

bool TCPCommunication::Connect(int port)
{
  int error;
  socklen_t len = sizeof(remoteAddress);

  if(fileDescriptor != -1) this->Disconnect();
  
  error = localFileDescriptor = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if((error < 0) && (Global::verbosity >= 1)) perror("TCPCommunication.cpp: unable to create a TCP socket"); 

  if(error >= 0)
    {
      remotePort = port;
      error = 0;
      
      error = Communication::bindPort(localFileDescriptor, (char*)"0.0.0.0", port);
      if(error != 0) 
        {
          error = -1;
          if(Global::verbosity >= 1) perror("TCPCommunication.cpp: unable to bind the TCP socket");
        }
    }
  if(error >= 0)
    {
      int maxNumberOfClients = 255;
      listen(localFileDescriptor, maxNumberOfClients);
      if((error < 0) && (Global::verbosity >= 1)) perror("TCPCommunication.cpp: unable to listen for incoming conections");
    }
  if(error >= 0)
    {
      error = fileDescriptor = accept(localFileDescriptor, (struct sockaddr*)&remoteAddress, &len);
      if((error < 0) && (Global::verbosity >= 1)) perror("TCPCommunication.cpp: unable to recieve a connection from a client");
    }
 
  return (error < 0) ? 0 : 1;
}


char* TCPCommunication::GetRemoteAddress()
{
  return inet_ntoa(remoteAddress.sin_addr);
}

int TCPCommunication::GetRemotePort()
{
  return remotePort;
}

void TCPCommunication::Disconnect()
{
  shutdown(fileDescriptor, SHUT_RDWR);
  Communication::Disconnect();
  shutdown(localFileDescriptor, SHUT_RDWR);   
  close(localFileDescriptor);
  localFileDescriptor = -1;
}

