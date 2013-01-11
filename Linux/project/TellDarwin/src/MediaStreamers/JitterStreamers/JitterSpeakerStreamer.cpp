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

const double TWO_PI = M_PI * 2;

bool sinewave(void* __this, audioSample_t* buffer, int numFrames, int numChannels, int frameRate)
{
  static double phase;
  double frequency = 1000;
  int i, j;
  for(i=0; i<numFrames*numChannels; i+= numChannels)
    {
      while(phase > 0) phase -= TWO_PI;
      buffer[i] = sin(phase);
      for(j=1; j<numChannels; buffer[i+j++] = 0);      
      phase += frequency * TWO_PI / (double)frameRate;
    }
  return true;
}


bool checkStreamFormat(jitterMatrix_t matrix)
{
  bool result = true;

  if(!JitterMatrix::isValid(matrix))
  {
    if(Global::verbosity >= 1)
      fprintf(stderr, "JitterSpeakerStreamer.cpp: Audio stream misaligned, try again\n");
      fprintf(stderr, "I will try to fix this condition later (I can be bribed). -MK\n");
    result = false; 
  }
  else if(JitterMatrix::getDataType(matrix) != JitterMatrix::FLOAT32_DATA_TYPE)
    {
      if(Global::verbosity >= 1)
        fprintf(stderr, "JitterSpeakerStreamer.cpp: only 32-bit floating point streams are supported\n");
      result = false;
    }
  else if(JitterMatrix::getNumDimensions(matrix) != 1)
   {
      if(Global::verbosity >= 1)
        fprintf(stderr, "JitterSpeakerStreamer.cpp: only one dimensional streams are supported (ie. no video, only audio)\n");
      result = false;    
   }
  else if(JitterMatrix::getNumPlanes(matrix) != 1)
  {
    if(Global::verbosity >= 1)
      fprintf(stderr, "JitterSpeakerStreamer.cpp: Only mono audio is supported (because the robot only has 1 speaker),\n");
    result = false;    
  }
  else if(JitterMatrix::getStride(matrix, 0) !=  JitterMatrix::bytesPerSample(matrix) * JitterMatrix::getNumPlanes(matrix))
   {
     if(Global::verbosity >= 1)
       fprintf(stderr, "JitterSpeakerStreamer.cpp: stream not supported, samples must be packed together with no padding between\n");
     result = false;    
   }
  return result;
}

bool speakerCallback(void* __this, audioSample_t* outBuffer, int numFrames, int numChannels, int frameRate)
{
  JitterSpeakerStreamer* _this = (JitterSpeakerStreamer*) __this;
  int numBytesToTransfer, numBytesTransferred = 0;

  int numFramesLeftToFillInOutBuffer = numFrames;
  audioSample_t* p = outBuffer;

  while(numFramesLeftToFillInOutBuffer > 0)
    {
      while(_this->numFramesLeftToReadInMatrix == 0)
        {
          numBytesToTransfer = JITTER_MATRIX_SIZE;
            while(numBytesToTransfer > 0)
              {
                numBytesTransferred = _this->tcp.Receive(&(_this->matrix[JITTER_MATRIX_SIZE - numBytesToTransfer]), numBytesToTransfer);
                if(numBytesTransferred <= 0) 
                  {
                    if(Global::verbosity >= 2)
                      fprintf(stderr, "JitterSpeakerStreamer.cpp: Stream Terminated by user while reading header\n");
                    return false;
                  }
                else numBytesToTransfer -= numBytesTransferred;
            }                     
          //if(!JitterMatrix::isValid(_this->jitterHeader))
          //  realignData(_this);
          JitterMatrix::timeStampDataReceived(_this->timestamp, _this->matrix);
          _this->numFramesLeftToReadInMatrix = JitterMatrix::getWidth(_this->matrix);
         
          if(!checkStreamFormat(_this->matrix)) return false; 
        }

      //int numInChannels = JitterMatrix::getNumPlanes(_this->matrix);
      numBytesToTransfer = (_this->numFramesLeftToReadInMatrix < numFramesLeftToFillInOutBuffer) ? 
                            _this->numFramesLeftToReadInMatrix : numFramesLeftToFillInOutBuffer;
      numBytesToTransfer *= sizeof(audioSample_t) * numChannels; 
      numBytesTransferred = _this->tcp.Receive(p, numBytesToTransfer);
      //if(numBytesTransferred != numBytesToTransfer) return false;
      if(numBytesTransferred <= 0)
        {
          if(Global::verbosity >= 2)
            fprintf(stderr, "JitterSpeakerStreamer.cpp: Stream Terminated by user while reading header\n");
          return false;
        }

      numBytesTransferred /= (sizeof(audioSample_t) * numChannels);    //numFramesTransferred;
      p += numBytesTransferred;                                        //numFramesTransferred;
      _this->numFramesLeftToReadInMatrix -= numBytesTransferred;       //numFramesTransferred;
      numFramesLeftToFillInOutBuffer     -= numBytesTransferred;       //numFramesTransferred;
    }

    p = outBuffer;
    int i;
    
    for(i=0; i<numFrames * numChannels; i++)
      {
        *((int*)p) = htonl(*((int*)p));
        p++;
      }
    
  JitterMatrix::timeStampReadyToSend(_this->timestamp);
  numBytesTransferred = _this->tcp.Send(_this->timestamp, JITTER_TIMESTAMP_SIZE);
  //if(numBytesTransferred != JITTER_TIMESTAMP_SIZE) {perror("Oh Crap 3"); } //return false;} //Don't care
  
  return true; 
}

void tcpShouldStopWaitingSigHandler(int sig)
{
  if(sig == SIGUSR1)
    return;   
}

void* startOnAnotherThreadBecauseTCPConnectBlocks(void* __this)
{
  JitterSpeakerStreamer* _this = (JitterSpeakerStreamer*)__this;
  signal(SIGUSR1, tcpShouldStopWaitingSigHandler);
  
  if(Global::verbosity >= 1) 
    fprintf(stderr, "JitterSpeakerStreamer.cpp::Awaiting TCP Connection...\n");
  _this->awaitingTCPConnection = true;
  if(_this->tcp.Connect(_this->tcpPort))
      _this->audioDevice->StartSpeaker(speakerCallback, _this, 1, 44100);
    
  _this->awaitingTCPConnection = false;
  
  return NULL;
}

JitterSpeakerStreamer::JitterSpeakerStreamer()
{
  audioDevice = Audio::GetInstance();
  numFramesLeftToReadInMatrix = 0;
  awaitingTCPConnection = false;
}

JitterSpeakerStreamer::~JitterSpeakerStreamer()
{
  Stop();
  Audio::Release();
}

bool JitterSpeakerStreamer::Start(char* ip, int port)
{
  tcpPort = port;
  if( (!audioDevice->SpeakerIsRunning()) && (!awaitingTCPConnection) )
  {
    pthread_create(&tcpListenThread, NULL, startOnAnotherThreadBecauseTCPConnectBlocks, this);
    pthread_detach( tcpListenThread);
  }
  return true;
}

void JitterSpeakerStreamer::Stop()
{
  if(awaitingTCPConnection)
  {
    pthread_kill(tcpListenThread, SIGUSR1);
    //pthread_cancel(tcpListenThread);
    pthread_join  (tcpListenThread, NULL);
    awaitingTCPConnection = false;
  }
  tcp.Disconnect(); //kill blocking processes
  audioDevice->StopSpeaker();
}


