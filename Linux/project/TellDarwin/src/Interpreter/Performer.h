/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_PERFORMER__
#define __UGA_PERFORMER__ 1

#include <string.h>
#include <math.h>
#include "../Darwin.h"
using namespace Robot;

#ifndef _RX_28M_H_
#define RX28M MX28
#endif

class Performer
{
 public:
 typedef enum
 {
   //used to index arrays
   ID_R_HAND     = 21,
   ID_L_HAND     = 22,
   ID_R_ARM_YAW  = 23,
   ID_L_ARM_YAW  = 24,
 }extraServosIDs_t;
 
  typedef enum
  {
    //these are used to index arrays
    //so dont change the values
    RED_COMPONENT   = 0,
    GREEN_COMPONENT = 1,
    BLUE_COMPONENT  = 2,
  }colorComponent_t;

  typedef enum
  {
    LED_ID_EYE      = CM730::P_LED_EYE_L ,
    LED_ID_FOREHEAD = CM730::P_LED_HEAD_L,
  }LED_ID_t;
  
  typedef enum
  {
    EAR_ID_R = CM730::P_RIGHT_MIC_L,
    EAR_ID_L = CM730::P_LEFT_MIC_L,
  }earID_t;

  typedef enum
  {
    //the accelerometer is mounted sideways.
    ACCEL_X = CM730::P_ACCEL_X_L,
    ACCEL_Y = CM730::P_ACCEL_Z_L,
    ACCEL_Z = CM730::P_ACCEL_Y_L,
  }accelComponent_t;

  typedef enum
  {
  //the Gyroscope is mounted sideways.
    GYRO_X = CM730::P_GYRO_Y_L,
    GYRO_Y = CM730::P_GYRO_Z_L,
    GYRO_Z = CM730::P_GYRO_X_L,
  }gyroComponent_t;

  typedef enum
  {
    FSR_RIGHT = FSR::ID_R_FSR,
    FSR_LEFT  = FSR::ID_L_FSR,
  }FSRID_t;

  typedef enum
  {
    //too many unrelated coordinate systems
    FSR_CENTER_X  = FSR::P_FSR_X,
    FSR_CENTER_Z  = FSR::P_FSR_Y,
  }FSRComponent_t;
    
  typedef enum
  {
    CAMERA_BRIGHTNESS = V4L2_CID_BRIGHTNESS,
    CAMERA_CONTRAST   = V4L2_CID_CONTRAST  ,
    CAMERA_SATURATION = V4L2_CID_SATURATION,
    CAMERA_GAIN       = V4L2_CID_GAIN      ,
    CAMERA_EXPOSURE   = V4L2_CID_EXPOSURE  ,
  }cameraProperty_t;
  
 private:
  CM730* cm730;
  LinuxCM730* linuxcm730;
  LinuxMotionTimer* motionTimer;
  
  bool isWalking; 
  static Performer* m_UniqueInstance;

  void extractColorComponents(int color, float components[3]);
  int makeColor(float components[3]);
  int getColor(LED_ID_t ID, int* value);
  Performer();
  void checkLimit(int ID, float* value);
  void fixLeftServoAngle(int ID, float *angle);
  int handleServoError(int ID);
  int servoExists(int ID);  
  int willInterefereWithWalking(int ID);
 public:

  bool isEnabled;
  static Performer* GetInstance() { return m_UniqueInstance; }
    
  ~Performer();

  bool Initialize();

  int KillRightElbow();

  /* call Get/SetServoSomething for several consecutive servos       */
  /* on call *numValues should be the number of elements in values[] */  
  /* on return it will be the number of values actually written      */ 
  int GetBodySomething(float values[], int* numValues, int (Performer::*GetServoSomething)(int, float*));
  /* if numValues is negative, all servos set to values[0]  */
  int SetBodySomething(float values[], int  numValues, int (Performer::*SetServoSomething)(int, float ));

  /* Degrees                             */
  int GetServoAngle(int ID,  float* angle);   
  int SetServoAngle(int ID,  float  angle);

  /* 0 to 326 Degrees / Second           */
  int GetServoSpeed(int ID,  float* speed);
  int SetServoSpeed(int ID,  float  speed);

  /* 0 to disable, !0 to enable          */
  int GetServoEnable(int ID,  float* enable);
  int SetServoEnable(int ID,  float  enable);

  /* X, Y, Z, about -4 to 4 (9.81*Meters)/(Second*Second)         */
  int GetAccelComponent(accelComponent_t component, float* value   , int numSamples);
  int GetAccel         (                            float  value[3], int numSamples);

  /* X, Y, Z, ? to ? Degrees/Second                               */
  int GetGyroComponent (gyroComponent_t  component, float* value   , int numSamples);
  int GetGyro          (                            float  value[3], int numSamples);

  /* R, G, B, 0 to 1 arbitrary units                                              */
  int SetLEDColorComponent(LED_ID_t ID, colorComponent_t component, float  value   );
  int GetLEDColorComponent(LED_ID_t ID, colorComponent_t component, float *value   );
  int GetLEDColor         (LED_ID_t ID, float  components[3]);
  int SetLEDColor         (LED_ID_t ID, float  components[3]);
  
  int SetCameraProperty  (cameraProperty_t property, float* value);
  int GetCameraProperty  (cameraProperty_t property, float* value);
  
  /* L, R  0 to 1 arbitrary units                    */
  int GetEar              (earID_t ID,  float* value   );
  int GetEars             (             float  value[2]); 
    
    
  void   StartWalking          (           );
  void   StopWalking           (           );
  /* -45 to 45 Degrees per step (negative is left)  */
  void   SetWalkingAngle       (float  value);
  void   GetWalkingAngle       (float* value);
  /* -23 to 23 Centimeters / Second         */
  void   SetWalkingVelocityX   (float  value); 
  void   GetWalkingVelocityX   (float* value); 
  void   SetWalkingVelocityY   (float  value); 
  void   GetWalkingVelocityY   (float* value); 
  void   SetWalkingVelocityZ   (float  value); 
  void   GetWalkingVelocityZ   (float* value); 
  /*milliseconds? default is 600            */
  void   GetWalkingPeriod      (float* value);
  void   SetWalkingPeriod      (float  value);
  
  void   GetWalkingBalance     (float* value);
  void   SetWalkingBalance     (float  value);
  /* on call *numValues should be the number of elements in values[], which must be at least 3 */
  /* on return it will be 3 for head, rightArm and leftArm, respectively  */
  void   GetWalkingEnable      (float* value, int* numValues);
  void   SetWalkingEnable      (bool head, bool rightArm, bool leftArm);
  
  /* 0.493~65.535 Newtons. Order: front medial, front lateral, rear lateral, rear medial */
  int GetFSR (FSRID_t ID, float value[4], int numSamples);
  /* -1 to 1 arbitrary units. positive is lateral or front */
  /* samples not under load are not averaged in, 0 returned if no sample is under load  */
  int GetFSRComponent(FSRID_t ID, FSRComponent_t component, float* value, int numSamples);

  void Speak(char* string);
};


#endif //__UGA_PERFORMER__
