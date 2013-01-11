/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_REQUEST_LISTENER__
#define __UGA_REQUEST_LISTENER__ 1

#include "../Communication/Communication.h"
      
class RequestListener
{
  private:
  protected:

  Communication* socket;
  public:
  int port;

  RequestListener(){socket = NULL; port = 7400;}
  virtual ~RequestListener(){delete socket;}  
    
  /*
   * called by the intrepreter. return false if the setup failed;
   */
  virtual bool Initialize() = 0;
  
  
  
    /* 
     * This will be called by the interpreter when it is not doing anything      
     * else (ie after the previous request was processed). It should block 
     * until a request is recieved, and then fill *buffer* with a null terminated              
     * string containing the request, not to exceed *bufferSize* bytes. Additional         
     * numbers to be passed to the interpreter may be put in *values*. On enter,    
     * *numValues* contains the size of the array *values*, and on exit it must               
     * contain the actual number of numbers put into *values*. Note that numbers       
     * may also be passed into the interpreter in *buffer*, as their
     * character-encoded equivalents. return false to terminate the interpreter
     * If the request does not contain a string, the value 0 should be written
     * into *bufferSize*. if the request should not be performed, the value -1 
     * should be written into *numValues*
     */
    virtual bool WaitForRequest(char* buffer, int* bufferSize, float* values, int* numValuess) = 0;



    /* If the previous request asked for data (ie "get accelerometer") then the 
     * interpreter will call this with the requested data. *label* will contain a 
     * short, NULL-terminated string describing the data, *values* is an array 
     * containing the requested data, and *numValues* is the number of elements 
     * in *values*.
     */
    virtual void ReturnRequestedValues(const char* label, float* values, int numValues)      = 0;
    
    
    virtual char* GetRequesterIP(){return socket->GetRemoteAddress();}
};

#include "MAX/MAXRequestListener.h"
#include "Netcat/NetcatRequestListener.h"
#include "StdIO/StdIORequestListener.h"

#endif//__UGA_REQUEST_LISTENER__
