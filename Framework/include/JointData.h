/*
 *   JointData.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _JOINT_DATA_H_
#define _JOINT_DATA_H_

#define BOT_HAS_HANDS
//#define BOT_HAS_WRISTS

#include "AXM.h"
#include "MX28.h"

namespace Robot
{	
	class JointData  
	{
	public:
		enum
		{
			ID_R_SHOULDER_PITCH     = 1,
			ID_L_SHOULDER_PITCH     = 2,
			ID_R_SHOULDER_ROLL      = 3,
			ID_L_SHOULDER_ROLL      = 4,
			ID_R_ELBOW              = 5,
			ID_L_ELBOW              = 6,
			ID_R_HIP_YAW            = 7,
			ID_L_HIP_YAW            = 8,
			ID_R_HIP_ROLL           = 9,
			ID_L_HIP_ROLL           = 10,
			ID_R_HIP_PITCH          = 11,
			ID_L_HIP_PITCH          = 12,
			ID_R_KNEE               = 13,
			ID_L_KNEE               = 14,
			ID_R_ANKLE_PITCH        = 15,
			ID_L_ANKLE_PITCH        = 16,
			ID_R_ANKLE_ROLL         = 17,
			ID_L_ANKLE_ROLL         = 18,
			ID_HEAD_PAN             = 19,
			ID_HEAD_TILT            = 20,
#ifdef BOT_HAS_HANDS
			ID_R_GRIPPER            = 21,
			ID_L_GRIPPER            = 22,
#endif
#ifdef BOT_HAS_WRISTS
			ID_R_WRIST              = 23,
			ID_L_WRIST              = 24,
#endif
			NUMBER_OF_JOINTS
		};

		enum
		{
			SLOPE_HARD			= 16,
			SLOPE_DEFAULT		= 32,
			SLOPE_SOFT			= 64,
			SLOPE_EXTRASOFT		= 128
		};

		enum
		{
		    P_GAIN_DEFAULT      = 32,
		    I_GAIN_DEFAULT      = 0,
		    D_GAIN_DEFAULT      = 0
		};

		enum
		{
		    SPEED_DEFAULT       = -1,
		    SPEED_LOW           = 200,
		    SPEED_MID           = 512,
		    SPEED_MAX           = 1023
		};

		enum
		{
		    TORQUE_DEFAULT      = -1,
		    TORQUE_LOW          = 200,
		    TORQUE_MID          = 512,
		    TORQUE_MAX          = 1023
		};

	private:		

	protected:
		bool m_Enable[NUMBER_OF_JOINTS];
		int m_Value[NUMBER_OF_JOINTS];
		double m_Angle[NUMBER_OF_JOINTS];
		int m_CWSlope[NUMBER_OF_JOINTS];
		int m_CCWSlope[NUMBER_OF_JOINTS];
		int m_PGain[NUMBER_OF_JOINTS];
        int m_IGain[NUMBER_OF_JOINTS];
        int m_DGain[NUMBER_OF_JOINTS];

        int m_SpeedLim[NUMBER_OF_JOINTS];
        int m_TorqueLim[NUMBER_OF_JOINTS];

	public:
		JointData();
		~JointData();

        void SetEnable(int id, bool enable);
		void SetEnable(int id, bool enable, bool exclusive);
		void SetEnableHeadOnly(bool enable);
        void SetEnableHeadOnly(bool enable, bool exclusive);
		void SetEnableRightArmOnly(bool enable);
        void SetEnableRightArmOnly(bool enable, bool exclusive);
		void SetEnableLeftArmOnly(bool enable);
        void SetEnableLeftArmOnly(bool enable, bool exclusive);
		void SetEnableRightLegOnly(bool enable);
        void SetEnableRightLegOnly(bool enable, bool exclusive);
		void SetEnableLeftLegOnly(bool enable);
        void SetEnableLeftLegOnly(bool enable, bool exclusive);
		void SetEnableUpperBodyWithoutHead(bool enable);
        void SetEnableUpperBodyWithoutHead(bool enable, bool exclusive);
		void SetEnableLowerBody(bool enable);
        void SetEnableLowerBody(bool enable, bool exclusive);
		void SetEnableBodyWithoutHead(bool enable);
        void SetEnableBodyWithoutHead(bool enable, bool exclusive);
		void SetEnableBody(bool enable);
        void SetEnableBody(bool enable, bool exclusive);
#ifdef BOT_HAS_HANDS
		void SetEnableBodyWithoutHands(bool enable);
		void SetEnableBodyWithoutHands(bool enable, bool exclusive);
		void SetEnableHands(bool enable);
		void SetEnableHands(bool enable, bool exclusive);
#endif
#ifdef BOT_HAS_WRISTS
		void SetEnableBodyWithoutWrists(bool enable);
		void SetEnableBodyWithoutWrists(bool enable, bool exclusive);
		void SetEnableWrists(bool enable);
		void SetEnableWrists(bool enable, bool exclusive);
#endif
		bool GetEnable(int id);

		void SetValue(int id, int value);
		int GetValue(int id);

		void SetAngle(int id, double angle);
		double GetAngle(int id);

		void SetRadian(int id, double radian);
		double GetRadian(int id);

		void SetSlope(int id, int cwSlope, int ccwSlope);
		void SetCWSlope(int id, int cwSlope);
		int  GetCWSlope(int id);
		void SetCCWSlope(int id, int ccwSlope);
		int  GetCCWSlope(int id);

        void SetPGain(int id, int pgain) { m_PGain[id] = pgain; }
        int  GetPGain(int id)            { return m_PGain[id]; }
        void SetIGain(int id, int igain) { m_IGain[id] = igain; }
        int  GetIGain(int id)            { return m_IGain[id]; }
        void SetDGain(int id, int dgain) { m_DGain[id] = dgain; }
        int  GetDGain(int id)            { return m_DGain[id]; }

        /// Set/Get Speed and PWM limits
        void SetSpeedLim(int id, int speed) { m_SpeedLim[id] = speed; }
        int  GetSpeedLim(int id)            { return m_SpeedLim[id]; }
        void SetTorqueLim(int id, int torque) { m_TorqueLim[id] = torque; }
        int  GetTorqueLim(int id)            { return m_TorqueLim[id]; }
	};

	class JointFeedback
	{
	protected:
		int		m_Model[JointData::NUMBER_OF_JOINTS];

		double	m_AngleNow[JointData::NUMBER_OF_JOINTS];
		int		m_SpeedNow[JointData::NUMBER_OF_JOINTS];
		int		m_TorqueNow[JointData::NUMBER_OF_JOINTS];

		int		m_Temperature[JointData::NUMBER_OF_JOINTS];

		int		m_Errors[JointData::NUMBER_OF_JOINTS];

	public:
		JointFeedback()
		{
			for (int id=1; id<JointData::NUMBER_OF_JOINTS; id++)
			{
				m_AngleNow[id] = 0.0;
				m_SpeedNow[id] = 0;
				m_TorqueNow[id] = 0;
				m_Temperature[id] = 0;
				m_Model[id] = 0;
				m_Errors[id] = 0;
			}
		}

		/// Set/Get Angle, Speed, and PWM values
		void	SetAngleNow(int id, double pos)
		{
			if (	(m_Model[id]==DXL_MODELS::AX12) ||
					(m_Model[id]==DXL_MODELS::AX18) ||
					(m_Model[id]==DXL_MODELS::AX12W) )
				m_AngleNow[id] = AXM::Value2Angle(pos);
			else if ( (m_Model[id]==DXL_MODELS::MX28) )
				m_AngleNow[id] = MX28::Value2Angle(pos);
			else
				m_AngleNow[id] = pos/(1.0);
		}
		double	GetAngleNow(int id)					{ return m_AngleNow[id]; }
		void	SetSpeedNow(int id, int speed)		{ m_SpeedNow[id] = speed; }
		int		GetSpeedNow(int id)					{ return m_SpeedNow[id]; }
		void	SetTorqueNow(int id, int torque)	{ m_TorqueNow[id] = torque; }
		int		GetTorqueNow(int id)				{ return m_TorqueNow[id]; }

		void	SetTemperature(int id, int tempy)	{ m_Temperature[id] = tempy; }
		int		GetTemperature(int id)				{ return m_Temperature[id]; }

		void	SetModel(int id, int moe)			{ m_Model[id] = moe; }
		int		GetModel(int id)					{ return m_Model[id]; }
		void	SetErrors(int id, int err)			{ m_Errors[id] = err; }
		int		GetErrors(int id)					{ return m_Errors[id]; }

	};
}

#endif
