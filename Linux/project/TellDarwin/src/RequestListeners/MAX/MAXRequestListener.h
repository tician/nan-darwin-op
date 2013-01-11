/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_MAX_REQUEST_LISTENER__
#define __UGA_MAX_REQUEST_LISTENER__ 1

#include "../../Communication/Communication.h"
#include "MAXTypes.h"

using namespace MAX;

class MAXRequestListener : public RequestListener
{
  private:
    MAXType_t  maxType;
  
    char outData[300]; 
  protected:
  public:
    MAXRequestListener();
   ~MAXRequestListener();
    bool Initialize();
    bool WaitForRequest(char* buffer, int *bufferSize, float* values, int* numValues);
    void ReturnRequestedValues(const char* label, float* values, int numValues)     ;
};


#endif//__UGA_MAX_REQUEST_LISTENER__
