/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_JITTER_CAMERA_STREAMER__
#define __UGA_JITTER_CAMERA_STREAMER__

#define ZEEB_ZOB 1

#include  <stdlib.h>

#include "LinuxDARwIn.h"
using namespace Robot;
#include "JitterTypes.h"

void* videoThreadRunLoop(void* __this);  

class JitterCameraStreamer : public MediaStreamer
{
 private:
  TCPCommunication   tcp;
  jitterMatrix_t     jitterHeader;
  jitterTimestamp_t  timestamp;
  double             latency;
  int                numTimestampsMeasured;
  unsigned char*     jitterData;
  pthread_t          thread;
  atomicBool_t       threadIsRunning;
  atomicBool_t       threadShouldContinueRunning;

  int     width;
  int     height;
  float   frameRate;
  bool    nonreentrantCaptureFrame();
  void    setSize(int _width, int _height);

  friend void* videoThreadRunLoop(void* __this);
  
 protected:

 public:
   JitterCameraStreamer();
  ~JitterCameraStreamer();


  bool Start(char* ip, int port);
  void Stop()                   ;
};


#endif//__UGA_JITTER_CAMERA_STREAMER__
