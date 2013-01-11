#ifndef __UGA_MEDIA_STREAMER__
#define __UGA_MEDIA_STREAMER__ 

#include "../Communication/Communication.h"

class MediaStreamer
{
 public:
  virtual ~MediaStreamer(){};
  virtual bool Start(char* ip, int port) = 0;
  virtual void Stop() = 0;
};

#include "JitterStreamers/JitterCameraStreamer.h"
#include "JitterStreamers/JitterMicrophoneStreamer.h"
#include "JitterStreamers/JitterSpeakerStreamer.h"
#include "JitterStreamers/JitterLineInStreamer.h"

#endif //__UGA_MEDIA_STREAMER__
