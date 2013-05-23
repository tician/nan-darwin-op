

#ifndef _GRIPPER_H_
#define _GRIPPER_H_

#include <string.h>

#include "minIni.h"
#include "MotionModule.h"

#define GRIPPER_SECTION		"Gripper"
#define INVALID_VALUE		-1024.0
#define CONTROL_RESOLUTION	0.01

namespace Robot
{
	class Gripper : public MotionModule
	{
	private:
		static Gripper *_right;
		static Gripper *_left;

		int _id;
		double _closedLimit;
		double _openLimit;
		double _neutral;
		double _current;
		
		double _torque;
		double _speed;
		bool _d_torque;
		bool _d_speed;

		Gripper(int id);
		
		void Bump();

	public:
//  Methinks we got to use one instance per class or we get seg faults
		static Gripper* GetRight()		{ return _right; }
		static Gripper* GetLeft()		{ return _left; }

		~Gripper();

		void Initialize();
		void Process();

		int GetID() 					{ return _id; }

		double GetClosedLimitAngle()	{ return _closedLimit; }
		double GetOpenLimitAngle()		{ return _openLimit; }
		double GetNeutralAngle()		{ return _neutral; }
		double GetCurrentAngle()		{ return _current; }
		double GetTorqueLimit()			{ return _torque; }
		double GetSpeedLimit()			{ return _speed; }
		
		double GetTorqueNow();
		double GetSpeedNow();

		double SetTorqueLimit(double torque);
		double SetSpeedLimit(double speed);

		void MoveToNeutral();
		void MoveToNeutral(double torque);
		void MoveToOpen();
		void MoveToOpen(double torque);
		void MoveToClosed();
		void MoveToClosed(double torque);
		double MoveByAngle(double delta);
		double MoveByAngle(double delta, double torque);
		void MoveToAngle(double angle);
		void MoveToAngle(double angle, double torque);

        void LoadINISettings(minIni* ini);
        void LoadINISettings(minIni* ini, const std::string &section);
        void SaveINISettings(minIni* ini);
        void SaveINISettings(minIni* ini, const std::string &section);
	};
}

#endif
