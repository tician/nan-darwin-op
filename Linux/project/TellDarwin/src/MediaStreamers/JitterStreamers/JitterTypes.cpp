/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include <stdint.h>
#include "JitterTypes.h"
#include <stdio.h> //for debugging only

void JitterMatrix::initialize(jitterMatrix_t matrix, int numDimensions, int dimensions[], dataType_t type, int numPlanes)
{
  int i;
  for(i=0; i<JITTER_MATRIX_SIZE; matrix[i++]=0);
  matrix[  0] = 'J';
  matrix[  1] = 'M';
  matrix[  2] = 'T';
  matrix[  3] = 'X';
  matrix[  4] =  32;
  matrix[  5] =   1;
  matrix[  8] = 'X';
  matrix[  9] = 'T';
  matrix[ 10] = 'M';
  matrix[ 11] = 'J';
  matrix[ 14] =   1;
  matrix[ 15] =  32;
  
  setNumDimensions(matrix, numDimensions);
  for(i=0; i<numDimensions; i++)
    setSize(matrix, dimensions[i], i);
  setDataType(matrix, type);
  setNumPlanes(matrix, numPlanes);
  calculatePackedStrides(matrix);
  calculateNumDataBytes(matrix);
  setTime(matrix);
}

bool JitterMatrix::isValid(jitterMatrix_t matrix)
{
  return ( (matrix[ 0] == 'J') &&
           (matrix[ 1] == 'M') &&
           (matrix[ 2] == 'T') &&
           (matrix[ 3] == 'X') &&
           (matrix[ 4] ==  32) &&
           (matrix[ 5] ==   1) &&
           (matrix[ 6] ==   0) &&
           (matrix[ 7] ==   0) &&
           (matrix[ 8] == 'X') &&
           (matrix[ 9] == 'T') &&
           (matrix[10] == 'M') &&
           (matrix[11] == 'J') &&
           (matrix[12] ==   0) &&
           (matrix[13] ==   0) &&
           (matrix[14] ==   1) &&
           (matrix[15] ==  32)  )
  ? true : false;
}

int JitterMatrix::getWidth(jitterMatrix_t matrix)
{
  return JitterMatrix::getSize(matrix, 0);
}

int JitterMatrix::getHeight(jitterMatrix_t matrix)
{
  return JitterMatrix::getSize(matrix, 1);
}

int JitterMatrix::getSize(jitterMatrix_t matrix, int dimension)
{
  int result = 0;
  if((dimension >= 0) && (dimension <= 31))
  {
    int i = 4 * dimension + 28;
    result |= matrix[i + 0] << 24;
    result |= matrix[i + 1] << 16;
    result |= matrix[i + 2] <<  8;
    result |= matrix[i + 3]      ;
  }
  return result;
}

int JitterMatrix::getNumDimensions(jitterMatrix_t matrix)
{
  return matrix[27];
}

int JitterMatrix::getNumPlanes(jitterMatrix_t matrix)
{
  return matrix[19];
}

int JitterMatrix::getDataNumBytes (jitterMatrix_t matrix)
{
  int result = 0             ;
  result |= matrix[284] << 24;
  result |= matrix[285] << 16;
  result |= matrix[286] <<  8;
  result |= matrix[287]      ;
  return result              ;
}

JitterMatrix::dataType_t JitterMatrix::getDataType(jitterMatrix_t matrix)
{
  return (JitterMatrix::dataType_t)matrix[23];
}

int JitterMatrix::getStride(jitterMatrix_t matrix, int dimension)
{
  int result = 0;
  if((dimension >= 0) && (dimension <= 31))
  {
    int i = 4 * dimension + 156;
    result |= matrix[i + 0] << 24;
    result |= matrix[i + 1] << 16;
    result |= matrix[i + 2] <<  8;
    result |= matrix[i + 3]      ;
  }
  return result;
}

void JitterMatrix::setWidth(jitterMatrix_t matrix, int value)
{
  JitterMatrix::setSize(matrix, value, 0); 
}

void JitterMatrix::setHeight(jitterMatrix_t matrix, int value)
{
  JitterMatrix::setSize(matrix, value, 1);
}

void JitterMatrix::setSize(jitterMatrix_t matrix, int value, int dimension)
{
  if((dimension >= 0) && (dimension <= 31))
  {
    int i = 4 * dimension + 28;
    matrix[i+0] = (value & 0xFF000000) >> 24;
    matrix[i+1] = (value & 0x00FF0000) >> 16;
    matrix[i+2] = (value & 0x0000FF00) >>  8;
    matrix[i+3] = (value & 0x000000FF);
  }
}

void JitterMatrix::setNumDimensions(jitterMatrix_t matrix, int value)
{
  if((value >= 0) && (value <= 32))
    matrix[27] = value;
}
void JitterMatrix::setNumPlanes    (jitterMatrix_t matrix, int value)
{
 if((value >= 0) && (value <= 32))
    matrix[19] = value;
}

void JitterMatrix::setDataNumBytes (jitterMatrix_t matrix, int value)
{
  matrix[284] = (value & 0xFF000000) >> 24;
  matrix[285] = (value & 0x00FF0000) >> 16;
  matrix[286] = (value & 0x0000FF00) >>  8;
  matrix[287] = (value & 0x000000FF);
}

void JitterMatrix::setDataType     (jitterMatrix_t matrix, dataType_t type) 
{
  if((type >= 0) && (type <= 3))
    matrix[23] = type;
}

void JitterMatrix::setStride(jitterMatrix_t matrix, int value, int dimension)
{
 if((dimension >= 0) && (dimension <= 31))
  {
    int i = 4 * dimension + 156;
    matrix[i+0] = (value & 0xFF000000) >> 24;
    matrix[i+1] = (value & 0x00FF0000) >> 16;
    matrix[i+2] = (value & 0x0000FF00) >>  8;
    matrix[i+3] = (value & 0x000000FF);
  } 
} 

void JitterMatrix::calculatePackedStrides(jitterMatrix_t matrix)
{
  int i;
  int stride = bytesPerSample(matrix);
  stride *= getNumPlanes(matrix);

  for(i=0; i<getNumDimensions(matrix); i++)
  {
    setStride(matrix, stride, i);
    stride *= getSize(matrix, i);
  }
}

void JitterMatrix::calculateNumDataBytes(jitterMatrix_t matrix)
{
  int i;
  int size = bytesPerSample(matrix) * getNumPlanes(matrix);
  for(i=0; i<getNumDimensions(matrix); size *= getSize(matrix, i++));
  setDataNumBytes(matrix, size);
}

int JitterMatrix::bytesPerSample(jitterMatrix_t matrix)
{
  int type = JitterMatrix::getDataType(matrix);
  int result = 0; 
  switch(type)
    { 
      case CHAR_DATA_TYPE   : result = 1; break; 
      case LONG_DATA_TYPE   : result = 4; break;
      case FLOAT32_DATA_TYPE: result = 4; break;
      case FLOAT64_DATA_TYPE: result = 8; break;
      default: break;     
    }
  return result;
}

void JitterMatrix::timeStampDataReceived(jitterTimestamp_t timestamp, jitterMatrix_t matrix)
{
  timestamp[ 0] = 'J';
  timestamp[ 1] = 'M';
  timestamp[ 2] = 'L';
  timestamp[ 3] = 'P';
  timestamp[ 4] = matrix[288];
  timestamp[ 5] = matrix[289];
  timestamp[ 6] = matrix[290];
  timestamp[ 7] = matrix[291];
  timestamp[ 8] = matrix[292];
  timestamp[ 9] = matrix[293];
  timestamp[10] = matrix[294];
  timestamp[11] = matrix[295];

  putTimeIntoBuffer(timestamp + 12);
}
void JitterMatrix::timeStampReadyToSend(jitterTimestamp_t timestamp)
{
  putTimeIntoBuffer(timestamp + 20);
}

double JitterMatrix::getTime(jitterMatrix_t matrix)
{
  return getTimeFromBuffer(matrix + 288);
}

void JitterMatrix::setTime(jitterMatrix_t matrix)
{
  putTimeIntoBuffer(matrix + 288);
}

double JitterMatrix::getOriginTime(jitterTimestamp_t timestamp)
{
  return getTimeFromBuffer(timestamp + 4);
}

double JitterMatrix::getReceivedTime(jitterTimestamp_t timestamp)
{
  return getTimeFromBuffer(timestamp + 12);
}

double JitterMatrix::getSentTime(jitterTimestamp_t timestamp)
{
  return getTimeFromBuffer(timestamp + 20);
}

double JitterMatrix::getLatency (jitterTimestamp_t timestamp)
{
  double latency;
  latency =  getCurrentTime(         ) - getOriginTime  (timestamp);
  latency += getSentTime   (timestamp) - getReceivedTime(timestamp);
  latency /= 2.0;
  return latency;
}

void JitterMatrix::putTimeIntoBuffer(unsigned char buffer[8])
{
  *((double*)buffer) = getCurrentTime();
  hostToNetworkDouble((double*)buffer);
}

double JitterMatrix::getTimeFromBuffer(unsigned char buffer[8])
{
  double result;
  unsigned char* p = (unsigned char*)(&result);
  p[0] = buffer[0];
  p[1] = buffer[1];
  p[2] = buffer[2];
  p[3] = buffer[3];
  p[4] = buffer[4];
  p[5] = buffer[5];
  p[6] = buffer[6];
  p[7] = buffer[7];
  networkToHostDouble(&result);
 
  return result;
}

double JitterMatrix::getCurrentTime()
{
  struct timeval time;
  gettimeofday(&time, NULL);
  
  /*subtract the current time to prevent overflow*/
  time.tv_sec -= 1309144300;

  double result =  (double)(time.tv_sec  * 1000.0);
         result += (double)(time.tv_usec / 1000.0);
 
  return result;
}

void JitterMatrix::hostToNetworkDouble(double* value)
{
  int test=1;
  if(*((char*)(&test)))  //little endian
    {
      char extra;
      char *p = (char*)(value);
      extra = p[0];
      p[0]  = p[7];
      p[7]  = extra;
      extra = p[1];
      p[1]  = p[6];
      p[6]  = extra;
      extra = p[2];
      p[2]  = p[5];
      p[5]  = extra;
      extra = p[3];
      p[3]  = p[4];
      p[4]  = extra;
    }
}

void JitterMatrix::networkToHostDouble (double* value)
{
  hostToNetworkDouble(value);
}
