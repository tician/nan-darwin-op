/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_NETCAT_REQUEST_LISTENER__
#define __UGA_NETCAT_REQUEST_LISTENER__ 1

#include "../../Communication/Communication.h"
#include <string.h>

class NetcatRequestListener : public RequestListener
{
  private:
  
    char outData[50]; 
  protected:
  public:
    NetcatRequestListener();
   ~NetcatRequestListener();
    bool Initialize();
    bool WaitForRequest(char* buffer, int *bufferSize, float* values, int* numValues);
    void ReturnRequestedValues(const char* label,      float* values, int  numValues);
};


#endif//__UGA_NETCAT_REQUEST_LISTENER__
