/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_JITTER_LINE_IN_STREAMER___
#define __UGA_JITTER_LINE_IN_STREAMER__

#include "JitterTypes.h"
#include "../Audio/Audio.h"

bool lineInCallback(void* __this, audioSample_t* buffer, int numFrames, int numChannels, int frameRate);

class JitterLineInStreamer : public JitterMicrophoneStreamer
{
 private:
 protected:
  friend bool lineInCallback(void* __this, audioSample_t* buffer, int numFrames, int numChannels, int frameRate);
 public:
  JitterLineInStreamer();
 ~JitterLineInStreamer();
  bool Start(char* ip, int port);
  void Stop();
};


#endif//__UGA_JITTER_LINE_IN_STREAMER__
