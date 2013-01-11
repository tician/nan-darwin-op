/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include "Audio.h"

namespace Global{extern int verbosity;}

Audio *Audio::singleton = NULL;
int    Audio::retainCount = 0;

/*main thread only*/
Audio::Audio()
{
  int i;
  for(i=0; i<NUM_DEVICES; i++)
    {
      threadIsRunning    [i] = false;
      device             [i] = NULL;
      callback           [i] = NULL ;
      sampleBuffer[i]        = NULL;
    }
  
  threadRunLoop[MICROPHONE] = microphoneThreadRunLoop; 
  threadRunLoop[SPEAKER   ] = speakerThreadRunLoop;
  threadRunLoop[LINE_IN   ] = lineInThreadRunLoop; 
}

/*main thread only*/
Audio::~Audio()
{
  int i;
  for(i=0; i<NUM_DEVICES; i++)
    stopDevice((deviceIndex_t)i); 
}

bool Audio::startDevice(deviceIndex_t deviceIndex, audioCallback_t callback, void* callbackArgument, unsigned int numChannels, unsigned int frameRate)
{
  using Global::verbosity;
  int error=0;
  unsigned long int internalBufferNumFrames;

  snd_pcm_hw_params_t  *hardwareParameters;
  snd_pcm_hw_params_alloca(&hardwareParameters);

  this->callbackArgument[deviceIndex] = callbackArgument;
  this->callback[deviceIndex] = callback;

 if(threadIsRunning[deviceIndex])
    {
      if(verbosity >= 1) 
        fprintf(stderr, "Audio: device is alreay running\n"); 
      return true;
    }

  if((error >= 0) && device[deviceIndex] == NULL)
    {
      if(deviceIndex == MICROPHONE)
        {
          error = snd_pcm_open(&device[MICROPHONE], MICROPHONE_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0);
          if(error < 0) 
            {
              if(verbosity >= 1) 
                fprintf(stderr, "Audio.cpp: Unable to open microphone device: %s\n", snd_strerror(error));
              device[MICROPHONE] = NULL;
            }
        }
      else if(deviceIndex == SPEAKER)
        {
          error = snd_pcm_open(&device[SPEAKER], SPEAKER_DEVICE_NAME, SND_PCM_STREAM_PLAYBACK, 0);
          if(error < 0) 
            {
              if(verbosity >= 1)
                fprintf(stderr, "Audio.cpp: Unable to open speaker device: %s\n", snd_strerror(error));
              device[SPEAKER] = NULL;
            } 
        }
      else if(deviceIndex == LINE_IN)
        {
          error = snd_pcm_open(&device[LINE_IN], LINE_IN_DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0);
          if(error < 0) 
            {
              if(verbosity >= 1)
                fprintf(stderr, "Audio.cpp: Unable to open line-in device: %s\n", snd_strerror(error));
              device[LINE_IN] = NULL;
            } 
        }
    }

  if(error >= 0)
    {
      error = snd_pcm_hw_params_any(device[deviceIndex], hardwareParameters);
      if((error < 0) && (verbosity >= 1)) 
        fprintf(stderr, "Audio.cpp: Unable to get a generic hardware configuration: %s\n", snd_strerror(error));
    }

  if(error >= 0)
    {
      error = snd_pcm_hw_params_set_access(device[deviceIndex], hardwareParameters, SND_PCM_ACCESS_RW_INTERLEAVED);
      if((error < 0) && (verbosity >= 1))
        fprintf(stderr, "Audio.cpp: Device does not support interleaved audio access: %s\n", snd_strerror(error));
    }

  if(error >= 0)
    {
      error = ((deviceIndex == MICROPHONE) || (deviceIndex == LINE_IN)) ? 
	snd_pcm_hw_params_set_format(device[deviceIndex], hardwareParameters, SND_PCM_FORMAT_S16  ) : 
        snd_pcm_hw_params_set_format(device[deviceIndex], hardwareParameters, SND_PCM_FORMAT_FLOAT) ;
      if((error < 0) && (verbosity >= 1))
	fprintf(stderr, "Audio.cpp: Unable to set sample format: %s\n", snd_strerror(error));
    }
  
  if(error >= 0)
    {
      this->numChannels[deviceIndex] = numChannels;
      error = snd_pcm_hw_params_set_channels_near(device[deviceIndex], hardwareParameters, &(this->numChannels[deviceIndex]));
      if((error < 0) && (verbosity >= 1)) 
        fprintf(stderr, "Audio.cpp: unable to set the number of channels: %s\n", snd_strerror(error));
      else if((numChannels != this->numChannels[deviceIndex]) && (verbosity >= 1))
        fprintf(stderr, "Audio.cpp: device does not support %i numChannels, %i will be used instead\n", 
                numChannels, this->numChannels[deviceIndex]);  
    }
  
  if(error >= 0)
    {
      this->frameRate[deviceIndex] = frameRate;
      error = snd_pcm_hw_params_set_rate_near(device[deviceIndex], hardwareParameters, &(this->frameRate[deviceIndex]), 0);
      if((error < 0) && (verbosity >= 1))  
        fprintf(stderr, "Audio.cpp: Unable to set the frame rate: %s\n", snd_strerror(error));
      else if((frameRate != this->frameRate[deviceIndex]) && (verbosity >= 1))
        fprintf(stderr, "Audio.cpp: device does not support %i frame rate, %i will be used instead\n", 
                frameRate, this->frameRate[deviceIndex]);
    }

  if(error >= 0)
    {
      int dir = 0;
      bufferNumFrames[deviceIndex] = BUFFER_NUM_FRAMES;
      error = snd_pcm_hw_params_set_period_size_near(device[deviceIndex], hardwareParameters, &bufferNumFrames[deviceIndex], &dir);
      if((error < 0) && (verbosity >= 1)) 
          fprintf(stderr, "Audio.cpp: Unable to set the sample buffer size: %s\n", snd_strerror(error));
      else if((bufferNumFrames[deviceIndex] != BUFFER_NUM_FRAMES) && (verbosity >= 1))
        fprintf(stderr, "Audio.cpp: device does not support %i period size, %lu will be used instead\n", 
                BUFFER_NUM_FRAMES, bufferNumFrames[deviceIndex]);
    }
  
  if(error >= 0)
    {
      internalBufferNumFrames = bufferNumFrames[deviceIndex] * NUM_BUFFERS;
      error = snd_pcm_hw_params_set_buffer_size_near(device[deviceIndex], hardwareParameters, &internalBufferNumFrames);
      if((error < 0) && (verbosity >= 1)) 
          fprintf(stderr, "Unable to set the internal buffer size: %s\n", snd_strerror(error));
      else if(internalBufferNumFrames != bufferNumFrames[deviceIndex] * NUM_BUFFERS)
        if((verbosity >= 1))
          fprintf(stderr, "Audio.cpp: device does not support %lu internal buffer size, %lu will be used instead\n", 
                  bufferNumFrames[deviceIndex] * NUM_BUFFERS, internalBufferNumFrames);
    }

  if(error >= 0)
    {
      error = snd_pcm_hw_params(device[deviceIndex], hardwareParameters);
      if((error < 0) && (verbosity >= 1)) 
        fprintf(stderr, "Audio.cpp: Unable to load the hardware parameters into the device: %s\n", snd_strerror(error));
    }

   if(error >= 0)
    {
      int size = sizeof(audioSample_t) * numChannels * bufferNumFrames[deviceIndex];
      if(deviceIndex == MICROPHONE) size /= 2;
  
      if(sampleBuffer[deviceIndex] == NULL)
        sampleBuffer[deviceIndex] = (audioSample_t*)malloc(size);
      if(sampleBuffer[deviceIndex] == NULL)
        {
           error = -1;
           if(verbosity >= 1) fprintf(stderr, "Audio.cpp: Unable to allocate audio buffers \n");
        }
    }

   if(error >= 0)
     {      
       error = pthread_create(&(thread[deviceIndex]), NULL, threadRunLoop[deviceIndex], this);
       if(error != 0)
	 {
	   if(verbosity >= 1) perror("Audio.cpp: error creating thread");
	   error = -1;
	 }    
     }
   
   bool result = (error < 0) ? false : true;
   if(result == false) closeDevice(deviceIndex);
   
   return result; 
}

void Audio::closeDevice(deviceIndex_t deviceIndex)
{
  if(device[deviceIndex] != NULL)
    {
      snd_pcm_close(device[deviceIndex]);
      device[deviceIndex] = NULL;
    } 
  if(sampleBuffer[deviceIndex] != NULL)
    {
      free(sampleBuffer[deviceIndex]);
      sampleBuffer[deviceIndex] = NULL;
    }
}

void Audio::stopDevice(deviceIndex_t deviceIndex)
{
  threadShouldContinueRunning[deviceIndex] = false;
  if(threadIsRunning[deviceIndex])
    pthread_join(thread[deviceIndex], NULL);
          
}

bool Audio::StartMicrophone(audioCallback_t callback, void* callbackArgument, unsigned int numChannels, unsigned int frameRate)
{
  return startDevice(MICROPHONE, callback, callbackArgument, numChannels, frameRate);
}

bool Audio::StartSpeaker   (audioCallback_t callback, void* callbackArgument, unsigned int numChannels, unsigned int frameRate)
{
  return startDevice(SPEAKER, callback, callbackArgument, numChannels, frameRate);
}

bool Audio::StartLineIn   (audioCallback_t callback, void* callbackArgument, unsigned int numChannels, unsigned int frameRate)
{
  return startDevice(LINE_IN, callback, callbackArgument, numChannels, frameRate);
}

void Audio::StopMicrophone ( )
{
  stopDevice(MICROPHONE);
}

void Audio::StopSpeaker( )
{
  stopDevice(SPEAKER);
}

void Audio::StopLineIn( )
{
  stopDevice(LINE_IN);
}

/*reentrant once for each device*/
void Audio::audioDeviceThreadRunLoop(deviceIndex_t deviceIndex)
{
  bool success = true;
  threadIsRunning[deviceIndex] = threadShouldContinueRunning[deviceIndex] = true;
  signal(SIGPIPE, SIG_IGN);
  
  while(threadShouldContinueRunning[deviceIndex])
    {
      //Read the Microphone
      if(deviceIndex == MICROPHONE || deviceIndex == LINE_IN)
          success = transferData(deviceIndex, snd_pcm_readi); 
      
     //Call the callback
      if(success)
        success = callback[deviceIndex](callbackArgument[deviceIndex], 
                                        sampleBuffer    [deviceIndex], 
                                        bufferNumFrames [deviceIndex], 
                                        numChannels     [deviceIndex], 
                                        frameRate       [deviceIndex]);

      //Write to the speaker
      if(success)
        if(deviceIndex == SPEAKER)
          success = transferData(deviceIndex, (snd_pcm_sframes_t (*)(snd_pcm_t*, void*, snd_pcm_uframes_t)) snd_pcm_writei);

      if(!success) break;
    }
  closeDevice(deviceIndex);
  threadIsRunning[deviceIndex] = false;
}

/*reentrant once for each device*/
bool Audio::transferData(deviceIndex_t deviceIndex, snd_pcm_sframes_t (*transfer)(snd_pcm_t*, void*, snd_pcm_uframes_t))
{
  int    numFramesTransferred = 0, &error = numFramesTransferred; 
  int    numFramesLeft        = bufferNumFrames[deviceIndex];
  audioSample_t* p            = sampleBuffer   [deviceIndex];

   while((numFramesLeft > 0) && threadShouldContinueRunning)
     {
       numFramesTransferred = transfer(device[deviceIndex], p, numFramesLeft);

       if(numFramesTransferred < 0)
         {  
           if(Global::verbosity >= 2) 
             fprintf(stderr, "Audio.cpp: audio device error: %s, attempting to recover... ", snd_strerror(error));
           switch(error)
            {
              case -EPIPE:   //overflow / underflow
                snd_pcm_wait(device[deviceIndex], 100);
                //printf("Number of Available Frames: %i\n", error = snd_pcm_avail(device[deviceIndex]));  
                //if(error < 0) printf("\terror: %s\n", snd_strerror(error));              
                if      ((deviceIndex == MICROPHONE) || (deviceIndex == LINE_IN) ) usleep(5000);      //underrun, wait for more data
                else if (deviceIndex == SPEAKER   ) 
                  {
                    if((error = snd_pcm_avail(device[deviceIndex])) < 0)   //broken pipe
                      usleep(10000);                                       //wait for more samples to come
                    else numFramesLeft = 0;                                //overrun, skip remaining samples;
                  }

                error = snd_pcm_prepare(device[deviceIndex]); 
                break;
             
             case -ESTRPIPE: 
              while(((error = snd_pcm_resume(device[deviceIndex])) == -EAGAIN) && threadShouldContinueRunning[deviceIndex]) 
                sleep(1);
              if(error == -ENOSYS) error = snd_pcm_prepare(device[deviceIndex]); 
              break;
             
           }
           if(error < 0)
             {
               if(Global::verbosity >= 2) fprintf(stderr, "Aborting\n");
               this->threadShouldContinueRunning[deviceIndex] = false;
               break;
             }
           else
             {
               if(Global::verbosity >= 2) fprintf(stderr, "Okay\n");
               numFramesTransferred = 0;
             } 
         }
        p +=  (deviceIndex==MICROPHONE) ? numFramesTransferred * numChannels[deviceIndex] / 2 :  numFramesTransferred * numChannels[deviceIndex];
        numFramesLeft -= numFramesTransferred;              
     } 
  return (numFramesLeft == 0) ? true : false;     
}

void* microphoneThreadRunLoop (void* __this)
{
  if(Global::verbosity >= 1) printf("microphone thread begin\n");
  ((Audio*)__this)->audioDeviceThreadRunLoop(Audio::MICROPHONE);
  if(Global::verbosity >= 1) printf("microphone thread end\n");
  return NULL;
}

void* speakerThreadRunLoop    (void* __this)
{
  if(Global::verbosity >= 1) printf("speaker thread begin\n");
  ((Audio*)__this)->audioDeviceThreadRunLoop(Audio::SPEAKER);
  if(Global::verbosity >= 1) printf("speaker thread end\n");
  return NULL;
}

void* lineInThreadRunLoop    (void* __this)
{
  if(Global::verbosity >= 1) printf("line in thread begin\n");
  ((Audio*)__this)->audioDeviceThreadRunLoop(Audio::LINE_IN);
  if(Global::verbosity >= 1) printf("line in thread end\n");
  return NULL;
}

