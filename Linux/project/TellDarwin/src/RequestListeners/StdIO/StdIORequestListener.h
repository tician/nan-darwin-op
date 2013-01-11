/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_STDIO_REQUEST_LISTENER__
#define __UGA_STDIO_REQUEST_LISTENER__ 1

#include <string.h>

class StdIORequestListener : public RequestListener
{
  private:
  
    char outData[50]; 
  protected:
  public:
    StdIORequestListener();
   ~StdIORequestListener();
    bool Initialize();
    bool WaitForRequest(char* buffer, int *bufferSize, float* values, int* numValues);
    void ReturnRequestedValues(const char* label,      float* values, int  numValues);
    char* GetRequesterIP();
};


#endif//__UGA_STDIO_REQUEST_LISTENER__
