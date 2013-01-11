/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include "../MediaStreamers.h"

namespace Global{extern int verbosity; extern bool robotMode;}

JitterCameraStreamer::JitterCameraStreamer()
{
  threadIsRunning = false;
  threadShouldContinueRunning = false;
  jitterData = NULL;
  frameRate = 24;
  numTimestampsMeasured = latency = 0;
  setSize(Camera::WIDTH, Robot::Camera::HEIGHT);
}

JitterCameraStreamer::~JitterCameraStreamer()
{
  Stop();
  
  if(jitterData != NULL)
    delete jitterData;
}

bool JitterCameraStreamer::Start(char* ip, int port)
{
  bool result = false;
  if(threadIsRunning)
    result = true;
  else if(tcp.Connect(ip, port))
      {
        pthread_create(&thread, NULL, videoThreadRunLoop, this);
        result = true;
      }
  return result;
}

void JitterCameraStreamer::Stop()
{
  if(threadIsRunning)
    {
      threadShouldContinueRunning = false;
      pthread_join(thread, NULL);
      tcp.Disconnect();
    }
}

void JitterCameraStreamer::setSize(int width, int height)
{
  int dimensions[2];
  if(!threadIsRunning)
    {
      if(width  > 720) width  = 720;
      if(height > 720) height = 720;
      if(width  <  10) width  =  10;
      if(height <  10) height =  10;
  
      dimensions[0] = this->width  = width;
      dimensions[1] = this->height =height;      

      JitterMatrix::initialize(jitterHeader, 2, dimensions, JitterMatrix::CHAR_DATA_TYPE, 4);
 
      if(jitterData != NULL)
	delete jitterData;    
      jitterData = new unsigned char[this->width * this->height * 4];
    }
}

void* videoThreadRunLoop(void* __this)
{
  JitterCameraStreamer* _this = (JitterCameraStreamer*)__this;
  _this->threadIsRunning = _this->threadShouldContinueRunning = true;
  if(Global::verbosity >= 1)fprintf(stderr, "video thread begin\n");

  int numBytesToTransfer;
  int numBytesTransferred;
  int sleepDuration = 20000/(_this->frameRate + 0.5);

  signal(SIGPIPE,SIG_IGN);
  
  while(_this->threadShouldContinueRunning)
    {
      if(!_this->nonreentrantCaptureFrame()) break;
      JitterMatrix::setTime(_this->jitterHeader);
      numBytesTransferred = _this->tcp.Send(_this->jitterHeader, JITTER_MATRIX_SIZE);
      if(numBytesTransferred < JITTER_MATRIX_SIZE) break;
      numBytesToTransfer = (_this->width) * (_this->height) * 4;
      numBytesTransferred = _this->tcp.Send(_this->jitterData, numBytesToTransfer);
      if(numBytesToTransfer < numBytesToTransfer)  break;
      numBytesTransferred = _this->tcp.Receive(_this->timestamp, JITTER_TIMESTAMP_SIZE);
      
      if(Global::verbosity >= 1)    
        if(numBytesTransferred == JITTER_TIMESTAMP_SIZE)
          {
            if(_this->numTimestampsMeasured == 100)
              fprintf(stderr, "JitterCameraStreamer.cpp: network latency is %lf milliseconds (average over 100 samples)\n", 
                      _this->latency / 100.0);
            if(_this->numTimestampsMeasured <= 100)
              {
                _this->latency += JitterMatrix::getLatency(_this->timestamp); 
                _this->numTimestampsMeasured++;
              }
          }
      
      usleep(sleepDuration);
    }
  
  if(Global::verbosity >= 1) printf("video thread end\n");
  _this->threadIsRunning = false;
  return NULL;
}

bool JitterCameraStreamer::nonreentrantCaptureFrame()
{ 
  if(Global::robotMode)
  {
    int i;
    int w = LinuxCamera::GetInstance()->fbuffer->m_RGBFrame->m_Width;
    int h = LinuxCamera::GetInstance()->fbuffer->m_RGBFrame->m_Height;
    unsigned char *b = LinuxCamera::GetInstance()->fbuffer->m_RGBFrame->m_ImageData;

    if((h != height) || (w !=width))
      {
        if(Global::verbosity <= 1)
          fprintf(stderr, "JitterVideo.cpp: error copying video frame\n");
        return false;
      }
    else
      {
        LinuxCamera::GetInstance()->CaptureFrame();
        for(i=0; i<width*height; i++) 
	  {
	    jitterData[i*4+0] = 0xFF;
	    jitterData[i*4+1] = b[i*3+0];
	    jitterData[i*4+2] = b[i*3+1];
	    jitterData[i*4+3] = b[i*3+2];
  	  }
      }
  }
  
  else  // !Global::robotMode
  {
    /*assume there is no camera and send a test-pattern*/
    int i, x, y;
    for(i=0; i<width*height*4; i+=4)
      {
        x = (i/4) % width;
        y = (i-x) / (width * 4);
        jitterData[i+0] = 0x00; 
        jitterData[i+1] = ((random() % 127) + 127) * y / height;
        jitterData[i+2] = ((random() % 127) + 127) * x / width;
        jitterData[i+3] = 255 - ((random() % 127) + 127) * y / height; 
      } 
  }

  return true;
}
