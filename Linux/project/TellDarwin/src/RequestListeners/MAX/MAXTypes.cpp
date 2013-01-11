/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>
#include "MAXTypes.h"
#include <time.h>

#include "stdio.h" //for debugging only

void MAX::floatToMAXFloat(float value, char buffer[16])
{
  int numCharsAlreadyWritten = 0;
  stringAndFloatsToMAXMixedMessage("float", &value, 1, (char*)buffer, 16, &numCharsAlreadyWritten);
}

void MAX::intToMAXInt(int value, char buffer[12])
{
  int32_t v = htonl(value);
  char*   b = (char*)&v; 

  buffer[0]  = 'i'   ;
  buffer[1]  = 'n'   ;
  buffer[2]  = 't'   ;
  buffer[3]  =  0    ;
  buffer[4]  = ','   ;
  buffer[5]  = 'i'   ;
  buffer[6]  =  0    ;
  buffer[7]  =  0    ;
  buffer[8]  =  b[0] ;
  buffer[9]  =  b[1] ;
  buffer[10] =  b[2] ;
  buffer[11] =  b[3] ;
}

float  MAX::MAXFloatToFloat  (char  buffer[16])
{
  int value = *((int *)(&(buffer[12])));
  value = ntohl(value);
  return *((float*)(&value));
} 
     
int    MAX::MAXIntToInt      (char  buffer[12])
{
  int value = *((int *)(&(buffer[8])));
  return ntohl(value);
}

void MAX::intToMAXString(int value, char* buffer, int bufferSize, int* numCharsAlreadyWritten)
{
  if(bufferSize < 8) return;
  intToChars(value, buffer, bufferSize-4, numCharsAlreadyWritten);
  addEndToMAXString(buffer, bufferSize,   numCharsAlreadyWritten);
}

void MAX::addEndToMAXString(char* buffer, int bufferSize, int *numCharsAlreadyWritten)
{
  memset(buffer+(*numCharsAlreadyWritten), 0, bufferSize - *numCharsAlreadyWritten);
  *numCharsAlreadyWritten += 4;
  *numCharsAlreadyWritten += 4-((*numCharsAlreadyWritten)%4);
  if(*numCharsAlreadyWritten > bufferSize)
    *numCharsAlreadyWritten = bufferSize;
  buffer[(*numCharsAlreadyWritten) - 4] = ',';
}

void MAX::stringToMAXString(const char* value, char* buffer, int bufferSize, int* numCharsAlreadyWritten)
{
  char *b = buffer;
  while((*value != '\0') && (*numCharsAlreadyWritten <= bufferSize))
    {
      *b++ = *value++;
      (*numCharsAlreadyWritten)++;
    }
  addEndToMAXString(buffer, bufferSize, numCharsAlreadyWritten);
}

void MAX::intToChars(int value, char* buffer, int bufferSize, int *numCharsAlreadyWritten)
{
  int n;
  if(value < 0)
    { 
      buffer[(*numCharsAlreadyWritten)++] = '-';
      value = -value;
    }
  if( (n=value/10) !=0 )
      intToChars(n, buffer, bufferSize, numCharsAlreadyWritten);
    
  if( (bufferSize - *numCharsAlreadyWritten) > 0)
    buffer[(*numCharsAlreadyWritten)++] = value % 10 + '0';
}

void MAX::MAXMixedMessageToStringAndFloats(char* inData, int numChars, float values[], int* numValues)
{
  int numCharsRead = 0;
  char *outData = inData; 
  
  numCharsRead = strlen(inData);
  if(numCharsRead < numChars)
    {
      outData += numCharsRead;
      *outData ++ = ' ';
      numCharsRead += 4 - (numCharsRead % 4);
    }
  formatToStringAndFloats(inData, numChars, numCharsRead, outData, values, numValues);
}

//this could share code with the above but it is not worth the effort
void  MAX::MAXListToStringAndFloats(char* inData, int numChars, float values[], int* numValues)
{ 
  int numCharsRead = 8; //"list\0\0\0\0"
  formatToStringAndFloats(inData, numChars, numCharsRead, inData, values, numValues);
}

//there is guaranteed to be enough space in outData because of how this is called...
void MAX::formatToStringAndFloats(char* inData, int numChars, int numCharsRead, char *outData, float values[], int* numValues)
{
  int maxNumValues = *numValues;
  *numValues = 0;
  int numFormatSpecifiers, i, j, length;
  char *format = NULL;
  
  //save format specifiers
  numFormatSpecifiers = strlen(inData + numCharsRead);

  if(numCharsRead + numFormatSpecifiers <= numChars)
    {
      format = (char*)malloc(numFormatSpecifiers);
      if(format == NULL) {printf("stack overflow\n"); return;}
      for(i=0; i<numFormatSpecifiers; format[i++] = inData[numCharsRead++]);
      numCharsRead += 4 - (numCharsRead % 4);
    }
  else
    {
      fprintf(stderr, "MaxTypes.cpp: List format error\n");
      numFormatSpecifiers = 0;
    }
   
  int numStrings = 0;
   //the first specifier is a comma
   for(i=1; i<numFormatSpecifiers; i++)
     {
     switch((int)format[i])
       {
	case (int)'i':
	  if(numCharsRead+4 <= numChars)
	    {
	      if(*numValues < maxNumValues)
		values[(*numValues)++] = (float)MAX4BytesToInt(&inData[numCharsRead]);
	      numCharsRead += 4;
	    }
	    break;

	case (int)'f':
	  if(numCharsRead+4 <= numChars)
	    {
	      if(*numValues < maxNumValues)
		values[(*numValues)++] = MAX4BytesToFloat(&inData[numCharsRead]);
	      numCharsRead += 4;
	    }
	  break;

	case (int)'s':
	  if(numCharsRead + (int)strlen(&inData[numCharsRead]) < numChars)
	    {
              numStrings ++;
	      if((*outData != ' ') && (numStrings != 1)) *outData++ = ' ';
	      length = strlen(&inData[numCharsRead]);
	      for(j=0; j<length; *outData++ = inData[numCharsRead++], j++);
	      numCharsRead += 4 - (numCharsRead % 4);
	    }
	  break;
       }   
   }
  *outData = '\0'; 
  if(format != NULL) free(format);
}

MAX::MAXType_t MAX::determineMAXType(char* buffer, int bufferSize)
{
  MAXType_t type;

  if(      (bufferSize  ==  12) &&
	   (buffer[0]   == 'i') &&
	   (buffer[1]   == 'n') &&
	   (buffer[2]   == 't') &&
	   (buffer[3]   ==  0 ) &&
	   (buffer[4]   == ',') &&
	   (buffer[5]   == 'i') &&
	   (buffer[6]   ==  0 ) &&
	   (buffer[7]   ==  0 )  )
    type = INT_TYPE;

  else if( (bufferSize  ==  16) &&
	   (buffer[ 0]  == 'f') &&
	   (buffer[ 1]  == 'l') &&
	   (buffer[ 2]  == 'o') &&
	   (buffer[ 3]  == 'a') &&
	   (buffer[ 4]  == 't') &&
	   (buffer[ 5]  ==  0 ) &&
	   (buffer[ 6]  ==  0 ) &&
	   (buffer[ 7]  ==  0 ) &&
	   (buffer[ 8]  == ',') &&
	   (buffer[ 9]  == 'f') &&
	   (buffer[10]  ==  0 ) &&
	   (buffer[11]  ==  0 )  )
    type = FLOAT_TYPE;

  else if( (bufferSize  ==  12) &&
	   (buffer[ 0]  == 'b') &&
	   (buffer[ 1]  == 'a') &&
	   (buffer[ 2]  == 'n') &&
	   (buffer[ 3]  == 'g') &&
	   (buffer[ 4]  ==  0 ) &&
	   (buffer[ 5]  ==  0 ) &&
	   (buffer[ 6]  ==  0 ) &&
	   (buffer[ 7]  ==  0 ) &&
	   (buffer[ 8]  == ',') &&
	   (buffer[ 9]  ==  0 ) &&
	   (buffer[10]  ==  0 ) &&
	   (buffer[11]  ==  0 )  )
    type = BANG_TYPE;

  else if( (bufferSize  >=  8 ) &&  
	   (buffer[ 0]  == 'l') &&
	   (buffer[ 1]  == 'i') &&
	   (buffer[ 2]  == 's') &&
	   (buffer[ 3]  == 't') &&
	   (buffer[ 4]  ==  0 ) &&
	   (buffer[ 5]  ==  0 ) &&
	   (buffer[ 6]  ==  0 ) &&
	   (buffer[ 7]  ==  0 ) &&
	   (buffer[ 8]  == ',')  )
    type = LIST_TYPE;

  else if( (bufferSize    >= 8 )            &&  
	   (bufferSize %4 == 0 )            &&
	   (buffer[bufferSize - 4]  == ',') && 
	   (buffer[bufferSize - 3]  ==  0 ) &&
	   (buffer[bufferSize - 2]  ==  0 ) &&
	   (buffer[bufferSize - 1]  ==  0 )  )  
    type = STRING_TYPE;

  else if( (bufferSize    >= 8 )  &&  
	   (bufferSize %4 == 0 )   )
    type = MIXED_TYPE;


  else
    type = UNKNOWN_TYPE;

  return type;
}

int MAX::MAX4BytesToInt(char bytes[4])
{
  int value = *((int *)(bytes));
  return ntohl(value);
}

float MAX::MAX4BytesToFloat(char* bytes)
{
  int value = *((int *)(bytes));
  value =  ntohl(value);
  return *((float*)(&value));
}

void MAX::stringAndFloatsToMAXMixedMessage(const char* string, float* values, int numValues, char* buffer, int bufferSize, int *numCharsAlreadyWritten)
{
  int i, j, numZeros;
  char* b;
  int32_t v;

  stringToMAXString(string, buffer, bufferSize, numCharsAlreadyWritten);

  *numCharsAlreadyWritten -= 3;

  for(i=0; (i<numValues) && (*numCharsAlreadyWritten <= bufferSize ); i++)
    buffer[(*numCharsAlreadyWritten)++] = 'f';

  numZeros = 4 -(*numCharsAlreadyWritten % 4);
  for(i=0; (i<numZeros) && (*numCharsAlreadyWritten <= bufferSize ); i++)
    buffer[(*numCharsAlreadyWritten)++] = 0;

  for(i=0; (i<numValues) && (*numCharsAlreadyWritten <= bufferSize ); i++)
    {
      v = htonl( *((int*)(&(values[i]))) );
      b = (char*)&v; 
      
      for(j=0; j<4; j++)
	buffer[(*numCharsAlreadyWritten)++] =  b[j]; //no pun intended
    }
}


