/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_JITTER_TYPES__
#define __UGA_JITTER_TYPES__

#include <stdlib.h>
#include <sys/time.h>


#define JITTER_MATRIX_SIZE    296
#define JITTER_TIMESTAMP_SIZE  28
typedef unsigned char jitterMatrix_t   [JITTER_MATRIX_SIZE   ];
typedef unsigned char jitterTimestamp_t[JITTER_TIMESTAMP_SIZE]; 
typedef char          atomicBool_t;

namespace JitterMatrix
{
  typedef enum
  {
    CHAR_DATA_TYPE    = 0,
    LONG_DATA_TYPE    = 1,
    FLOAT32_DATA_TYPE = 2,
    FLOAT64_DATA_TYPE = 3,
  }dataType_t;
  
  void initialize(jitterMatrix_t matrix, int numDimensions, int dimensions[], dataType_t type, int numPlanes);
  bool            isValid         (jitterMatrix_t matrix); 
  int             getWidth        (jitterMatrix_t matrix);
  int             getHeight       (jitterMatrix_t matrix);
  int             getSize         (jitterMatrix_t matrix, int dimension);
  int             getNumDimensions(jitterMatrix_t matrix);
  int             getNumPlanes    (jitterMatrix_t matrix);
  int             getDataNumBytes (jitterMatrix_t matrix); 
  dataType_t      getDataType     (jitterMatrix_t matrix);
  int             getStride       (jitterMatrix_t matrix, int dimension);

  void            setWidth        (jitterMatrix_t matrix, int value);
  void            setHeight       (jitterMatrix_t matrix, int value);
  void            setSize         (jitterMatrix_t matrix, int value, int dimension);
  void            setNumDimensions(jitterMatrix_t matrix, int value);
  void            setNumPlanes    (jitterMatrix_t matrix, int value);
  void            setDataNumBytes (jitterMatrix_t matrix, int value);
  void            setDataType     (jitterMatrix_t matrix, dataType_t type); 
  void            setStride       (jitterMatrix_t matrix, int value, int dimension); 

  void            calculatePackedStrides(jitterMatrix_t matrix);
  void            calculateNumDataBytes (jitterMatrix_t matrix);
  int             bytesPerSample        (jitterMatrix_t matrix);

  void            timeStampDataReceived(jitterTimestamp_t timestamp, jitterMatrix_t matrix);
  void            timeStampReadyToSend (jitterTimestamp_t timestamp);
  
  double          getTime(jitterMatrix_t matrix);
  void            setTime(jitterMatrix_t matrix); 

  double          getOriginTime  (jitterTimestamp_t timestamp); 
  double          getReceivedTime(jitterTimestamp_t timestamp);
  double          getSentTime    (jitterTimestamp_t timestamp);  
  double          getLatency     (jitterTimestamp_t timestamp);  

  double          getCurrentTime      (                       );
  void            putTimeIntoBuffer   (unsigned char buffer[8]);
  double          getTimeFromBuffer   (unsigned char buffer[8]);
  void            networkToHostDouble (double* value);
  void            hostToNetworkDouble (double* value);
}

#endif//__UGA_JITTER_TYPES__
