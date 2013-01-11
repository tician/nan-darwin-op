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

bool lineInCallback(void* __this, audioSample_t* buffer, int numFrames, int numChannels, int frameRate)
{
  bool success = true;
  int numBytesTransferred, numBytesToSend;
  JitterLineInStreamer* _this = (JitterLineInStreamer*)__this;

  JitterMatrix::initialize(_this->jitterHeader, 1, &numFrames, JitterMatrix::FLOAT32_DATA_TYPE, numChannels);
  numBytesTransferred = _this->tcp.Send(_this->jitterHeader, JITTER_MATRIX_SIZE);
  if(numBytesTransferred != JITTER_MATRIX_SIZE) {success = false; goto cleanup;}
  numBytesToSend = numChannels * numFrames * sizeof(*buffer);
  numBytesTransferred = _this->tcp.Send(buffer, numBytesToSend);
  //printf("%f ", *(buffer) );
  //if(numBytesTransferred != numBytesToSend) {success = false; goto cleanup;}
  if(numBytesTransferred <= 0) {success = false; goto cleanup;}

  numBytesTransferred = _this->tcp.Receive((char*)_this->timestamp, JITTER_TIMESTAMP_SIZE);
  
  //get latency
  if(Global::verbosity >= 1)
    if(numBytesTransferred == JITTER_TIMESTAMP_SIZE)  
      {
        if(_this->numTimestampsMeasured == 100)
          fprintf(stderr, "JitterLineInStreamer.cpp: network latency is %lf milliseconds (average over 100 samples)\n",
                         _this->latency / 100.0);
        if(_this->numTimestampsMeasured <= 100)
          {
            _this->latency += JitterMatrix::getLatency(_this->timestamp); 
            _this->numTimestampsMeasured++;
          }
      }
    
 cleanup: 
  return success;
}


JitterLineInStreamer::JitterLineInStreamer()
{
    latency = numTimestampsMeasured = 0;
    audioDevice = Audio::GetInstance();
}

JitterLineInStreamer::~JitterLineInStreamer()
{
  Stop();
  Audio::Release();
}

bool JitterLineInStreamer::Start(char* ip, int port)
{
  bool result = false;
  if(audioDevice->LineInIsRunning())
     result = true;
  else
    if(tcp.Connect(ip, port))
      result = audioDevice->StartLineIn(microphoneCallback, this, 2, 44100);

  return result;
}

void JitterLineInStreamer::Stop()
{
  audioDevice->StopLineIn();
  tcp.Disconnect();
}


