/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_MAX_TYPES__
#define __UGA_MAX_TYPES__ 1

#include <stdlib.h>
#include <sys/time.h>

namespace MAX
{
  typedef enum 
    {
      INT_TYPE     , 
      FLOAT_TYPE   ,
      STRING_TYPE  ,
      BANG_TYPE    ,
      LIST_TYPE    ,
      MIXED_TYPE   ,
 
      UNKNOWN_TYPE,
    }MAXType_t;

  MAXType_t   determineMAXType(                 char* buffer, int bufferSize                             );
  void   floatToMAXFloat    (float       value, char  buffer[16]                                         );
  void   intToMAXInt        (int         value, char  buffer[12]                                         );
  float  MAXFloatToFloat    (                   char  buffer[16]                                         );      
  int    MAXIntToInt        (                   char  buffer[12]                                         );
  float  MAXStringToFloat   (                   char* buffer, int bufferSize                             );
  int    MAXStringToInt     (                   char* buffer, int bufferSize                             );
  void   stringToMAXString  (const char* value, char* buffer, int bufferSize, int* numCharsAlreadyWritten);
  void   intToMAXString     (int         value, char* buffer, int bufferSize, int *numCharsAlreadyWritten);
  void   addEndToMAXString  (                   char* buffer, int bufferSize, int *numCharsAlreadyWritten);
  void   intToChars         (int         value, char* buffer, int bufferSize, int *numCharsAlreadyWritten);

  void   formatToStringAndFloats(char* inData, int numChars, int numCharsRead, char *outData, float values[], int* numValues);
  void   MAXMixedMessageToStringAndFloats(char* inData, int numChars, float values[], int* numValues);
  void   stringAndFloatsToMAXMixedMessage(const char* string, float* values, int numValues, char* buffer, int   bufferSize, int *numCharsAlreadyWritten);
  void   MAXListToStringAndFloats(char* buffer,  int bufferSize, float values[], int* numValues);

  int   MAX4BytesToInt  (char bytes[4]);
  float MAX4BytesToFloat(char bytes[4]);
}

#endif//__UGA_MAX_TYPES__
