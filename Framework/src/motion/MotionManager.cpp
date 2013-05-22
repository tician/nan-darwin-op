/*
 *   MotionManager.cpp
 *
 *   Author: ROBOTIS
 *
 */

#include <stdio.h>
#include <math.h>
#include "FSR.h"
#include "MX28.h"
#include "MotionManager.h"

using namespace Robot;

MotionManager* MotionManager::m_UniqueInstance = new MotionManager();

MotionManager::MotionManager() :
        m_CM730(0),
        m_ProcessEnable(false),
        m_Enabled(false),
        m_IsRunning(false),
        m_IsThreadRunning(false),
        m_IsLogging(false),
        DEBUG_PRINT(false)
{
    for(int i = 0; i < JointData::NUMBER_OF_JOINTS; i++)
        m_Offset[i] = 0;
}

MotionManager::~MotionManager()
{
}

bool MotionManager::Initialize(CM730 *cm730)
{
	int value, error;

	m_CM730 = cm730;
	m_Enabled = false;
	m_ProcessEnable = true;

	if(m_CM730->Connect() == false)
	{
		if(DEBUG_PRINT == true)
			fprintf(stderr, "Fail to connect CM-730\n");
		return false;
	}

	for(int id=JointData::ID_R_SHOULDER_PITCH; id<JointData::NUMBER_OF_JOINTS; id++)
	{
		if(DEBUG_PRINT == true)
			fprintf(stderr, "ID:%d initializing...", id);
		
		if(m_CM730->ReadWord(id, MX28::P_PRESENT_POSITION_L, &value, &error) == CM730::SUCCESS)
		{
			MotionStatus::m_CurrentJoints.SetValue(id, value);
			MotionStatus::m_CurrentJoints.SetEnable(id, true);

#ifdef CHANGING_TEMPERATURE_LIMIT
            m_CM730->WriteByte(id, MX28::P_HIGH_LIMIT_TEMPERATURE, MX28::CUSTOM_TEMPERATURE_LIMIT, &error);
#endif

			if(DEBUG_PRINT == true)
				fprintf(stderr, "[%d] Success\n", value);
		}
		else
		{
			MotionStatus::m_CurrentJoints.SetEnable(id, false);

#ifdef CHANGING_TEMPERATURE_LIMIT
            m_CM730->WriteByte(id, MX28::P_HIGH_LIMIT_TEMPERATURE, MX28::CUSTOM_TEMPERATURE_LIMIT, &error);
#endif

			if(DEBUG_PRINT == true)
				fprintf(stderr, " Fail\n");
		}
	}

	CheckServoExistance();

	m_CalibrationStatus = 0;
	m_FBGyroCenter = 512;
	m_RLGyroCenter = 512;

	return true;
}

bool MotionManager::Reinitialize()
{
	m_ProcessEnable = false;

	m_CM730->DXLPowerOn();

	int value, error;
	for(int id=JointData::ID_R_SHOULDER_PITCH; id<JointData::NUMBER_OF_JOINTS; id++)
	{
		if(DEBUG_PRINT == true)
			fprintf(stderr, "ID:%d initializing...", id);
		
		if(m_CM730->ReadWord(id, MX28::P_PRESENT_POSITION_L, &value, &error) == CM730::SUCCESS)
		{
			MotionStatus::m_CurrentJoints.SetValue(id, value);
			MotionStatus::m_CurrentJoints.SetEnable(id, true);

			if(DEBUG_PRINT == true)
				fprintf(stderr, "[%d] Success\n", value);
		}
		else
		{
			MotionStatus::m_CurrentJoints.SetEnable(id, false);

			if(DEBUG_PRINT == true)
				fprintf(stderr, " Fail\n");
		}
	}

	CheckServoExistance();

	m_ProcessEnable = true;
	return true;
}

void MotionManager::StartLogging()
{
    char szFile[32] = {0,};

    int count = 0;
    while(1)
    {
        sprintf(szFile, "Logs/Log%d.csv", count);
        if(0 != access(szFile, F_OK))
            break;
        count++;
		if(count > 256) return;
    }

    m_LogFileStream.open(szFile, std::ios::out);
    for(int id = 1; id < JointData::NUMBER_OF_JOINTS; id++)
        m_LogFileStream << "ID_" << id << "_GP,ID_" << id << "_PP,";
    m_LogFileStream << "GyroFB,GyroRL,AccelFB,AccelRL,L_FSR_X,L_FSR_Y,R_FSR_X,R_FSR_Y" << std::endl;

    m_IsLogging = true;
}

void MotionManager::StopLogging()
{
    m_IsLogging = false;
    m_LogFileStream.close();
}

void MotionManager::LoadINISettings(minIni* ini)
{
    LoadINISettings(ini, OFFSET_SECTION);
}
void MotionManager::LoadINISettings(minIni* ini, const std::string &section)
{
    int ivalue = INVALID_VALUE;

    for(int i = 1; i < JointData::NUMBER_OF_JOINTS; i++)
    {
        char key[10];
        sprintf(key, "ID_%.2d", i);
        if((ivalue = ini->geti(section, key, INVALID_VALUE)) != INVALID_VALUE)  m_Offset[i] = ivalue;
    }
}
void MotionManager::SaveINISettings(minIni* ini)
{
    SaveINISettings(ini, OFFSET_SECTION);
}
void MotionManager::SaveINISettings(minIni* ini, const std::string &section)
{
    for(int i = 1; i < JointData::NUMBER_OF_JOINTS; i++)
    {
        char key[10];
        sprintf(key, "ID_%.2d", i);
        ini->put(section, key, m_Offset[i]);
    }
}

#define GYRO_WINDOW_SIZE    100
#define ACCEL_WINDOW_SIZE   30
#define MARGIN_OF_SD        2.0
void MotionManager::Process()
{
    if(m_ProcessEnable == false || m_IsRunning == true)
        return;

    m_IsRunning = true;

    // calibrate gyro sensor
    if(m_CalibrationStatus == 0 || m_CalibrationStatus == -1)
    {
        static int fb_gyro_array[GYRO_WINDOW_SIZE] = {512,};
        static int rl_gyro_array[GYRO_WINDOW_SIZE] = {512,};
        static int buf_idx = 0;

        if(buf_idx < GYRO_WINDOW_SIZE)
        {
            if(m_CM730->m_BulkReadData[CM730::ID_CM].error == 0)
            {
                fb_gyro_array[buf_idx] = m_CM730->m_BulkReadData[CM730::ID_CM].ReadWord(CM730::P_GYRO_Y_L);
                rl_gyro_array[buf_idx] = m_CM730->m_BulkReadData[CM730::ID_CM].ReadWord(CM730::P_GYRO_X_L);
                buf_idx++;
            }
        }
        else
        {
            double fb_sum = 0.0, rl_sum = 0.0;
            double fb_sd = 0.0, rl_sd = 0.0;
            double fb_diff, rl_diff;
            double fb_mean = 0.0, rl_mean = 0.0;

            buf_idx = 0;

            for(int i = 0; i < GYRO_WINDOW_SIZE; i++)
            {
                fb_sum += fb_gyro_array[i];
                rl_sum += rl_gyro_array[i];
            }
            fb_mean = fb_sum / GYRO_WINDOW_SIZE;
            rl_mean = rl_sum / GYRO_WINDOW_SIZE;

            fb_sum = 0.0; rl_sum = 0.0;
            for(int i = 0; i < GYRO_WINDOW_SIZE; i++)
            {
                fb_diff = fb_gyro_array[i] - fb_mean;
                rl_diff = rl_gyro_array[i] - rl_mean;
                fb_sum += fb_diff * fb_diff;
                rl_sum += rl_diff * rl_diff;
            }
            fb_sd = sqrt(fb_sum / GYRO_WINDOW_SIZE);
            rl_sd = sqrt(rl_sum / GYRO_WINDOW_SIZE);

            if(fb_sd < MARGIN_OF_SD && rl_sd < MARGIN_OF_SD)
            {
                m_FBGyroCenter = (int)fb_mean;
                m_RLGyroCenter = (int)rl_mean;
                m_CalibrationStatus = 1;
                if(DEBUG_PRINT == true)
                    fprintf(stderr, "FBGyroCenter:%d , RLGyroCenter:%d \n", m_FBGyroCenter, m_RLGyroCenter);
            }
            else
            {
                m_FBGyroCenter = 512;
                m_RLGyroCenter = 512;
                m_CalibrationStatus = -1;
            }
        }
    }

    if(m_CalibrationStatus == 1 && m_Enabled == true)
    {
        static int fb_array[ACCEL_WINDOW_SIZE] = {512,};
        static int buf_idx = 0;
        if(m_CM730->m_BulkReadData[CM730::ID_CM].error == 0)
        {
            MotionStatus::FB_GYRO = m_CM730->m_BulkReadData[CM730::ID_CM].ReadWord(CM730::P_GYRO_Y_L) - m_FBGyroCenter;
            MotionStatus::RL_GYRO = m_CM730->m_BulkReadData[CM730::ID_CM].ReadWord(CM730::P_GYRO_X_L) - m_RLGyroCenter;
            MotionStatus::RL_ACCEL = m_CM730->m_BulkReadData[CM730::ID_CM].ReadWord(CM730::P_ACCEL_X_L);
            MotionStatus::FB_ACCEL = m_CM730->m_BulkReadData[CM730::ID_CM].ReadWord(CM730::P_ACCEL_Y_L);
            fb_array[buf_idx] = MotionStatus::FB_ACCEL;
            if(++buf_idx >= ACCEL_WINDOW_SIZE) buf_idx = 0;
        }

        int sum = 0, avr = 512;
        for(int idx = 0; idx < ACCEL_WINDOW_SIZE; idx++)
            sum += fb_array[idx];
        avr = sum / ACCEL_WINDOW_SIZE;

        if(avr < MotionStatus::FALLEN_F_LIMIT)
            MotionStatus::FALLEN = FORWARD;
        else if(avr > MotionStatus::FALLEN_B_LIMIT)
            MotionStatus::FALLEN = BACKWARD;
        else
            MotionStatus::FALLEN = STANDUP;

///This should be the easiest way to keep the grippers and wrists out of trouble
// If no module is actively controlling them and overriding their positions,
//  then they default to 0[degrees]
#ifdef BOT_HAS_HANDS
        MotionStatus::m_CurrentJoints.SetAngle(JointData::ID_R_GRIPPER, 0);
        MotionStatus::m_CurrentJoints.SetAngle(JointData::ID_L_GRIPPER, 0);
#endif
#ifdef BOT_HAS_WRISTS
        MotionStatus::m_CurrentJoints.SetAngle(JointData::ID_R_WRIST, 0);
        MotionStatus::m_CurrentJoints.SetAngle(JointData::ID_L_WRIST, 0);
#endif
        if(m_Modules.size() != 0)
        {
            for(std::list<MotionModule*>::iterator i = m_Modules.begin(); i != m_Modules.end(); i++)
            {
                (*i)->Process();
                for(int id=JointData::ID_R_SHOULDER_PITCH; id<JointData::NUMBER_OF_JOINTS; id++)
                {
                    if((*i)->m_Joint.GetEnable(id) == true)
                    {
                        MotionStatus::m_CurrentJoints.SetSlope(id, (*i)->m_Joint.GetCWSlope(id), (*i)->m_Joint.GetCCWSlope(id));
                        MotionStatus::m_CurrentJoints.SetValue(id, (*i)->m_Joint.GetValue(id));

                        MotionStatus::m_CurrentJoints.SetPGain(id, (*i)->m_Joint.GetPGain(id));
                        MotionStatus::m_CurrentJoints.SetIGain(id, (*i)->m_Joint.GetIGain(id));
                        MotionStatus::m_CurrentJoints.SetDGain(id, (*i)->m_Joint.GetDGain(id));

#ifdef GRIPPER_EXPERIMENTAL
	/// Add speed and torque control?
                        MotionStatus::m_CurrentJoints.SetSpeedLim(id, (*i)->m_Joint.GetSpeedLim(id));
                        MotionStatus::m_CurrentJoints.SetTorqueLim(id, (*i)->m_Joint.GetTorqueLim(id));
#endif
                    }
                }
            }
        }

        int param[JointData::NUMBER_OF_JOINTS * MX28::PARAM_BYTES];
        int n = 0;
        int joint_num = 0;

#ifdef GRIPPER_EXPERIMENTAL
        int param_spd[JointData::NUMBER_OF_JOINTS * 2], param_trq[JointData::NUMBER_OF_JOINTS * 2];
        int m = 0, M = 0;
        int spd_num = 0, trq_num = 0;
#endif

        for(int id=JointData::ID_R_SHOULDER_PITCH; id<JointData::NUMBER_OF_JOINTS; id++)
        {
            if(MotionStatus::m_CurrentJoints.GetEnable(id) == true)
            {
                param[n++] = id;
#ifdef MX28_1024
                param[n++] = MotionStatus::m_CurrentJoints.GetCWSlope(id);
                param[n++] = MotionStatus::m_CurrentJoints.GetCCWSlope(id);
#else
                param[n++] = MotionStatus::m_CurrentJoints.GetDGain(id);
                param[n++] = MotionStatus::m_CurrentJoints.GetIGain(id);
                param[n++] = MotionStatus::m_CurrentJoints.GetPGain(id);
                param[n++] = 0;
#endif
                param[n++] = CM730::GetLowByte(MotionStatus::m_CurrentJoints.GetValue(id) + m_Offset[id]);
                param[n++] = CM730::GetHighByte(MotionStatus::m_CurrentJoints.GetValue(id) + m_Offset[id]);
                joint_num++;

#ifdef GRIPPER_EXPERIMENTAL
				int extras = 0;
                if ( (extras=MotionStatus::m_CurrentJoints.GetSpeedLim(id)) > JointData::SPEED_DEFAULT )
                {
                    param_spd[m++] = id;
                    param_spd[m++] = CM730::GetLowByte(extras);
                    param_spd[m++] = CM730::GetHighByte(extras);
                    spd_num++;
                }
                if ( (extras=MotionStatus::m_CurrentJoints.GetTorqueLim(id)) > JointData::TORQUE_DEFAULT )
                {
                    param_trq[M++] = id;
                    param_trq[M++] = CM730::GetLowByte(extras);
                    param_trq[M++] = CM730::GetHighByte(extras);
                    trq_num++;
                }
#endif
            }

            if(DEBUG_PRINT == true)
                fprintf(stderr, "ID[%d] : %d \n", id, MotionStatus::m_CurrentJoints.GetValue(id));
        }

        if(joint_num > 0)
#ifdef MX28_1024
            m_CM730->SyncWrite(MX28::P_CW_COMPLIANCE_SLOPE, MX28::PARAM_BYTES, joint_num, param);
#else
            m_CM730->SyncWrite(MX28::P_D_GAIN, MX28::PARAM_BYTES, joint_num, param);
#endif
#ifdef GRIPPER_EXPERIMENTAL
        if(spd_num > 0)
            m_CM730->SyncWrite(MX28::P_MOVING_SPEED_L, 2, spd_num, param_spd);
        if(trq_num > 0)
            m_CM730->SyncWrite(MX28::P_TORQUE_LIMIT_L, 2, trq_num, param_trq);
#endif
    }

    m_CM730->BulkRead();

    if(m_IsLogging)
    {
        for(int id = 1; id < JointData::NUMBER_OF_JOINTS; id++)
            m_LogFileStream << MotionStatus::m_CurrentJoints.GetValue(id) << "," << m_CM730->m_BulkReadData[id].ReadWord(MX28::P_PRESENT_POSITION_L) << ",";

        m_LogFileStream << m_CM730->m_BulkReadData[CM730::ID_CM].ReadWord(CM730::P_GYRO_Y_L) << ",";
        m_LogFileStream << m_CM730->m_BulkReadData[CM730::ID_CM].ReadWord(CM730::P_GYRO_X_L) << ",";
        m_LogFileStream << m_CM730->m_BulkReadData[CM730::ID_CM].ReadWord(CM730::P_ACCEL_Y_L) << ",";
        m_LogFileStream << m_CM730->m_BulkReadData[CM730::ID_CM].ReadWord(CM730::P_ACCEL_X_L) << ",";
        m_LogFileStream << m_CM730->m_BulkReadData[FSR::ID_L_FSR].ReadByte(FSR::P_FSR_X) << ",";
        m_LogFileStream << m_CM730->m_BulkReadData[FSR::ID_L_FSR].ReadByte(FSR::P_FSR_Y) << ",";
        m_LogFileStream << m_CM730->m_BulkReadData[FSR::ID_R_FSR].ReadByte(FSR::P_FSR_X) << ",";
        m_LogFileStream << m_CM730->m_BulkReadData[FSR::ID_R_FSR].ReadByte(FSR::P_FSR_Y) << ",";
        m_LogFileStream << std::endl;
    }

#ifdef BOT_HAS_HANDS
    if(MotionStatus::m_JointStatus.GetExists(JointData::ID_R_GRIPPER) == true)
    {
        MotionStatus::m_JointStatus.SetSpeedNow( JointData::ID_R_GRIPPER, m_CM730->m_BulkReadData[JointData::ID_R_GRIPPER].ReadWord(MX28::P_PRESENT_SPEED_L) );
        MotionStatus::m_JointStatus.SetTorqueNow( JointData::ID_R_GRIPPER, m_CM730->m_BulkReadData[JointData::ID_R_GRIPPER].ReadWord(MX28::P_PRESENT_LOAD_L) );
        MotionStatus::m_JointStatus.SetTemperature( JointData::ID_R_GRIPPER, m_CM730->m_BulkReadData[JointData::ID_R_GRIPPER].ReadWord(MX28::P_PRESENT_TEMPERATURE) );
        MotionStatus::m_JointStatus.SetErrors( JointData::ID_R_GRIPPER, m_CM730->m_BulkReadData[JointData::ID_R_GRIPPER].error );
    }

    if(MotionStatus::m_JointStatus.GetExists(JointData::ID_L_GRIPPER) == true)
    {
        MotionStatus::m_JointStatus.SetSpeedNow( JointData::ID_L_GRIPPER, m_CM730->m_BulkReadData[JointData::ID_L_GRIPPER].ReadWord(MX28::P_PRESENT_SPEED_L) );
        MotionStatus::m_JointStatus.SetTorqueNow( JointData::ID_L_GRIPPER, m_CM730->m_BulkReadData[JointData::ID_L_GRIPPER].ReadWord(MX28::P_PRESENT_LOAD_L) );
        MotionStatus::m_JointStatus.SetTemperature( JointData::ID_L_GRIPPER, m_CM730->m_BulkReadData[JointData::ID_L_GRIPPER].ReadWord(MX28::P_PRESENT_TEMPERATURE) );
        MotionStatus::m_JointStatus.SetErrors( JointData::ID_L_GRIPPER, m_CM730->m_BulkReadData[JointData::ID_L_GRIPPER].error );
    }
#endif

#ifdef BOT_HAS_WRISTS
    if(MotionStatus::m_JointStatus.GetExists(JointData::ID_R_WRIST) == true)
    {
        MotionStatus::m_JointStatus.SetSpeedNow( JointData::ID_R_WRIST, m_CM730->m_BulkReadData[JointData::ID_R_WRIST].ReadWord(MX28::P_PRESENT_SPEED_L) );
        MotionStatus::m_JointStatus.SetTorqueNow( JointData::ID_R_WRIST, m_CM730->m_BulkReadData[JointData::ID_R_WRIST].ReadWord(MX28::P_PRESENT_LOAD_L) );
        MotionStatus::m_JointStatus.SetTemperature( JointData::ID_R_WRIST, m_CM730->m_BulkReadData[JointData::ID_R_WRIST].ReadWord(MX28::P_PRESENT_TEMPERATURE) );
        MotionStatus::m_JointStatus.SetErrors( JointData::ID_R_WRIST, m_CM730->m_BulkReadData[JointData::ID_R_WRIST].error );
    }

    if(MotionStatus::m_JointStatus.GetExists(JointData::ID_L_WRIST) == true)
    {
        MotionStatus::m_JointStatus.SetSpeedNow( JointData::ID_L_WRIST, m_CM730->m_BulkReadData[JointData::ID_L_WRIST].ReadWord(MX28::P_PRESENT_SPEED_L) );
        MotionStatus::m_JointStatus.SetTorqueNow( JointData::ID_L_WRIST, m_CM730->m_BulkReadData[JointData::ID_L_WRIST].ReadWord(MX28::P_PRESENT_LOAD_L) );
        MotionStatus::m_JointStatus.SetTemperature( JointData::ID_L_WRIST, m_CM730->m_BulkReadData[JointData::ID_L_WRIST].ReadWord(MX28::P_PRESENT_TEMPERATURE) );
        MotionStatus::m_JointStatus.SetErrors( JointData::ID_L_WRIST, m_CM730->m_BulkReadData[JointData::ID_L_WRIST].error );
    }
#endif



    if(m_CM730->m_BulkReadData[CM730::ID_CM].error == 0)
        MotionStatus::BUTTON = m_CM730->m_BulkReadData[CM730::ID_CM].ReadByte(CM730::P_BUTTON);

    m_IsRunning = false;
}

void MotionManager::SetEnable(bool enable)
{
	m_Enabled = enable;
	if(m_Enabled == true)
		m_CM730->WriteWord(CM730::ID_BROADCAST, MX28::P_MOVING_SPEED_L, 0, 0);
}

void MotionManager::AddModule(MotionModule *module)
{
	module->Initialize();
	m_Modules.push_back(module);
}

void MotionManager::RemoveModule(MotionModule *module)
{
	m_Modules.remove(module);
}

void MotionManager::SetJointDisable(int index)
{
    if(m_Modules.size() != 0)
    {
        for(std::list<MotionModule*>::iterator i = m_Modules.begin(); i != m_Modules.end(); i++)
            (*i)->m_Joint.SetEnable(index, false);
    }
}

int MotionManager::CheckServoExistance(void)
{
    int count = 0;
    for(int id = 1; id < JointData::NUMBER_OF_JOINTS; id++)
    {
        int moe = 0;
        if (m_CM730->ReadWord(id, 0, &moe, 0)==CM730::SUCCESS)
        {
            MotionStatus::m_JointStatus.SetModel(id, moe);
            count++;
        }
        else
            MotionStatus::m_JointStatus.SetExists(id, 0);
    }
    return count;
}
