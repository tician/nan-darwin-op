/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include "../RequestListeners.h"

StdIORequestListener:: StdIORequestListener()
{}

StdIORequestListener::~StdIORequestListener()
{}

bool StdIORequestListener::Initialize()
{
  return true;
}

bool StdIORequestListener::WaitForRequest(char* buffer, int *bufferSize, float* values, int* numValues)
{
  char* p = buffer;
  int next;
  //fflush(stdin);
  while(p-buffer < *bufferSize)
  {
    next = fgetc(stdin);
    if     ((next == '\n')  ||
            (next ==  ';')) break;
    else if (next ==  EOF) return false;
    else    *p++ = (char)next; 
  }
  *p = '\0';
  *bufferSize = p - buffer;
  *numValues = (*bufferSize > 0) ? 0 : -1;
  return true;  
}

void StdIORequestListener::ReturnRequestedValues(const char* label, float* values, int numValues)
{
  char* p = outData;
  int i;
  for(i=0; i<numValues; i++)
    {
      sprintf(p, "%f ", values[i]);
      p += strlen(p);
    }
  *p++ = '\n';
  *p = '\0';
  printf("%s", outData);
}

char* StdIORequestListener::GetRequesterIP()
{
  printf("what IP do you want to stream it to?\n");
  scanf("%s", outData);
  return outData;
}
