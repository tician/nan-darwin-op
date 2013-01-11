/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include "../MediaStreamers.h"
#include <math.h>

namespace Global{extern int verbosity;}

//samples come as 16 bit signed ints (not audioSample_t) because hardware does not support floats
bool microphoneCallback(void* __this, audioSample_t* buffer, int numFrames, int numChannels, int frameRate)
{
  bool success = true;
  int numBytesTransferred, numBytesToSend;
  JitterMicrophoneStreamer* _this = (JitterMicrophoneStreamer*)__this;
  int16_t* p = (int16_t*)buffer;


  audioSample_t* floatBuffer = (audioSample_t*)malloc(sizeof(*floatBuffer) * numFrames * numChannels);
  if(floatBuffer == NULL) return false; 
  audioSample_t* f = floatBuffer;

  //swap byte order  
 
  int i, j=numFrames * numChannels;
  for(i=0; i<j; i++)
  {
      *f = *p / (double)0x7FFE;
      *(int*)f = htonl(*(int*)f);
      p++; f++;
  }

  JitterMatrix::initialize(_this->jitterHeader, 1, &numFrames, JitterMatrix::FLOAT32_DATA_TYPE, numChannels);
  numBytesTransferred = _this->tcp.Send(_this->jitterHeader, JITTER_MATRIX_SIZE);
  if(numBytesTransferred != JITTER_MATRIX_SIZE) {success = false; goto cleanup;}
  numBytesToSend = numChannels * numFrames * sizeof(*buffer);
  numBytesTransferred = _this->tcp.Send(floatBuffer, numBytesToSend);
  //if(numBytesTransferred != numBytesToSend) {success = false; goto cleanup;}
  if(numBytesTransferred <= 0) {success = false; goto cleanup;}

  numBytesTransferred = _this->tcp.Receive((char*)_this->timestamp, JITTER_TIMESTAMP_SIZE);
  
  //get latency
  if(Global::verbosity >= 1)
    if(numBytesTransferred == JITTER_TIMESTAMP_SIZE)  
      {
        if(_this->numTimestampsMeasured == 100)
          fprintf(stderr, "JitterMicrophoneStreamer.cpp: network latency is %lf milliseconds (average over 100 samples)\n",
                         _this->latency / 100.0);
        if(_this->numTimestampsMeasured <= 100)
          {
            _this->latency += JitterMatrix::getLatency(_this->timestamp); 
            _this->numTimestampsMeasured++;
          }
      }
    
 cleanup: 
  if(floatBuffer != NULL) free(floatBuffer); 
  return success;
}

JitterMicrophoneStreamer::JitterMicrophoneStreamer()
{
    latency = numTimestampsMeasured = 0;
    audioDevice = Audio::GetInstance();
}

JitterMicrophoneStreamer::~JitterMicrophoneStreamer()
{
  Stop();
  Audio::Release();
}

bool JitterMicrophoneStreamer::Start(char* ip, int port)
{
  bool result = false;
  if(audioDevice->MicrophoneIsRunning())
     result = true;
  else
    if(tcp.Connect(ip, port))
      result = audioDevice->StartMicrophone(microphoneCallback, this, 1, 44100);

  return result;
}

void JitterMicrophoneStreamer::Stop()
{
  audioDevice->StopMicrophone();
  tcp.Disconnect();
}


