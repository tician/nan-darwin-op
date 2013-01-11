/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_JITTER_SPEAKER_STREAMER__
#define __UGA_JITTER_SPEAKER_STREAMER__ 1

#include "JitterTypes.h"
#include "../Audio/Audio.h"

bool microphoneCallback(void* __this, audioSample_t* buffer, int numFrames, int numChannels, int frameRate);
void* startOnAnotherThreadBecauseTCPConnectBlocks(void* __this);

class JitterSpeakerStreamer : public MediaStreamer
{
 private:
  Audio*            audioDevice;
  TCPCommunication  tcp;
  atomicBool_t      awaitingTCPConnection;
  int               tcpPort;
  pthread_t         tcpListenThread;
  jitterTimestamp_t timestamp;
  jitterMatrix_t    matrix;
  int numFramesLeftToReadInMatrix;
  friend bool  speakerCallback(void* __this, audioSample_t* buffer, int numFrames, int numChannels, int frameRate);
  friend void* startOnAnotherThreadBecauseTCPConnectBlocks(void* __this);
 protected:
  
 public:
  JitterSpeakerStreamer(); 
 ~JitterSpeakerStreamer();

  bool Start(char* ip, int port);
  void Stop();
};


#endif//__UGA_JITTER_SPEAKER_STREAMER__
