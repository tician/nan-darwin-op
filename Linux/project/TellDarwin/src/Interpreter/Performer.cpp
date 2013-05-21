/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include "Performer.h"
#include "Kinematics.h"

#include <stdio.h>

//#define degreesToRadians(x) ((x) / 57.295779513082322)
//#define radiansToDegrees(x) ((x) * 57.295779513082322)

namespace Global {extern int verbosity;}

#define SHOULDER_ROLL_OFFSET 45
#define ELBOW_OFFSET 90

using namespace Robot;

const float minAngle[] = 
{
  -0  ,  
  -180,  //ID_R_SHOULDER_PITCH
  -180,  //ID_L_SHOULDER_PITCH
  -75 ,  //ID_R_SHOULDER_ROLL 
  -129,  //ID_L_SHOULDER_ROLL 
  -94 ,  //ID_R_ELBOW         
  -66 ,  //ID_L_ELBOW         
  -30 ,  //ID_R_HIP_YAW       
  -50 ,  //ID_L_HIP_YAW            
  -5  ,  //ID_R_HIP_ROLL      
  -57 ,  //ID_L_HIP_ROLL      
  -96 ,  //ID_R_HIP_PITCH     
  -29 ,  //ID_L_HIP_PITCH
   0  ,  //ID_R_KNEE          
  -132,  //ID_L_KNEE          
  -51 ,  //ID_R_ANKLE_PITCH   
  -79 ,  //ID_L_ANKLE_PITCH   
  -37 ,  //ID_R_ANKLE_ROLL    
  -57 ,  //ID_L_ANKLE_ROLL    
  -120,  //ID_HEAD_PAN
  -22 ,  //ID_HEAD_TILT       
  -90 ,  //ID_R_HAND
  -90 ,  //ID_L_HAND
  0,    //ID_R_WRIST_YAW
  0,    //ID_L_WRIST_YAW
};

const float maxAngle[] = 
{
  0  ,  //nothing with this ID
  180,  //ID_R_SHOULDER_PITCH
  180,  //ID_L_SHOULDER_PITCH
  129,  //ID_R_SHOULDER_ROLL 
  75 ,  //ID_L_SHOULDER_ROLL 
  66 ,  //ID_R_ELBOW         
  94 ,  //ID_L_ELBOW         
  50 ,  //ID_R_HIP_YAW       
  30 ,  //ID_L_HIP_YAW          
  57 ,  //ID_R_HIP_ROLL      
  5  ,  //ID_L_HIP_ROLL
  29 ,  //ID_R_HIP_PITCH     
  96 ,  //ID_L_HIP_PITCH        
  132,  //ID_R_KNEE          
  0  ,  //ID_L_KNEE          
  79 ,  //ID_R_ANKLE_PITCH   
  51 ,  //ID_L_ANKLE_PITCH   
  57 ,  //ID_R_ANKLE_ROLL    
  37 ,  //ID_L_ANKLE_ROLL    
  120,  //ID_HEAD_PAN
  58 ,  //ID_HEAD_TILT
  20 ,  //ID_R_HAND
  20 ,  //ID_L_HAND
  180,  //ID_R_WRIST_YAW
  180,  //ID_L_WRIST_YAW
};

Performer* Performer::m_UniqueInstance = new Performer();

Performer::Performer()
{
  cm730 = NULL;
  linuxcm730 = NULL;
  motionTimer = NULL;
  isWalking = false;
}

Performer::~Performer()
{
  delete cm730;
  delete linuxcm730;
  delete motionTimer;
}

void Performer::checkLimit(int ID, float* value)
{
  int numModeratedJoints = sizeof(maxAngle) / sizeof(*maxAngle);

  if(ID < numModeratedJoints)
    {
      if(*value > maxAngle[ID])
        *value = maxAngle[ID];
      else if(*value < minAngle[ID])
        *value = minAngle[ID];
    }      
}

bool Performer::Initialize()
{
  /////////////////////////////////////////////////////////////////////
  //Initialize the Framework
  
  linuxcm730 = new LinuxCM730("/dev/ttyUSB0");
  cm730 = new CM730(linuxcm730);
  
  cm730->Connect();
  LinuxCamera::GetInstance()->Initialize(0);
  
  if(MotionManager::GetInstance()->Initialize(cm730) == false)
   {
      printf("Failed to initialize Motion Manager! \n");
      delete cm730; cm730 = NULL;
      delete linuxcm730; linuxcm730 = NULL;
      return false;
   }
    
  motionTimer = new LinuxMotionTimer(MotionManager::GetInstance());
  //LinuxMotionTimer::Initialize(MotionManager::GetInstance());
  motionTimer->Stop();

  minIni* ini = new minIni("/darwin/Data/config.ini");
  Walking::GetInstance()->LoadINISettings(ini);
  MotionManager::GetInstance()->LoadINISettings(ini);

  MotionManager::GetInstance()->SetEnable(false);
  
  return true;
  /////////////////////////////////////////////////////////////////////
}

int Performer::GetBodySomething(float values[], int* numValues, int (Performer::*GetServoSomething)(int, float*))
{
  int i = 0, error = -1;
  if(*numValues > 0)
    {
      while((this->*GetServoSomething)(i+1, &(values[i])))
        {
          if(*numValues <= ++i)
            break;
        }
    }
  *numValues = i;
  return error;  
}

int Performer::SetBodySomething(float values[], int numValues, int (Performer::*SetServoSomething)(int, float))
{
  int i = 0, next = 0, error = -1;
  if(numValues != 0)
    {
      while((this->*SetServoSomething)(i+++1, values[next++]))
        {
          if(numValues < 0)
            next = 0;
          else if(!(--numValues))
            break;
        }
    }
    return error; 
}

int Performer::servoExists(int ID)
{
  int result = false;
  if(cm730 != NULL)
    result = ((ID >= 1)&&(ID <= 20)) ? true : (cm730->Ping(ID, 0) == CM730::SUCCESS);
  return result;
}

int Performer::willInterefereWithWalking(int ID)
{
  int result = false;
    if((ID >= 1) && (ID <= JointData::NUMBER_OF_SERVOS))
      result = (isWalking && MotionStatus::m_CurrentJoints.GetEnable(ID));
  return result;
}

int Performer::GetServoAngle(int ID, float* angle)
{
  int error = -1, exists = this->servoExists(ID), value=0;
  if(exists)
    {
      cm730->ReadWord(ID, RX28M::P_PRESENT_POSITION_L, &value, &error);
      
      *angle = RX28M::Value2Angle(value);
      fixLeftServoAngle(ID, angle);
      if( (ID == JointData::ID_R_ELBOW) || (ID == JointData::ID_L_ELBOW))
	*angle += ELBOW_OFFSET;
      else if( (ID == JointData::ID_R_SHOULDER_ROLL) || (ID == JointData::ID_L_SHOULDER_ROLL))
	*angle -= SHOULDER_ROLL_OFFSET;
    }
  return exists;
}

int Performer::SetServoAngle(int ID, float angle)
{
  int error = -1, exists = this->servoExists(ID);

  if(!willInterefereWithWalking(ID))
    if(exists)
      {
        if( (ID == JointData::ID_R_ELBOW) || (ID == JointData::ID_L_ELBOW))
          angle -= ELBOW_OFFSET;
        else if( (ID == JointData::ID_R_SHOULDER_ROLL) || (ID == JointData::ID_L_SHOULDER_ROLL))
          angle += SHOULDER_ROLL_OFFSET;
              
        fixLeftServoAngle(ID, &angle);
        checkLimit(ID, &angle);
        int value = RX28M::Angle2Value(angle);

        cm730->WriteWord(ID, RX28M::P_GOAL_POSITION_L, value, &error);
      }

  return exists;
}

void Performer::fixLeftServoAngle(int ID, float* value)
{
  if( (ID == JointData::ID_L_SHOULDER_PITCH) ||
      (ID == JointData::ID_L_SHOULDER_ROLL ) ||
      (ID == JointData::ID_L_ELBOW         ) ||
      (ID == JointData::ID_L_HIP_YAW       ) ||
      (ID == JointData::ID_L_HIP_ROLL      ) ||
      (ID == JointData::ID_R_HIP_PITCH     ) ||
      (ID == JointData::ID_R_KNEE          ) ||
      (ID == JointData::ID_L_ANKLE_PITCH   ) ||
      (ID == JointData::ID_HEAD_PAN        ) ||
      (ID == JointData::ID_R_ANKLE_ROLL    ) ||
      (ID == JointData::ID_R_ANKLE_ROLL    ) ||
      (ID == JointData::ID_R_ANKLE_ROLL    ))
      {*value *=- 1;}
}

int Performer::SetServoSpeed(int ID, float speed)
{ /*hardware unit is 0.053 RPM*/
  int error = -1, exists = this->servoExists(ID), value;
  
  if(!willInterefereWithWalking(ID))
    if(exists)
      {
        value = (int)(speed *= 3.144654088);
        if(value < 1   ) value = 1   ;
        if(value > 1023) value = 0   ; //0 is special, fastest;
        cm730->WriteWord(ID, RX28M::P_MOVING_SPEED_L, value, &error);
      }
       
  return exists;
}

int Performer::GetServoSpeed(int ID, float* speed)
{
  int error =- 1, exists = this->servoExists(ID), value=0;
  if(exists)
    cm730->ReadWord(ID, RX28M::P_MOVING_SPEED_L, &value, &error);
    
  if(value == 0) value = 1023;
  *speed = ((float)value) / 3.144654088;
  return exists;
}

int Performer::GetServoEnable(int ID,  float* enable)
{
  int error=-1, exists = this->servoExists(ID), value=0;
  if(exists)
    cm730->ReadWord(ID, RX28M::P_TORQUE_ENABLE, &value, &error);
  *enable = (float) (value != 0);
  return exists;
}

int Performer::SetServoEnable(int ID,  float  enable)
{
  int error = -1, exists = this->servoExists(ID), value=(enable != 0);
  
  if(!willInterefereWithWalking(ID))
    if(exists)
      {
        if(enable) error = handleServoError(ID);
        cm730->WriteWord(ID, RX28M::P_TORQUE_ENABLE, value, &error);
      }
      
  return exists;
}

int Performer::handleServoError(int ID)
{
  int error = -1, value = 0;
  
  //ASSUME FOR NOW THAT THIS HAS ALREADY BEEN CHECKED
  //int servoExists = servoExists(ID);
  //if(servoExixts)
    //{
       static const char* text[] = {"warning", "shutdown error"};
       int reg[] = {RX28M::P_ALARM_LED, RX28M::P_ALARM_SHUTDOWN};
       int i;
       
       for(i=0; i<2; i++)
         {
           cm730->ReadByte(ID, reg[i], &value, &error); 
           if(value != 0)
             {
               if(Global::verbosity >= 1)
                 {
                   fprintf(stderr, "Performer.cpp: detetcted servo %s: %i, ", text[i], value);
                   if(value & 0x01) fprintf(stderr, "Input Voltage Error... ");
                   if(value & 0x02) fprintf(stderr, "Angle Limit Error... ");
                   if(value & 0x04) fprintf(stderr, "Overheating Error... ");
                   if(value & 0x08) fprintf(stderr, "Range Error... ");
                   if(value & 0x10) fprintf(stderr, "Checksum Error... ");
                   if(value & 0x20) fprintf(stderr, "Overload Error... ");
                   if(value & 0x40) fprintf(stderr, "Instruction Error... ");
                   if(value & 0x80) fprintf(stderr, "Unknown Error... ");
                   fprintf(stderr, "\n");
                 }
               if(i == 0)
                 {
                   cm730->WriteByte(ID, RX28M::P_ALARM_LED, 0, &error);
                 }
               else
                 {
                   cm730->WriteByte(ID, RX28M::P_ALARM_SHUTDOWN, 0, &error); 
                   //cm730->ReadWord(ID, RX28M::P_MAX_TORQUE_L, &value, &error);
                   //cm730->WriteWord(ID, RX28M::P_TORQUE_LIMIT_L, value, &error);  
                   cm730->WriteWord(ID, RX28M::P_TORQUE_LIMIT_L, 1023, &error); 
                 }  
             }
         }
    //}
  
  return error;
}

int Performer::GetAccelComponent(accelComponent_t component, float* value, int numSamples)
{  
  int error = -1, temp = 0, i;
  float thisSample;
  *value = 0;
  
  if(numSamples > 20) numSamples = 20;
  if(numSamples <  1) numSamples =  1;
  
  if(cm730 != NULL)
    if((component == ACCEL_X) || (component == ACCEL_Y) || (component == ACCEL_Z))
      for(i=0; i<numSamples; i++)
        {
          cm730->ReadWord(CM730::ID_CM, component, &temp, &error);
          thisSample = (float)temp;
          thisSample -= 512;
          thisSample *= 4/512.0;
          *value += thisSample;
        }
          
  *value /= numSamples;      

  if((component == ACCEL_X) || (component == ACCEL_Z))
    *value *= -1;

  return error;
}

int Performer::GetAccel(float  value[3], int numSamples)
{
  int error = -1;
  
  error = GetAccelComponent(ACCEL_X, &(value[0]), numSamples);
  error = GetAccelComponent(ACCEL_Y, &(value[1]), numSamples);
  error = GetAccelComponent(ACCEL_Z, &(value[2]), numSamples);

  return error;
}

int Performer::GetGyroComponent (gyroComponent_t  component, float* value, int numSamples)
{
  int error = -1, temp = 0, i;
  float thisSample;
  *value = 0;
  
  if(numSamples > 20) numSamples = 20;
  if(numSamples <  1) numSamples =  1;
  
  if(cm730 != NULL)
    if((component == GYRO_X) || (component == GYRO_Y) || (component == GYRO_Z))
      for(i=0; i<numSamples; i++)
        {
          cm730->ReadWord(CM730::ID_CM, component, &temp, &error); 
          thisSample = (float)temp;
          thisSample -= 512;
          thisSample *= -1600.0 / 512.0;
          *value += thisSample;         
        }
  *value /= numSamples;
  return error;
}

int Performer::GetGyro(float  value[3], int numSamples)
{
  int error = -1;
  //should I check error values? ... nah!
  error = GetGyroComponent(GYRO_X, &(value[0]), numSamples);
  error = GetGyroComponent(GYRO_Y, &(value[1]), numSamples);
  error = GetGyroComponent(GYRO_Z, &(value[2]), numSamples);

  return error;
}

int Performer::SetLEDColorComponent(LED_ID_t ID, colorComponent_t component, float value)
{
  int color = 0;
  float components[3] = {0, 0, 0};
  int error = getColor(ID, &color);
  extractColorComponents(color, components);
  components[component%3] = value;

  color = makeColor(components);
 
  if(error != -1)
    cm730->WriteWord(ID, color, &error);
  return error;
}

int Performer::GetLEDColor(LED_ID_t ID, float components[3])
{
  int color = 0, error;
  error = getColor(ID, &color);
  extractColorComponents(color, components);
  return error;
}

int Performer::GetLEDColorComponent(LED_ID_t ID, colorComponent_t component, float* value)
{
  int color = 0;
  float components[3] = {0, 0, 0};
  int error = getColor(ID, &color);
  extractColorComponents(color, components);
  *value = components[component % 3];
  return error;
}

int Performer::SetLEDColor(LED_ID_t ID, float components[3])
{
  int error = -1;
  int color = makeColor(components);
  if(cm730 != NULL)
    //if(isEnabled)
      if( (ID == LED_ID_EYE) || (ID == LED_ID_FOREHEAD) )
	      cm730->WriteWord(ID, color, &error);
  return error;
}

int Performer::getColor(LED_ID_t ID, int* value)
{
  int error = -1;
  if(cm730 != NULL)
    //if(isEnabled)
      if( (ID == LED_ID_EYE) || (ID == LED_ID_FOREHEAD) )
	      cm730->ReadWord(CM730::ID_CM, ID, value, &error);
  
  return error;    
}

void Performer::extractColorComponents(int color, float components[3])
{
  int r, g, b;
  b = (color >> 10) & 0x1F;
  g = (color >>  5) & 0x1F;
  r = (color >>  0) & 0x1F;

  components[0] = r / 31.0;
  components[1] = g / 31.0;
  components[2] = b / 31.0;
}

int Performer::makeColor(float components[3])
{
  int rgb[3];
  int i;
  for(i=0; i<3; i++)
    {
      if(components[i] < 0) components[i] = 0;
      if(components[i] > 1) components[i] = 1;
      components[i] *= 31;
      rgb[i] = (unsigned char)components[i];
    }

  return ( (rgb[2] << 10) | (rgb[1] << 5) | (rgb[0] << 0) );
}

int Performer::GetEar(earID_t ID,  float* value   )
{
  /*cm730 returns value of adc, 0 ~ 1023*/
  int error = -1, temp = 0;
  //if(isEnabled)
    if(cm730 != NULL)
      if((ID == EAR_ID_R) || (ID == EAR_ID_L))
        cm730->ReadWord(CM730::ID_CM, ID, &temp, &error);
    *value = ((double) temp - 511.5) / 511.5;
  return error;

}

int Performer::GetEars(float  value[2])
{
  int error;
  error = GetEar(EAR_ID_L,  &value[0]);
  error = GetEar(EAR_ID_R,  &value[1]);
  return error;
}

int Performer::SetCameraProperty  (cameraProperty_t property, float* value)
{
  int temp=0, error=0;
  if(*value > 1) *value = 1;
  if(*value < 0) *value = 0;
  
  if(property == CAMERA_EXPOSURE) temp = (int)(((double)*value * 10000) + 0.5);
  else                            temp = (int)(((double)*value *   255) + 0.5);
  LinuxCamera::GetInstance()->v4l2SetControl((int)property, temp);

  return error;
}

int Performer::GetCameraProperty  (cameraProperty_t property, float* value)
{
  int temp=0, error=0;
  
  temp = LinuxCamera::GetInstance()->v4l2GetControl((int)property);

  if(property == CAMERA_EXPOSURE) *value = (double)temp / 10000.0;
  else                            *value = (double)temp /   255.0;

  return error;
}

void Performer::StartWalking()
{
  //isEnabled = false;
  if(cm730 != NULL)
    if(!isWalking)
      {
        isWalking = true;
        MotionManager::GetInstance()->AddModule((MotionModule*)Walking::GetInstance());
        motionTimer->Start();
        MotionManager::GetInstance()->SetEnable(true);
        Walking::GetInstance()->Start();
      }
}

void Performer::StopWalking()
{
  int error = -1;
  if(cm730 != NULL)
    if(isWalking)
      {
        int timer = 100000;
        Walking::GetInstance()->Stop();
        while((Walking::GetInstance()->IsRunning()) && (timer--)) usleep(10000);
        MotionManager::GetInstance()->SetEnable(false);
        motionTimer->Stop();
        MotionManager::GetInstance()->RemoveModule((MotionModule*)Walking::GetInstance());
        //this will be correct once firmware is updated
        //incase the walking module changed them...
        cm730->WriteByte(CM730::ID_BROADCAST, RX28M::P_P_GAIN, 32, &error); 
        cm730->WriteByte(CM730::ID_BROADCAST, RX28M::P_I_GAIN, 0, &error);
        cm730->WriteByte(CM730::ID_BROADCAST, RX28M::P_D_GAIN, 0, &error); 
        isWalking = false;
      }
  //isEnabled = true;
  //return error;
}

void  Performer::SetWalkingAngle  (float value)
{
  if(value >  45) value =  45;
  if(value < -45) value = -45;
  
  Walking::GetInstance()->A_MOVE_AMPLITUDE = -value;
}

void  Performer::GetWalkingAngle(float *value)
{
  *value = -(float)Walking::GetInstance()->A_MOVE_AMPLITUDE;
}


void   Performer::SetWalkingVelocityX   (float  value)
{
  value *= -1;
  if(value > 23) value = 23; 
  if(value < -23 ) value = -23;

  /* No, standard convention in robotics:
       +X is forward
       +Y is left
       +Z is up
  */
  Walking::GetInstance()->Y_MOVE_AMPLITUDE = (double)value; 
}

void  Performer::GetWalkingVelocityX   (float* value)
{
  /* they mistakenly defined X as Y */
  /* No, standard convention in robotics:
       +X is forward
       +Y is left
       +Z is up
  */
  *value = (float)Walking::GetInstance()->Y_MOVE_AMPLITUDE;
}

void   Performer::SetWalkingVelocityY   (float  value)
{
  if(value > 40) value = 40; 
  if(value < 5 ) value =  5;
  /* they mistakenly defined Y as Z */
  /* No, standard convention in robotics:
       +X is forward
       +Y is left
       +Z is up
  */
  //printf("Walking Y SET\n");
  Walking::GetInstance()->Z_MOVE_AMPLITUDE = (double)value; 
}

void   Performer::GetWalkingVelocityY   (float* value)
{
  /* they mistakenly defined Y as Z */
  /* No, standard convention in robotics:
       +X is forward
       +Y is left
       +Z is up
  */
  *value = (float)Walking::GetInstance()->Z_MOVE_AMPLITUDE;
}

void   Performer::SetWalkingVelocityZ   (float  value)
{
  if(value > 23) value = 23; 
  if(value < -23 ) value =  -23;
  /* they mistakenly defined Z as X */
  /* No, standard convention in robotics:
       +X is forward
       +Y is left
       +Z is up
  */
  //printf("Walking Z SET\n");
  Walking::GetInstance()->X_MOVE_AMPLITUDE = (double)value;
}

void   Performer::GetWalkingVelocityZ   (float* value)
{
  /* they mistakenly defined Z as X */
  /* No, standard convention in robotics:
       +X is forward
       +Y is left
       +Z is up
  */
  *value = (float)Walking::GetInstance()->X_MOVE_AMPLITUDE;
}

void   Performer::GetWalkingPeriod(float* value)
{
  *value = (float)(Walking::GetInstance()->PERIOD_TIME);
}

void Performer::SetWalkingPeriod(float value)
{
  Walking::GetInstance()->PERIOD_TIME = (double)value;
}

void Performer::GetWalkingBalance     (float* value)
{
  *value = (float)(Walking::GetInstance()->BALANCE_ENABLE);
}

void Performer::SetWalkingBalance    (float  value)
{
  Walking::GetInstance()->BALANCE_ENABLE = (value != 0);  
}

void Performer::GetWalkingEnable(float* value, int* numValues)
{
  if(*numValues < 3) return;
  
  //quick and dirty, but will work unless other motions are added
  *value++ = (float)MotionStatus::m_CurrentJoints.GetEnable(20);
  *value++ = (float)MotionStatus::m_CurrentJoints.GetEnable( 1);
  *value++ = (float)MotionStatus::m_CurrentJoints.GetEnable( 2);
    
  *numValues = 3;
}

void Performer::SetWalkingEnable(bool head, bool rightArm, bool leftArm)
{
  int i, enable; 
  for(i=1; i<=JointData::NUMBER_OF_SERVOS; i++)
    {
      if(i<=6)
        {
          if(i%2) enable = rightArm;
          else    enable = leftArm;
        }
       else if ( (i == JointData::ID_HEAD_PAN) || (i == JointData::ID_HEAD_TILT) )
         enable = head;
       else if (i == JointData::ID_R_GRIPPER)
         enable = rightArm
       else if (i == JointData::ID_L_GRIPPER)
         enable = leftArm
       else
         enable = true;
      MotionStatus::m_CurrentJoints.SetEnable(i, enable);
    }
}

int Performer::GetFSR (FSRID_t ID, float value[4], int numSamples)
{
  int error = -1; 
  int sum[4] = {0, 0, 0, 0}, temp, i;
  int exists = servoExists(ID);
    
  if(exists)
    {
      if(numSamples < 1)  numSamples = 1;
      if(numSamples > 20) numSamples = 20;
  
      for(i=0; i<numSamples; i++)
        {
          cm730->ReadWord(ID,  FSR::P_FSR1_L, &temp, &error);
          sum[0] += temp;
          cm730->ReadWord(ID,  FSR::P_FSR2_L, &temp, &error);
          sum[1] += temp;
          cm730->ReadWord(ID,  FSR::P_FSR3_L, &temp, &error);
          sum[2] += temp;
          cm730->ReadWord(ID,  FSR::P_FSR4_L, &temp, &error);
          sum[3] += temp;
        }       
    }
  if(ID == FSR_RIGHT)
    {
      value[0] = 0.001 * (double) sum[0] / (double) numSamples;
      value[1] = 0.001 * (double) sum[0] / (double) numSamples;
      value[2] = 0.001 * (double) sum[0] / (double) numSamples;
      value[3] = 0.001 * (double) sum[0] / (double) numSamples;
    }
  else
    {
      value[3] = 0.001 * (double) sum[0] / (double) numSamples;
      value[1] = 0.001 * (double) sum[0] / (double) numSamples;
      value[1] = 0.001 * (double) sum[0] / (double) numSamples;
      value[0] = 0.001 * (double) sum[0] / (double) numSamples;
    }

  return exists;
}

int Performer::GetFSRComponent(FSRID_t ID, FSRComponent_t component, float* value, int numSamples)
{
  int error = -1,  exists = servoExists(ID);
  int temp, sum = 0, i, numActualSamples = 0;
  
  if((component == FSR_CENTER_Z) || (component == FSR_CENTER_X))
    if(exists)
      {
        if(numSamples < 1)  numSamples = 1;
        if(numSamples > 20) numSamples = 20;
        for(i=0; i<numSamples; i++)
          {
            error = cm730->ReadByte(ID, component, &temp, &error);
            if(temp != 255) {sum += temp; numActualSamples++;}
          }
      }
  if(numActualSamples == 0)
    {
      *value = -10;
    }
  else
    {
      *value = (double) sum / (double) numActualSamples;
      if(  ((ID==FSR_RIGHT)&&(component==FSR_CENTER_Z))  ||  ((ID==FSR_LEFT)&&(FSR_CENTER_X))  )
        *value = 254 - *value;
      *value /= 127;
      *value -= 1;
    }
  return exists;
}

void Performer::Speak(char* string)
{
  char* command;
  asprintf(&command, "%s %c%s%s", "espeak -ven-us -s 120 -g 1", '"', string, "\" &");
  if(Global::verbosity >= 2)
    printf("speak: %s\n", command);
  system(command);
  free(command);
}
