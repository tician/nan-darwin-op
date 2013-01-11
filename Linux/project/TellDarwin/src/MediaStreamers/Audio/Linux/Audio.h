/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_AUDIO__
#define __UGA_AUDIO__ 1

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>

#define NUM_BUFFERS        4
#define BUFFER_NUM_FRAMES  1024

//#define MICROPHONE_DEVICE_NAME "plughw:CARD=U0x46d0x80a"
#define MICROPHONE_DEVICE_NAME "plughw:1,0"
#define SPEAKER_DEVICE_NAME    "plughw:0,0"
#define LINE_IN_DEVICE_NAME    "plughw:2,0" //this is actually a usb device, not the line in.

//NB: MICROPHONE does not use this format. It uses int16_t, and does type-punning.
//This was a last minute fix because the hardware does not support floats
//and I can't figure out how to get ALSA to convert.
typedef float audioSample_t;
typedef bool (*audioCallback_t)(void* __this, audioSample_t* buffer, int numFrames, int numChannels, int frameRate);

void* microphoneThreadRunLoop (void* __this);
void* speakerThreadRunLoop    (void* __this);
void* lineInThreadRunLoop     (void* __this);

class Audio
{
 public:
  typedef enum
  {
    //used to index arrays.
    MICROPHONE = 0,
    SPEAKER    = 1,
    LINE_IN    = 2,
    NUM_DEVICES   ,
  }deviceIndex_t;
 
 private:
  typedef            char                       atomicBool_t;
  snd_pcm_t         *device                     [NUM_DEVICES];
  pthread_t          thread                     [NUM_DEVICES];
  atomicBool_t       threadIsRunning            [NUM_DEVICES];
  atomicBool_t       threadShouldContinueRunning[NUM_DEVICES];
  void*             (*threadRunLoop             [NUM_DEVICES])(void*); 
  audioCallback_t    callback                   [NUM_DEVICES];
  void*              callbackArgument           [NUM_DEVICES];
  unsigned int       numChannels                [NUM_DEVICES]; 
  unsigned int       frameRate                  [NUM_DEVICES];
  unsigned long int  bufferNumFrames            [NUM_DEVICES]; 
  audioSample_t     *sampleBuffer               [NUM_DEVICES];

  /*class variables*/
  static Audio* singleton;
  static int  retainCount;
  

  friend void* microphoneThreadRunLoop(void* __this);  
  friend void* speakerThreadRunLoop   (void* __this); 
  friend void* lineInThreadRunLoop    (void* __this);
  
  void audioDeviceThreadRunLoop(deviceIndex_t deviceIndex);

  bool transferData        (deviceIndex_t deviceIndex, snd_pcm_sframes_t (*transfer)(snd_pcm_t*, void*, snd_pcm_uframes_t));
  bool startDevice(deviceIndex_t deviceIndex, audioCallback_t callback, void* callbackArgument, unsigned int numChannels, unsigned int frameRate);
  void stopDevice(deviceIndex_t deviceIndex);
  void closeDevice(deviceIndex_t deviceIndex);

  Audio();
 ~Audio(); 		
 protected:

 public:
  static Audio* GetInstance()
  {
    if(singleton == NULL)
      singleton = new Audio;
    retainCount++;
    return singleton;
  }

  static void Release()
  {
    if(singleton != NULL)
      {
        retainCount--;
        if(retainCount <= 0)
          {
            delete singleton;
            singleton = NULL;
          }      
      }
  }
  
  bool StartMicrophone(audioCallback_t callback, void* callbackArgument, unsigned int numChannels, unsigned int frameRate);
  bool StartSpeaker   (audioCallback_t callback, void* callbackArgument, unsigned int numChannels, unsigned int frameRate);
  bool StartLineIn    (audioCallback_t callback, void* callbackArgument, unsigned int numChannels, unsigned int frameRate);
  void StopMicrophone ( );
  void StopSpeaker    ( );
  void StopLineIn     ( );
  bool MicrophoneIsRunning(){ return threadIsRunning[MICROPHONE]; }
  bool SpeakerIsRunning   (){ return threadIsRunning[SPEAKER   ]; }
  bool LineInIsRunning    (){ return threadIsRunning[LINE_IN   ]; }
};

#endif//__UGA_AUDIO__
