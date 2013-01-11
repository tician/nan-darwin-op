/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include "../RequestListeners.h"

NetcatRequestListener:: NetcatRequestListener()
{
  socket = NULL;
}

NetcatRequestListener::~NetcatRequestListener()
{
  socket->Disconnect();
}

bool NetcatRequestListener::Initialize()
{
  if(socket == NULL)
    socket = new TCPCommunication;

  if(!socket->Connect(port))
    return false;
  return true;
}

bool NetcatRequestListener::WaitForRequest(char* buffer, int *bufferSize, float* values, int* numValues)
{
  char* p = buffer;
  int numBytes;
  while(p-buffer < *bufferSize)
  {
    numBytes = socket->Receive(p, 1);
    if(numBytes <=0) Initialize();
    else if     ((*p == '\n')  || (*p ==  ';')) break;
    else    p++;
  }
  *p = '\0';
  *bufferSize = p - buffer;
  *numValues = (*bufferSize > 0) ? 0 : -1;
  return true;  
}


void NetcatRequestListener::ReturnRequestedValues(const char* label, float* values, int numValues)
{
  char* p = outData;
  int i;
  for(i=0; i<numValues; i++)
    {
      sprintf(p, "%f ", values[i]);
      p += strlen(p);
    }
  *p++ = '\n';
  *p++ = '\0';
                     
  socket->Send(outData, strlen(outData)+1);
}

