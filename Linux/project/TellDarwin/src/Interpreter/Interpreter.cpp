 /*
 * Written by Michael Krzyzaniak
 * at the University of Georgia
 * Ideas for Creative Exploration (ICE)
 * Summer, 2011
 * krzyzani@uga.edu
 */

#include "Interpreter.h" 

#include <stdio.h> //for debugging only
#include <math.h>  

#define performer  Performer::GetInstance()

namespace Global {extern int verbosity;}


//Must be alphabetized
const Interpreter::dictionaryEntry_t Interpreter::dictionary[] = 
  {
    {"accel"          , Interpreter::ACCELEROMETER},
    {"accelerometer"  , Interpreter::ACCELEROMETER},
    {"angle"          , Interpreter::ANGLE        },
    {"ankle"          , Interpreter::ANKLE        },
    {"arm"            , Interpreter::ARM          },
    {"balance"        , Interpreter::BALANCE      },
    {"begin"          , Interpreter::START        },
    {"blue"           , Interpreter::Z            },
    {"body"           , Interpreter::BODY         },
    {"brightness"     , Interpreter::BRIGHTNESS   },
    {"camera"         , Interpreter::CAMERA       },
    {"clear"          , Interpreter::CLEAR        },
    {"contrast"       , Interpreter::CONTRAST     },
    {"degree"         , Interpreter::DEGREES      },
    {"degrees"        , Interpreter::DEGREES      },
    {"direction"      , Interpreter::ANGLE        },
    {"ear"            , Interpreter::EAR          },
    {"ears"           , Interpreter::EAR          },
    {"elbow"          , Interpreter::ELBOW        },
    {"enable"         , Interpreter::ENABLE       },
    {"enabled"        , Interpreter::ENABLE       },
    {"exposure"       , Interpreter::EXPOSURE     },
    {"eye"            , Interpreter::EYE          },
    {"eyes"           , Interpreter::EYE          },
    {"foot"           , Interpreter::ANKLE        },
    {"forehead"       , Interpreter::FOREHEAD     },
    {"gain"           , Interpreter::GAIN         },
    {"get"            , Interpreter::GET          },
    {"green"          , Interpreter::Y            },
    {"gyro"           , Interpreter::GYROSCOPE    },
    {"gyroscope"      , Interpreter::GYROSCOPE    },
    {"hand"           , Interpreter::HAND         },
    {"head"           , Interpreter::HEAD         },
    {"hip"            , Interpreter::HIP          },
    {"increment"      , Interpreter::INCREMENT    },
    {"kill"           , Interpreter::KILL         },
    {"knee"           , Interpreter::KNEE         },
    {"left"           , Interpreter::LEFT         },
    {"leg"            , Interpreter::HIP          },
    {"mic"            , Interpreter::MICROPHONE   },
    {"microphone"     , Interpreter::MICROPHONE   },
    {"neck"           , Interpreter::HEAD         },
    {"pan"            , Interpreter::Y            },
    {"pause"          , Interpreter::SLEEP        },
    {"period"         , Interpreter::PERIOD       },
    {"pitch"          , Interpreter::X            },
    {"position"       , Interpreter::ANGLE        },
    {"pressure"       , Interpreter::PRESSURE     },
    {"quit"           , Interpreter::STOP         },
    {"radian"         , Interpreter::RADIANS      },
    {"radians"        , Interpreter::RADIANS      },
    {"red"            , Interpreter::X            },
    {"right"          , Interpreter::RIGHT        },
    {"roll"           , Interpreter::Z            },
    {"saturation"     , Interpreter::SATURATION   },
    {"say"            , Interpreter::SPEAK        },
    {"set"            , Interpreter::SET          },
    {"shoulder"       , Interpreter::SHOULDER     },
    {"sleep"          , Interpreter::SLEEP        },
    {"speak"          , Interpreter::SPEAK        },
    {"speaker"        , Interpreter::SPEAKER      },
    {"speed"          , Interpreter::SPEED        },
    {"start"          , Interpreter::START        },
    {"stop"           , Interpreter::STOP         },
    {"system"         , Interpreter::SYSTEM       },
    {"tilt"           , Interpreter::X            },
    {"usbmic"         , Interpreter::LINEIN       },
    {"usbmicrophone"  , Interpreter::LINEIN       },
    {"velocity"       , Interpreter::SPEED        },
    {"video"          , Interpreter::CAMERA       },
    {"walk"           , Interpreter::WALKING      },
    {"walking"        , Interpreter::WALKING      },
    {"x"              , Interpreter::X            },
    {"y"              , Interpreter::Y            },
    {"yaw"            , Interpreter::Y            },
    {"z"              , Interpreter::Z            },
    
    //for iterating;
    {""               , Interpreter::NOT_UNDERSTOOD},
  };

Interpreter::Interpreter()
{
  NUM_WORDS_IN_DICTIONARY = 0;
  while(dictionary[NUM_WORDS_IN_DICTIONARY].ID != NOT_UNDERSTOOD) NUM_WORDS_IN_DICTIONARY++;

  clearRequest();
  listener   = NULL;
  camera     = NULL;
  microphone = NULL;
  linein     = NULL;
  speaker    = NULL;
}

Interpreter::~Interpreter()
{
  delete camera;
  delete microphone;
  delete speaker;
  delete linein;
}

int Interpreter::stringToWords(char* string)
{
  int numWords = 1;
  while(*string != '\0')
    {
      if(*string == ' ')
	{
	  *string = '\0';
	  numWords++;
	}
      string++;
    }
  return numWords;
}

void Interpreter::rejoinWords(char* firstWord, int numWords)
{
  numWords -=1;
  while(numWords > 0)
    {
      if(*firstWord == '\0')
	{
	  *firstWord = ' ';
	  --numWords;
	}
      ++firstWord;
    }
}

void Interpreter::makeLowercase(char* string)
{
  while(*string != '\0')
    {
      if( (*string >= 'A') && (*string <= 'Z') )
	*string += ('a' - 'A');
      string++;
    }
}

Interpreter::wordID_t Interpreter::getIDForWord(char* word)
{
//printf("get ID for Word: %s\n", word);
  const dictionaryEntry_t* nextEntry;
  int indexOfNextEntry = NUM_WORDS_IN_DICTIONARY/2;
  int i=2, whichComesFirst, advanceAmount=1;

  while(advanceAmount)
    {
      if (indexOfNextEntry >= NUM_WORDS_IN_DICTIONARY) break;
      nextEntry = &dictionary[indexOfNextEntry];
      whichComesFirst = strcmp(word, nextEntry->word);
      advanceAmount = NUM_WORDS_IN_DICTIONARY / (float)(1 << i) + 0.5;

      if     (whichComesFirst == 0) return nextEntry->ID;
      else if(whichComesFirst  > 0) indexOfNextEntry += advanceAmount;
      else if(whichComesFirst  < 0) indexOfNextEntry -= advanceAmount;
      i++;
    }
    
  return NOT_UNDERSTOOD;
}

void  Interpreter::ListenForRequests (RequestListener* listener)
{
  float values[INTERPRETER_MAX_NUM_VALUES] = {0, };
  int   numValues = 0;
  char  inData[INTERPRETER_IN_DATA_SIZE];
  int   numCharsInBuffer;
  requestID_t requestID = ID_UNKNOWN_REQUEST;
  this->listener = listener;

  if(listener->Initialize())
    {      
      while(1)
	{
	  numCharsInBuffer = INTERPRETER_IN_DATA_SIZE;
	  numValues        = INTERPRETER_MAX_NUM_VALUES;
	  if(listener->WaitForRequest(inData, &numCharsInBuffer, values, &numValues))
            {
              if(numCharsInBuffer > 0)
                requestID = InterpretRequest(inData, values, &numValues);
              
              if(numValues > -1)
                PerformRequest(requestID, values, &numValues);
            
              if(numValues > 0)
                listener->ReturnRequestedValues(getReturnLabel(requestID), values, numValues);
                
          }
          else break;
        }
      }
      this->listener = NULL;
}

void  Interpreter::PerformRequest(requestID_t requestID, float values[INTERPRETER_MAX_NUM_VALUES], int* numValues)
{
  switch(type)
    {
    case GET: getRequestedValues(values, numValues, requestID); break;
    case SET: setRequestedValues(&values[0], *numValues, requestID); *numValues = 0; break;
    case INCREMENT: incrementRequestedValues(&values[0], *numValues, requestID); *numValues = 0;  break;
    default: break;
    }
}

Interpreter::requestID_t Interpreter::InterpretRequest(char* request, float values[INTERPRETER_MAX_NUM_VALUES], int* numValues)
{
  if(Global::verbosity >= 2) printf("Request: %s\n", request); 
  int numWords = stringToWords(request);
  componentWasSet = sideWasSet = false;
  bool shouldSleep = false;
  wordID_t id;

  while(numWords-- > 0)
    {
      makeLowercase(request);  // do this for each word to preserve case for 'SYSTEM' requests
      if(  ((*request >= '0') && (*request <= '9')) || (*request == '.') || (*request == '-'))
        {
          if(*numValues < INTERPRETER_MAX_NUM_VALUES)
	    values[(*numValues)++] = atof(request);
	}
      else
        {//printf("word: %s, numWords: %i\n", request, numWords);
          id = getIDForWord(request);
          if      (id <= MAX_COMPONENT_OFFSET)    { component   = id; componentWasSet = true;}
          else if(id <= MAX_REQUEST_TYPE_OFFSET) { type        = id;}
          else if(id <= MAX_BODY_PART_OFFSET)    { bodyPart    = id;}
          else if(id <= MAX_SIDE_OFFSET)         { side        = id; sideWasSet = true;}
          else if(id <= MAX_UNITS_OFFSET)        { units       = id;}
          else if(id <= MAX_PROPERTY_OFFSET)     { property    = id;}
          else if(id <= MAX_NO_CATEGORY_OFFSET)
            {
              switch(id)
              {
                case SLEEP        : shouldSleep = true; break;
                case KILL         : exit(0); /*not really necessary*/ break;
                case SYSTEM       : 
	          request += strlen(request)+1;
                  rejoinWords(request, numWords);
                  if(Global::verbosity >= 2)
                    printf("system: %s \n", request);
                  system(request);
	          *numValues = -1;
	         numWords = 0;
	          break;               
                case SPEAK        :	    
	          if(speaker == NULL)
	            {
	              request += strlen(request)+1;
	              rejoinWords(request, numWords);
	              performer->Speak(request);
	              *numValues = 0;
	            }
	          else if(Global::verbosity >= 1) 
	            fprintf(stderr, "cannot speak while streaming to the speaker\n");
	          numWords = 0;
	          break;
	        default: break;
              }
            }
        }
      request += strlen(request)+1;
    }

  if(shouldSleep)
    if(*numValues >= 1)
      {
        usleep(values[0] * 1000);
        *numValues = -1;
      }
      
  //See if anything needs to happen now
  handleStartStop(values, numValues);
  return lookupRequestID();
}

void  Interpreter::handleStartStop(float* values, int *numValues)
{
  switch(type)
    {
    case START: 
      switch(bodyPart)
	{
	case WALKING:
	  performer->StartWalking(); 
	  break;
	case CAMERA:
	  if(listener != NULL)
	    {
	      if(camera == NULL) 
                camera = new JitterCameraStreamer;
              if(!(camera->Start(listener->GetRequesterIP(), *numValues > 0 ? values[0] : 7474)))
                {delete camera; camera = NULL;}
	    }
	  break;
	case MICROPHONE:
	  if(listener != NULL)
	    {
	      if(microphone == NULL) 
                microphone = new JitterMicrophoneStreamer;
              if(!(microphone->Start(listener->GetRequesterIP(),  *numValues > 0 ? values[0] : 8080)))
                {delete microphone; microphone = NULL;}
	    }
          break;

	case LINEIN:
	  if(listener != NULL)
	    {
	      if(linein == NULL) 
                linein = new JitterLineInStreamer;
              if(!(linein->Start(listener->GetRequesterIP(),  *numValues > 0 ? values[0] : 8082)))
                {delete linein; linein = NULL;}
	    }
          break;

	case SPEAKER:
	    {
	      if(speaker == NULL) 
                speaker = new JitterSpeakerStreamer;
              if(!(speaker->Start(listener->GetRequesterIP(),  *numValues > 0 ? values[0] : 8081)))
                {delete speaker; speaker = NULL;}
	    }   
          break;
        default: break;
	}
      *numValues = -1;
      break;
      
    case STOP:
      switch(bodyPart)
	{
	case WALKING: 
	  performer->StopWalking();
	  break;
	case CAMERA:
	  delete camera; camera = NULL;
	  break;
	case MICROPHONE:
          delete microphone; microphone = NULL;
	  break;
	case LINEIN:
          delete linein; linein = NULL;
	  break;
	case SPEAKER:
          delete speaker; speaker = NULL;
	  break;
        default: break;
	}
      *numValues = -1;
      break;
     
    default: break;
    }
}

void  Interpreter::incrementRequestedValues(float* values, int numValues, requestID_t requestID)
{
  int numGetValues = 0;
  float getValues[INTERPRETER_MAX_NUM_VALUES]; 

  getRequestedValues(getValues, &numGetValues, requestID);
    
  int i;
  if((bodyPart == BODY) && (numValues == 1))
    {
      for(i=0; (i<numGetValues) && (i<INTERPRETER_MAX_NUM_VALUES); i++)
        getValues[i] += values[0];
    }
  else
    {
      for(i=0; (i<numValues) && (i<numGetValues); i++)
        getValues[i] += values[i];
    }
  if(property == ENABLE)
    {
      for(i=0; i<numGetValues; i++)
        getValues[i] = (int)getValues[i] % 2;
    }
  setRequestedValues(getValues, i, requestID);
}


const char* Interpreter::getReturnLabel(requestID_t requestID)
{
  static const char* labels[ID_NUM_IDS] = 
    {
      //order must match enum requestID_t
      "UNKNOWN_VALUE"      ,
      "R_SHOULDER_X"       ,
      "L_SHOULDER_X"       ,
      "R_SHOULDER_Z"       ,
      "L_SHOULDER_Z"       ,
      "R_ELBOW"            ,
      "L_ELBOW"            ,
      "R_HIP_Y"            ,
      "L_HIP_Y"            ,
      "R_HIP_Z"            ,
      "L_HIP_Z"            ,
      "R_HIP_X"            ,
      "L_HIP_X"            ,
      "R_KNEE"             ,
      "L_KNEE"             ,
      "R_ANKLE_X"          ,
      "L_ANKLE_X"          ,
      "R_ANKLE_Z"          ,
      "L_ANKLE_Z"          ,
      "HEAD_Y"             ,
      "HEAD_X"             ,
      "R_HAND"             ,
      "L_HAND"             ,
      "R_ARM_Y"          ,
      "L_ARM_Y"          ,
      
      "UNKNOWN_VALUE"      , //because ROBOTIS 1-indexes servos
      "R_SHOULDER_X_SPEED" ,
      "L_SHOULDER_X_SPEED" ,
      "R_SHOULDER_Z_SPEED" ,
      "L_SHOULDER_Z_SPEED" ,
      "R_ELBOW_SPEED"      ,
      "L_ELBOW_SPEED"      ,
      "R_HIP_Y_SPEED"      ,
      "L_HIP_Y_SPEED"      ,
      "R_HIP_Z_SPEED"      ,
      "L_HIP_Z_SPEED"      ,
      "R_HIP_X_SPEED"      ,
      "L_HIP_X_SPEED"      ,
      "R_KNEE_SPEED"       ,
      "L_KNEE_SPEED"       ,
      "R_ANKLE_X_SPEED"    ,
      "L_ANKLE_X_SPEED"    ,
      "R_ANKLE_Z_SPEED"    ,
      "L_ANKLE_Z_SPEED"    ,
      "HEAD_Y_SPEED"       ,
      "HEAD_X_SPEED"       ,
      "R_HAND_SPEED"       ,
      "L_HAND_SPEED"       ,
      "R_ARM_Y_SPEED"    ,
      "L_ARM_Y_SPEED"    ,
 
      "UNKNOWN_VALUE"      , //because ROBOTIS 1-indexes servos      
      "R_SHOULDER_X_ENABLE",
      "L_SHOULDER_X_ENABLE",
      "R_SHOULDER_Z_ENABLE",
      "L_SHOULDER_Z_ENABLE",
      "R_ELBOW_ENABLE"     ,
      "L_ELBOW_ENABLE"     ,
      "R_HIP_Y_ENABLE"     ,
      "L_HIP_Y_ENABLE"     ,
      "R_HIP_Z_ENABLE"     ,
      "L_HIP_Z_ENABLE"     ,
      "R_HIP_X_ENABLE"     ,
      "L_HIP_X_ENABLE"     ,
      "R_KNEE_ENABLE"      ,
      "L_KNEE_ENABLE"      ,
      "R_ANKLE_X_ENABLE"   ,
      "L_ANKLE_X_ENABLE"   ,
      "R_ANKLE_Z_ENABLE"   ,
      "L_ANKLE_Z_ENABLE"   ,
      "HEAD_Y_ENABLE"      ,
      "HEAD_X_ENABLE"      ,
      "R_HAND_ENABLE"      ,
      "L_HAND_ENABLE"      ,
      "R_ARM_Y_ENABLE"    ,
      "L_ARM_Y_ENABLE"    ,
 
      "BODY"               ,
      "BODY_SPEED"         ,
      "BODY_ENABLE"        ,
      
      "ACCEL"              ,
      "ACCEL_X"            ,
      "ACCEL_Y"            ,
      "ACCEL_Z"            ,
      
      "GYRO"               ,
      "GYRO_X"             ,
      "GYRO_Y"             ,
      "GYRO_Z"             ,
      
      "EARS"               ,
      "R_EAR"              ,
      "L_EAR"              ,
      
      "EYE"                ,
      "EYE_R"              ,
      "EYE_G"              ,
      "EYE_B"              ,
      
      "FOREHEAD"           ,
      "FOREHEAD_R"         ,
      "FOREHEAD_G"         ,
      "FOREHEAD_B"         ,
      
      "WALK_DIRECTION"     ,
      "WALK_X"             ,
      "WALK_Y"             ,
      "WALK_Z"             ,
      "WALK_PERIOD"        ,
      "WALK_ENABLE"        ,
      "WALK_BALANCE"       ,
      
      "CAMERA_BRIGHT"      ,
      "CAMERA_CONTR"       ,
      "CAMERA_SAT"         ,
      "CAMERA_GAIN"        ,
      "CAMERA_EXP"         ,
      
      "R_FSR"              ,
      "L_FSR"              ,
      "R_FSR_X"            ,
      "L_FSR_X"            ,
      "R_FSR_Z"            ,      
      "L_FSR_Z"            , 
           
      "UNKNOWN_VALUE"      , 
    };
  return labels[requestID];
}

void  Interpreter::getRequestedValues(float  values[INTERPRETER_MAX_NUM_VALUES], int *numValues, requestID_t requestID)
{
  if(requestID < SPEED_ID_OFFSET)
    {performer->GetServoAngle(requestID, values); *numValues = 1;} 
  else if(requestID < ENABLE_ID_OFFSET)
    {performer->GetServoSpeed((requestID - SPEED_ID_OFFSET), values); *numValues = 1;}
  else if(requestID < BODY_ID_OFFSET)
    {performer->GetServoEnable((requestID - ENABLE_ID_OFFSET), values); *numValues = 1;}
    
    else {switch(requestID){  
    case ID_BODY_POS           : *numValues = INTERPRETER_MAX_NUM_VALUES; performer->GetBodySomething(values, numValues, &Performer::GetServoAngle ); break;   
    case ID_BODY_SPEED         : *numValues = INTERPRETER_MAX_NUM_VALUES; performer->GetBodySomething(values, numValues, &Performer::GetServoSpeed ); break;
    case ID_BODY_ENABLE        : *numValues = INTERPRETER_MAX_NUM_VALUES; performer->GetBodySomething(values, numValues, &Performer::GetServoEnable); break;
          
    case ID_ACCELEROMETER_ALL  : performer->GetAccel(values, (*numValues>0 )?values[0]:1); *numValues = 3; break;
    case ID_ACCELEROMETER_X    : performer->GetAccelComponent(Performer::ACCEL_X, values, (*numValues>0 )?values[0]:1); *numValues = 1; break;
    case ID_ACCELEROMETER_Y    : performer->GetAccelComponent(Performer::ACCEL_Y, values, (*numValues>0 )?values[0]:1); *numValues = 1; break;
    case ID_ACCELEROMETER_Z    : performer->GetAccelComponent(Performer::ACCEL_Z, values, (*numValues>0 )?values[0]:1); *numValues = 1; break;
      
    case ID_GYROSCOPE_ALL      : performer->GetGyro(values, (*numValues>0 )?values[0]:1);                             *numValues = 3; break;
    case ID_GYROSCOPE_X        : performer->GetGyroComponent(Performer::GYRO_X, values, (*numValues>0 )?values[0]:1); *numValues = 1; break;
    case ID_GYROSCOPE_Y        : performer->GetGyroComponent(Performer::GYRO_Y, values, (*numValues>0 )?values[0]:1); *numValues = 1; break;
    case ID_GYROSCOPE_Z        : performer->GetGyroComponent(Performer::GYRO_Z, values, (*numValues>0 )?values[0]:1); *numValues = 1; break;
      
    case ID_R_FSR              : performer->GetFSR (Performer::FSR_RIGHT, values, (*numValues>0 )?values[0]:1); *numValues = 4; break;
    case ID_L_FSR              : performer->GetFSR (Performer::FSR_LEFT , values, (*numValues>0 )?values[0]:1); *numValues = 4; break;
    case ID_R_FSR_X            : performer->GetFSRComponent (Performer::FSR_RIGHT, Performer::FSR_CENTER_X, values, (*numValues>0 )?values[0]:1); *numValues = 1; break;
    case ID_L_FSR_X            : performer->GetFSRComponent (Performer::FSR_LEFT , Performer::FSR_CENTER_X, values, (*numValues>0 )?values[0]:1); *numValues = 1; break;
    case ID_R_FSR_Z            : performer->GetFSRComponent (Performer::FSR_RIGHT, Performer::FSR_CENTER_Z, values, (*numValues>0 )?values[0]:1); *numValues = 1; break;
    case ID_L_FSR_Z            : performer->GetFSRComponent (Performer::FSR_LEFT , Performer::FSR_CENTER_X, values, (*numValues>0 )?values[0]:1); *numValues = 1; break;
      
    case ID_EYE_ALL            : performer->GetLEDColor(Performer::LED_ID_EYE, values); *numValues = 3; break; 
    case ID_EYE_RED            : /*cascade*/
    case ID_EYE_GREEN          : /*cascade*/
    case ID_EYE_BLUE           : 
      performer->GetLEDColorComponent(Performer::LED_ID_EYE, (Performer::colorComponent_t)component, values);
      *numValues = 1;
      break; 
    case ID_EAR_BOTH           : performer->GetEars(                     values); *numValues = 2; break;
    case ID_R_EAR              : performer->GetEar (Performer::EAR_ID_R, values); *numValues = 1; break;
    case ID_L_EAR              : performer->GetEar (Performer::EAR_ID_L, values); *numValues = 1; break;  
      
    case ID_CAMERA_BRIGHTNESS  : performer->GetCameraProperty(Performer::CAMERA_BRIGHTNESS, values); *numValues = 1; break;                   
    case ID_CAMERA_CONTRAST    : performer->GetCameraProperty(Performer::CAMERA_CONTRAST  , values); *numValues = 1; break;
    case ID_CAMERA_SATURATION  : performer->GetCameraProperty(Performer::CAMERA_SATURATION, values); *numValues = 1; break;
    case ID_CAMERA_GAIN        : performer->GetCameraProperty(Performer::CAMERA_GAIN      , values); *numValues = 1; break;
    case ID_CAMERA_EXPOSURE    : performer->GetCameraProperty(Performer::CAMERA_EXPOSURE  , values); *numValues = 1; break;
    
    case ID_FOREHEAD_ALL       :  performer->GetLEDColor(Performer::LED_ID_FOREHEAD, values); *numValues = 3; break;
    case ID_FOREHEAD_RED       : /*cascade*/
    case ID_FOREHEAD_GREEN     : /*cascade*/
    case ID_FOREHEAD_BLUE      : 
      performer->GetLEDColorComponent(Performer::LED_ID_FOREHEAD, (Performer::colorComponent_t)component, &values[0]);
      *numValues = 1;
      break; 
      
    case ID_WALKING_ANGLE      : performer->GetWalkingAngle     (values); *numValues = 1; break;
    case ID_WALKING_VELOCITY_X : performer->GetWalkingVelocityX (values); *numValues = 1; break;
    case ID_WALKING_VELOCITY_Y : performer->GetWalkingVelocityY (values); *numValues = 1; break;
    case ID_WALKING_VELOCITY_Z : performer->GetWalkingVelocityZ (values); *numValues = 1; break;
    case ID_WALKING_PERIOD     : performer->GetWalkingPeriod    (values); *numValues = 1; break;
    case ID_WALKING_ENABLE     : *numValues = INTERPRETER_MAX_NUM_VALUES; performer->GetWalkingEnable(values, numValues); break;
    case ID_WALKING_BALANCE    : performer->GetWalkingBalance   (values); *numValues = 1; break;
    default: break;
    }}
  convertUnits(requestID, values, *numValues, GET);

  if(Global::verbosity >= 2)
    {
      printf("Get %s: ", getReturnLabel(requestID));
      int i;
      for(i=0; i<*numValues; i++)
        printf("%f ", values[i]);
      printf("\n");
    }
}


void  Interpreter::setRequestedValues(float* values   , int  numValues, requestID_t requestID)
{
  if(numValues == 0) return;

  if(Global::verbosity >= 2)
    {
      printf("Set %s: ", getReturnLabel(requestID));
      int i;
      for(i=0; i<numValues; i++)
        printf("%f ", values[i]);
      printf("\n");
    }
    
  convertUnits(requestID, values, numValues, SET);
  
  if(requestID < SPEED_ID_OFFSET)
    performer->SetServoAngle(requestID, values[0]);
  else if(requestID < ENABLE_ID_OFFSET)
    performer->SetServoSpeed((requestID - SPEED_ID_OFFSET), values[0]);
  else if(requestID < BODY_ID_OFFSET)
    performer->SetServoEnable((requestID - ENABLE_ID_OFFSET), values[0]);

  else{switch(requestID){ 
      
    case ID_BODY_POS              :
      if((numValues == 1) || (numValues >= 20))
        performer->SetBodySomething(values, (numValues == 1) ? -1 : numValues, &Performer::SetServoAngle);
      break;
    case ID_BODY_SPEED            :
      if((numValues == 1) || (numValues >= 20))
        performer->SetBodySomething(values, (numValues == 1) ? -1 : numValues, &Performer::SetServoSpeed);
      break;
    case ID_BODY_ENABLE           :
      if((numValues == 1) || (numValues >= 20))
        performer->SetBodySomething(values, (numValues == 1) ? -1 : numValues, &Performer::SetServoEnable);
      break;
    case ID_EYE_ALL               : 
      if(numValues != 3) break;
      else performer->SetLEDColor(Performer::LED_ID_EYE, values);
      break; 
    case ID_EYE_RED               : /*cascade*/
    case ID_EYE_GREEN             : /*cascade*/
    case ID_EYE_BLUE              : 
      performer->SetLEDColorComponent(Performer::LED_ID_EYE, (Performer::colorComponent_t)component, values[0]);
      break; 
      
    case ID_CAMERA_BRIGHTNESS  : if(numValues>=1)performer->SetCameraProperty(Performer::CAMERA_BRIGHTNESS, values); break;                   
    case ID_CAMERA_CONTRAST    : if(numValues>=1)performer->SetCameraProperty(Performer::CAMERA_CONTRAST  , values); break;
    case ID_CAMERA_SATURATION  : if(numValues>=1)performer->SetCameraProperty(Performer::CAMERA_SATURATION, values); break;
    case ID_CAMERA_GAIN        : if(numValues>=1)performer->SetCameraProperty(Performer::CAMERA_GAIN      , values); break;
    case ID_CAMERA_EXPOSURE    : if(numValues>=1)performer->SetCameraProperty(Performer::CAMERA_EXPOSURE  , values); break;
      
    case ID_FOREHEAD_ALL          : 
      if(numValues != 3) break;
      else performer->SetLEDColor(Performer::LED_ID_FOREHEAD, values);
      break;
    case ID_FOREHEAD_RED          : /*cascade*/
    case ID_FOREHEAD_GREEN        : /*cascade*/
    case ID_FOREHEAD_BLUE         : 
      performer->SetLEDColorComponent(Performer::LED_ID_FOREHEAD, (Performer::colorComponent_t)component, values[0]);
      break; 
      
    case ID_WALKING_ANGLE         : performer->SetWalkingAngle     (values[0]); break; 
    case ID_WALKING_VELOCITY_X    : performer->SetWalkingVelocityX (values[0]); break; 
    case ID_WALKING_VELOCITY_Y    : performer->SetWalkingVelocityY (values[0]); break;
    case ID_WALKING_VELOCITY_Z    : performer->SetWalkingVelocityZ (values[0]); break;
    case ID_WALKING_PERIOD        : performer->SetWalkingPeriod    (values[0]); break;
    case ID_WALKING_BALANCE       : performer->SetWalkingBalance   (values[0]); break;
    case ID_WALKING_ENABLE        : performer->SetWalkingEnable    ((numValues>0) ? values[0] : 1, 
                                                                     (numValues>1) ? values[1] : 1, 
                                                                     (numValues>2) ? values[2] : 1);
                                                                      break;
    default: break;
    }}
}


/*What is the cyclomatic complexity of this function?*/
Interpreter::requestID_t  Interpreter::lookupRequestID()
{
  requestID_t result = ID_UNKNOWN_REQUEST;
  
  switch(bodyPart)
    {    
      case SHOULDER:
        if(component == Z) result = ID_R_SHOULDER_ROLL_POS ;
        else {result = ID_R_SHOULDER_PITCH_POS; component = X;}
        if(side == LEFT) *((int*)(&result)) += 1;
        break;
      
      case ELBOW:
        result = (side == RIGHT) ? ID_R_ELBOW_POS : ID_L_ELBOW_POS;
        break;
      
      case HIP: 
        switch(component)
          {
            case Y: result = ID_R_HIP_YAW_POS  ; break;
            case Z: result = ID_R_HIP_ROLL_POS ; break;
            default: result = ID_R_HIP_PITCH_POS; component = X; break;
          }
        if(side == LEFT) *((int*)(&result)) += 1;
        break;
      
      case KNEE: 
        result = (side == RIGHT) ? ID_R_KNEE_POS : ID_L_KNEE_POS;
        break;
      
      case ANKLE: 
        if(property == PRESSURE)
          {
            if(componentWasSet)
              {
                if(component == Z) result = ID_R_FSR_Z;
                else {result = ID_R_FSR_X; component = X;}
              }
            else /*(!componentWasSet)*/ result = ID_R_FSR;
          }
        else //(property != PRESSURE)
          {
	    if(component == Z) result = ID_R_ANKLE_ROLL_POS ;
            else{result = ID_R_ANKLE_PITCH_POS; component = X;}
	  }
	  if(side == LEFT) *((int*)(&result)) += 1;
        break;
      
      case HEAD:
        if(component == X) result = ID_HEAD_TILT_POS;
        else {result = ID_HEAD_PAN_POS ; component = Y;}   
        break;
     
      case HAND:
        result = (side == RIGHT) ? ID_R_HAND_POS : ID_L_HAND_POS;
        break;
    
      case ARM:
        if(component == Y) result = ID_R_ARM_YAW_POS;
        else{result = ID_R_ELBOW_POS; component = X;} 
        if(side == LEFT) *((int*)(&result)) += 1;
        break;
    
      default: 
        break;
    }
  
  if(result != ID_UNKNOWN_REQUEST)
    {
      if(property == SPEED)
        //ARRRRGH I HATE C++ !!!!
        *((int*)(&result)) += SPEED_ID_OFFSET;
      else if(property == ENABLE)
        *((int*)(&result)) += ENABLE_ID_OFFSET;
    }
  else /*(result == UNKNOWN_VALUE)*/
    {
    switch(bodyPart)
      {
        case BODY:
          switch(property)
            {
              case ENABLE: result = ID_BODY_ENABLE; break;   
              case SPEED : result = ID_BODY_SPEED; break;
              default             : result = ID_BODY_POS; property = ANGLE;  break;
            }
          break;

      
      case ACCELEROMETER:
	if(componentWasSet)
	  {
	    switch(component)
	      {
	        case X: result = ID_ACCELEROMETER_X; break;
	        case Y: result = ID_ACCELEROMETER_Y; break;
	        case Z: result = ID_ACCELEROMETER_Z; break;
                default: break;
	      }
	  }
	else result = ID_ACCELEROMETER_ALL;
	break;
	
      case GYROSCOPE:
	if(componentWasSet)
	  {
	    switch(component)
	      {
	        case X: result = ID_GYROSCOPE_X; break;
	        case Y: result = ID_GYROSCOPE_Y; break;
	        case Z: result = ID_GYROSCOPE_Z; break;
                default: break;
	      }
	  }
	else result = ID_GYROSCOPE_ALL;
	break;  

      case EAR:
	if(sideWasSet)
	  {
	    switch(side)
	      {
	        case RIGHT:   result = ID_R_EAR  ; break;
	        case LEFT :   result = ID_L_EAR  ; break;
                default: break;
	      }
	  }
	else result = ID_EAR_BOTH;
	break; 
		
      case EYE:
	if(componentWasSet)
	  {
	    switch(component)
	      {
	        case RED:   result = ID_EYE_RED  ; break;
	        case GREEN: result = ID_EYE_GREEN; break;
	        case BLUE:  result = ID_EYE_BLUE ; break;
                default: break;
	      }
	  }
	else result = ID_EYE_ALL;
	break;  
	
      case FOREHEAD:
	if(componentWasSet)
	  {
	    switch(component)
	      {
	        case RED  : result = ID_FOREHEAD_RED  ; break;
	        case GREEN: result = ID_FOREHEAD_GREEN; break;
	        case BLUE : result = ID_FOREHEAD_BLUE ; break;
                default: break;
	      }
	  }
	else result = ID_FOREHEAD_ALL;
	break;  
	
      case CAMERA:
        switch(property)
	  {
	    case BRIGHTNESS: result = ID_CAMERA_BRIGHTNESS; break;
	    case CONTRAST  : result = ID_CAMERA_CONTRAST  ; break;
	    case SATURATION: result = ID_CAMERA_SATURATION; break;
	    case GAIN      : result = ID_CAMERA_GAIN      ; break;
	    case EXPOSURE  : result = ID_CAMERA_EXPOSURE  ; break;
	    default: break;
	  }      
        break;
       
      case WALKING:
	{
	  switch(property)
	    {
	    case ANGLE :  result = ID_WALKING_ANGLE ;  break;
	    case PERIOD:  result = ID_WALKING_PERIOD;  break;
	    case BALANCE: result = ID_WALKING_BALANCE; break;
	    case ENABLE:  result = ID_WALKING_ENABLE;  break;
	    case SPEED : 
	      if(componentWasSet)
		{
		  switch(component)
		    {
		      case X: result = ID_WALKING_VELOCITY_X; break;
		      case Y: result = ID_WALKING_VELOCITY_Y; break;
		      case Z: result = ID_WALKING_VELOCITY_Z; break;
                      default: break;		    
		    }
		}
	      else result = ID_WALKING_VELOCITY_Z;
	      break;	      
            default: break;
	    }
        default: break;
	}
      }
  } 
  return result;
}

void Interpreter::convertUnits(requestID_t requestID, float* values, int numValues, wordID_t requestType)
{
  int   i;
  float (*conversionFunction)(float) = NULL;

  if((requestID <  ENABLE_ID_OFFSET) ||
     (requestID == ID_BODY_POS    )||
     (requestID == ID_BODY_SPEED  ) ||
     (requestID == ID_GYROSCOPE_ALL)||
     (requestID == ID_GYROSCOPE_X ) ||
     (requestID == ID_GYROSCOPE_Y ) ||
     (requestID == ID_GYROSCOPE_Z ) ||
     (requestID == ID_WALKING_ANGLE ))
       {
         if(units == RADIANS)
	  {
	    if     (requestType == GET)
	      conversionFunction = degreesToRadians;
	    else if(requestType == SET)
	      conversionFunction = radiansToDegrees;
          }
       }
       
  if(conversionFunction != NULL)
    for(i=0; i<numValues; i++)
      values[i] = conversionFunction(values[i]);
}

void Interpreter::clearRequest()
{
  type      = NOT_UNDERSTOOD;
  property  = ANGLE         ;
  bodyPart  = HEAD          ;
  side      = RIGHT         ;
  component = Y             ;
  units     = DEGREES       ;
}
