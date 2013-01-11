/*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#ifndef __UGA_INTERPRETER__
#define __UGA_INTERPRETER__ 1

#define INTERPRETER_IN_DATA_SIZE  300
#define INTERPRETER_MAX_NUM_VALUES 30

#include "LinuxDARwIn.h"
using namespace Robot;

#include "Performer.h"
#include "../Communication/Communication.h" //remove me later
#include "../MediaStreamers/MediaStreamers.h"
#include "../RequestListeners/RequestListeners.h"
#include <string.h>  //strlen()


class Interpreter
{
 public:
 
  typedef enum 
  { // order matters, used to index return labels
    ID_PLACEHOLDER          =  0                             ,
    ID_R_SHOULDER_PITCH_POS =  JointData::ID_R_SHOULDER_PITCH,
    ID_L_SHOULDER_PITCH_POS =  JointData::ID_L_SHOULDER_PITCH,
    ID_R_SHOULDER_ROLL_POS  =  JointData::ID_R_SHOULDER_ROLL ,
    ID_L_SHOULDER_ROLL_POS  =  JointData::ID_L_SHOULDER_ROLL ,
    ID_R_ELBOW_POS          =  JointData::ID_R_ELBOW         ,
    ID_L_ELBOW_POS          =  JointData::ID_L_ELBOW         ,
    ID_R_HIP_YAW_POS        =  JointData::ID_R_HIP_YAW       ,
    ID_L_HIP_YAW_POS        =  JointData::ID_L_HIP_YAW       ,
    ID_R_HIP_ROLL_POS       =  JointData::ID_R_HIP_ROLL      ,
    ID_L_HIP_ROLL_POS       =  JointData::ID_L_HIP_ROLL      ,
    ID_R_HIP_PITCH_POS      =  JointData::ID_R_HIP_PITCH     ,
    ID_L_HIP_PITCH_POS      =  JointData::ID_L_HIP_PITCH     ,
    ID_R_KNEE_POS           =  JointData::ID_R_KNEE          ,
    ID_L_KNEE_POS           =  JointData::ID_L_KNEE          ,
    ID_R_ANKLE_PITCH_POS    =  JointData::ID_R_ANKLE_PITCH   ,
    ID_L_ANKLE_PITCH_POS    =  JointData::ID_L_ANKLE_PITCH   ,
    ID_R_ANKLE_ROLL_POS     =  JointData::ID_R_ANKLE_ROLL    ,
    ID_L_ANKLE_ROLL_POS     =  JointData::ID_L_ANKLE_ROLL    ,
    ID_HEAD_PAN_POS         =  JointData::ID_HEAD_PAN        ,
    ID_HEAD_TILT_POS        =  JointData::ID_HEAD_TILT       ,
    ID_R_HAND_POS           =  Performer::ID_R_HAND          ,
    ID_L_HAND_POS           =  Performer::ID_L_HAND          ,
    ID_R_ARM_YAW_POS        =  Performer::ID_R_ARM_YAW       ,
    ID_L_ARM_YAW_POS        =  Performer::ID_L_ARM_YAW       ,
    
    SPEED_ID_OFFSET           ,
    ID_R_SHOULDER_PITCH_SPEED ,
    ID_L_SHOULDER_PITCH_SPEED ,     
    ID_R_SHOULDER_ROLL_SPEED  ,    
    ID_L_SHOULDER_ROLL_SPEED  ,   
    ID_R_ELBOW_SPEED          ,
    ID_L_ELBOW_SPEED          ,
    ID_R_HIP_YAW_SPEED        ,
    ID_L_HIP_YAW_SPEED        ,
    ID_R_HIP_ROLL_SPEED       ,
    ID_L_HIP_ROLL_SPEED       ,
    ID_R_HIP_PITCH_SPEED      ,
    ID_L_HIP_PITCH_SPEED      ,
    ID_R_KNEE_SPEED           ,
    ID_L_KNEE_SPEED           , 
    ID_R_ANKLE_PITCH_SPEED    ,
    ID_L_ANKLE_PITCH_SPEED    ,
    ID_R_ANKLE_ROLL_SPEED     ,
    ID_L_ANKLE_ROLL_SPEED     ,
    ID_HEAD_PAN_SPEED         ,
    ID_HEAD_TILT_SPEED        ,
    ID_R_HAND_SPEED           ,
    ID_L_HAND_SPEED           ,
    ID_R_ARM_YAW_SPEED        ,
    ID_L_ARM_YAW_SPEED        ,
    
    ENABLE_ID_OFFSET           ,
    ID_R_SHOULDER_PITCH_ENABLE ,
    ID_L_SHOULDER_PITCH_ENABLE ,     
    ID_R_SHOULDER_ROLL_ENABLE  ,    
    ID_L_SHOULDER_ROLL_ENABLE  ,   
    ID_R_ELBOW_ENABLE          ,
    ID_L_ELBOW_ENABLE          ,
    ID_R_HIP_YAW_ENABLE        ,
    ID_L_HIP_YAW_ENABLE        ,
    ID_R_HIP_ROLL_ENABLE       ,
    ID_L_HIP_ROLL_ENABLE       ,
    ID_R_HIP_PITCH_ENABLE      ,
    ID_L_HIP_PITCH_ENABLE      ,
    ID_R_KNEE_ENABLE           ,
    ID_L_KNEE_ENABLE           , 
    ID_R_ANKLE_PITCH_ENABLE    ,
    ID_L_ANKLE_PITCH_ENABLE    ,
    ID_R_ANKLE_ROLL_ENABLE     ,
    ID_L_ANKLE_ROLL_ENABLE     ,
    ID_HEAD_PAN_ENABLE         ,
    ID_HEAD_TILT_ENABLE        ,
    ID_R_HAND_ENABLE           ,
    ID_L_HAND_ENABLE           ,
    ID_R_ARM_YAW_ENABLE        ,
    ID_L_ARM_YAW_ENABLE        ,

    ID_BODY_POS                , 
    BODY_ID_OFFSET = ID_BODY_POS, 
    ID_BODY_SPEED              , 
    ID_BODY_ENABLE             ,     

    ID_ACCELEROMETER_ALL      ,
    ID_ACCELEROMETER_X        ,
    ID_ACCELEROMETER_Y        ,
    ID_ACCELEROMETER_Z        ,  

    ID_GYROSCOPE_ALL          ,
    ID_GYROSCOPE_X            ,
    ID_GYROSCOPE_Y            ,
    ID_GYROSCOPE_Z            ,

    ID_EAR_BOTH               ,
    ID_R_EAR                  ,
    ID_L_EAR                  ,
 
    ID_EYE_ALL                ,
    ID_EYE_RED                ,
    ID_EYE_GREEN              ,
    ID_EYE_BLUE               ,

    ID_FOREHEAD_ALL           ,
    ID_FOREHEAD_RED           ,
    ID_FOREHEAD_GREEN         ,
    ID_FOREHEAD_BLUE          ,

    ID_WALKING_ANGLE          ,
    ID_WALKING_VELOCITY_X     ,
    ID_WALKING_VELOCITY_Y     ,
    ID_WALKING_VELOCITY_Z     ,
    ID_WALKING_PERIOD         ,
    ID_WALKING_ENABLE         ,
    ID_WALKING_BALANCE        ,

    ID_CAMERA_BRIGHTNESS      ,
    ID_CAMERA_CONTRAST        ,
    ID_CAMERA_SATURATION      ,
    ID_CAMERA_GAIN            ,
    ID_CAMERA_EXPOSURE        ,
    
    ID_R_FSR                  ,  //4 values as list
    ID_L_FSR                  ,
    ID_R_FSR_X                ,  //center values
    ID_L_FSR_X                ,
    ID_R_FSR_Z                ,
    ID_L_FSR_Z                ,

    ID_UNKNOWN_REQUEST        ,
    ID_NUM_IDS                ,
  }requestID_t;

typedef enum 
  {          
    /* component values used for array indexing */
    /* others interchangeable within categories */
    X            ,
    Y            ,
    Z            ,
    RED = X      ,
    GREEN = Y    ,
    BLUE  = Z    ,
    MAX_COMPONENT_OFFSET = BLUE,  

    GET          ,
    SET          ,
    INCREMENT    ,
    START        ,
    STOP         ,
    MAX_REQUEST_TYPE_OFFSET = STOP,

    BODY         ,
    HEAD         ,
    SHOULDER     ,
    ELBOW        ,
    HAND         ,
    ARM          ,  //for optional gripper arm
    HIP          ,
    KNEE         ,
    ANKLE        ,
    ACCELEROMETER,
    GYROSCOPE    ,
    EAR          ,
    EYE          ,
    FOREHEAD     ,
    CAMERA       ,
    MICROPHONE   ,
    LINEIN       ,
    SPEAKER      ,
    WALKING      ,
    MAX_BODY_PART_OFFSET = WALKING, 
   
    RIGHT        ,
    LEFT         ,
    MAX_SIDE_OFFSET = LEFT,
    
    DEGREES      ,
    RADIANS      ,
    MAX_UNITS_OFFSET = RADIANS,
    
    PROPERTY_OFFSET,
    ANGLE        ,
    SPEED        ,
    ENABLE       ,
    BALANCE      ,
    PRESSURE     ,
    PERIOD       ,
    BRIGHTNESS   ,
    SATURATION   ,
    CONTRAST     ,
    GAIN         ,
    EXPOSURE     ,
    MAX_PROPERTY_OFFSET = EXPOSURE,
             
    CLEAR        ,
    SPEAK        ,
    SLEEP        ,
    KILL         ,
    SYSTEM       ,
    MAX_NO_CATEGORY_OFFSET = SYSTEM,
    
    NOT_UNDERSTOOD,
  }wordID_t;
  
typedef struct 
{
  const char*    word;
  wordID_t       ID  ;
}dictionaryEntry_t;


static const dictionaryEntry_t dictionary[];




 private:
  int   NUM_WORDS_IN_DICTIONARY;
  wordID_t getIDForWord(char* word);
  static float degreesToRadians(float value) { return value / 57.295779513082322; }
  static float radiansToDegrees(float value) { return value * 57.295779513082322; }
  void  makeLowercase          (char* string);
  int   stringToWords          (char* string);
  void  rejoinWords            (char* firstWord, int numWords);
  //void  fixLeftServoAngle      (requestID_t ID, float* value   );
  void  handleStartStop        (float* values, int *numValues);
  /* Explicit request type to accomodate INCREMENT_REQUEST_TYPE                               */
  void  convertUnits(requestID_t requestID, float* values, int numValues, wordID_t requestType);
  requestID_t lookupRequestID (             );
  void  clearRequest      (             );
  const char* getReturnLabel(requestID_t requestID);
  void  setRequestedValues(float* values   , int  numValues, requestID_t requestID);
  void  getRequestedValues(float  values[INTERPRETER_MAX_NUM_VALUES], int *numValues, requestID_t requestID);
  void  incrementRequestedValues(float* values, int numValues, requestID_t requestID);
  bool  componentWasSet; 
  bool  sideWasSet;  
 protected:
  RequestListener  *listener;
  MediaStreamer    *camera;
  MediaStreamer    *microphone;
  MediaStreamer    *linein;
  MediaStreamer    *speaker;
  
  //char inData[INTERPRETER_IN_DATA_SIZE], outData[INTERPRETER_OUT_DATA_SIZE];

 public:
  wordID_t type; 
  wordID_t bodyPart;    
  wordID_t side;        
  wordID_t component;   
  wordID_t units;      
  wordID_t property;

  Interpreter();
 ~Interpreter(); 

  void        ListenForRequests (RequestListener* listener);
  requestID_t InterpretRequest  (char* request, float values[INTERPRETER_MAX_NUM_VALUES], int* numValues);
  void        PerformRequest    (requestID_t requestID, float values[INTERPRETER_MAX_NUM_VALUES], int* numValues);
};


#endif//__INTERPRETER__
