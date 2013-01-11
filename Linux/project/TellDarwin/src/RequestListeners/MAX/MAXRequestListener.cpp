/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include "../RequestListeners.h"

MAXRequestListener:: MAXRequestListener()
{
    socket = NULL;
}

MAXRequestListener::~MAXRequestListener()
{
  socket->Disconnect();
  delete socket;
}

bool MAXRequestListener::Initialize()
{
  if(socket == NULL)  
    socket = new UDPCommunication;
  if(!socket->Connect(port))
    return false;
  return true;
}

bool MAXRequestListener::WaitForRequest(char* buffer, int *bufferSize, float* values, int* numValues)
{
  *bufferSize = socket->Receive(buffer, *bufferSize);
  if(*bufferSize <= 0) return false;
  
  maxType = determineMAXType(buffer, *bufferSize);       
  
  switch(maxType)
  {
    case INT_TYPE:
      *numValues  = 1;
      *bufferSize = 0;
      values[0] = (float)MAXIntToInt(buffer);
      break;
    case FLOAT_TYPE:
      *numValues  = 1;
      *bufferSize = 0;
      values[0] = MAXFloatToFloat(buffer);
      break;  
    case BANG_TYPE:
      *bufferSize = 0;
      *numValues  = 0; 
      break;
    case MIXED_TYPE: 
      MAXMixedMessageToStringAndFloats(buffer, *bufferSize, values, numValues);
      break;
    case STRING_TYPE:
      *numValues = 0;
      break;
    case LIST_TYPE:
      MAXListToStringAndFloats(buffer, *bufferSize, values, numValues); 
      break;
    default: 
      *bufferSize = 0;
      *numValues  = -1;
    }

 return true;  
}

void MAXRequestListener::ReturnRequestedValues(const char* label, float* values, int numValues)
{
  if(numValues > 0)
    {
      if((maxType != FLOAT_TYPE) && (maxType != INT_TYPE))
	{
	  int numCharsInOutBuffer = 0;	  
	  stringAndFloatsToMAXMixedMessage(label, values, numValues, outData, sizeof(outData), &numCharsInOutBuffer);
	  socket->Send(outData, numCharsInOutBuffer);
	}
    }
}

