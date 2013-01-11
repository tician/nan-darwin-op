/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_JITTER_MICROPHONE_STREAMER___
#define __UGA_JITTER_MICROPHONE_STREAMER__

#include "JitterTypes.h"
#include "../Audio/Audio.h"

bool microphoneCallback(void* __this, audioSample_t* buffer, int numFrames, int numChannels, int frameRate);

class JitterMicrophoneStreamer : public MediaStreamer
{
 private:

 protected:
  Audio* audioDevice;
  TCPCommunication  tcp;
  jitterMatrix_t    jitterHeader;
  jitterTimestamp_t timestamp;
  double latency;
  int numTimestampsMeasured;
  friend bool microphoneCallback(void* __this, audioSample_t* buffer, int numFrames, int numChannels, int frameRate);

 public:
  JitterMicrophoneStreamer();
 ~JitterMicrophoneStreamer();
  bool Start(char* ip, int port);
  void Stop();
};


#endif//__UGA_JITTER_MICROPHONE_STREAMER__
