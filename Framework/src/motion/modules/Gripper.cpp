
#include <stdio.h>
#include "MX28.h"
#include "CM730.h"
#include "MotionStatus.h"
#include "Gripper.h"
#include <math.h>
#include <iostream>

using namespace Robot;

Gripper* Gripper::_right = new Gripper(JointData::ID_R_GRIPPER);
Gripper* Gripper::_left = new Gripper(JointData::ID_L_GRIPPER);

Gripper::Gripper(int id)
: _id(0)
, _closedLimit(20.0)
, _openLimit(-90.0)
, _neutral(0.0)
, _current(0.0)
, _torque(0.25)
, _d_torque(false)
{
	if (id > 20)
	{
		_id = id;
		m_Joint.SetEnable(_id, true, true);
	}
}

Gripper::~Gripper()
{
}

void Gripper::Bump()
{
	if (_torque < 0.0)
		_torque *= -1.0;
	if (_torque > 1.0)
		_torque = 1.0;

	if (_closedLimit > _openLimit)
	{
		if (_current > _closedLimit)
			_current = _closedLimit;
		else if (_current < _openLimit)
			_current = _openLimit;
	}
	else
	{
		if (_current < _closedLimit)
			_current = _closedLimit;
		else if (_current > _openLimit)
			_current = _openLimit;
	}
}

void Gripper::Initialize()
{
	_current = MotionStatus::m_CurrentJoints.GetAngle(_id);

	MoveToNeutral(0.1);
}

void Gripper::LoadINISettings(minIni* ini)
{
	LoadINISettings(ini, GRIPPER_SECTION);
}

void Gripper::LoadINISettings(minIni* ini, const std::string &section)
{
	double value = INVALID_VALUE;

	if((value = ini->getd(section,	"open_limit",		INVALID_VALUE)) != INVALID_VALUE)
		_openLimit = value;
	if((value = ini->getd(section,	"closed_limit",		INVALID_VALUE)) != INVALID_VALUE)
		_closedLimit = value;
	if((value = ini->getd(section,	"torque_limit",		INVALID_VALUE)) != INVALID_VALUE)
		_torque = value;
	if((value = ini->getd(section,	"neutral_position",	INVALID_VALUE)) != INVALID_VALUE)
		_neutral = value;
}

void Gripper::SaveINISettings(minIni* ini)
{
	SaveINISettings(ini, GRIPPER_SECTION);
}

void Gripper::SaveINISettings(minIni* ini, const std::string &section)
{
	ini->put(section,	"open_limit",		_openLimit);
	ini->put(section,	"closed_limit",		_closedLimit);
	ini->put(section,	"torque_limit",		_torque);
	ini->put(section,	"neutral_position",	_neutral);
}

double Gripper::GetTorqueNow()
{
	int tempy = MotionStatus::m_JointStatus.GetTorqueNow(_id);
	if ( (tempy >= 0) && (tempy < (1<<11)) )
	{
	    int mag = (tempy&(0x01FF));
	    int dir = (tempy&(1<<10));

		if (dir > 0)	// CCW Load (closing: positive angle rotation)
			return (mag/1023.0);
		else			// CW Load (opening: negative angle rotation)
			return -(mag/1023.0);
	}

	return -2.0;
}

double Gripper::GetSpeedNow()
{
	int tempy = MotionStatus::m_JointStatus.GetSpeedNow(_id);
	if ( (tempy >= 0) && (tempy < (1<<11)) )
	{
	    int mag = (tempy&(0x01FF));
	    int dir = (tempy&(1<<10));

		if (dir > 0)	// CCW Load (closing: positive angle rotation)
			return (mag/1023.0);
		else			// CW Load (opening: negative angle rotation)
			return -(mag/1023.0);
	}

	return -2.0;
}

double Gripper::SetTorqueLimit(double torque)
{
	if ( fabs(_torque - torque) > CONTROL_RESOLUTION )
	{
		_torque = torque;
		_d_torque = true;
	}

	Bump();
	return _torque;
}

void Gripper::MoveToNeutral()
{
	MoveToAngle(_neutral);
}

void Gripper::MoveToNeutral(double torque)
{
	MoveToAngle(_neutral, torque);
}

void Gripper::MoveToOpen()
{
	MoveToAngle(_openLimit);
}

void Gripper::MoveToOpen(double torque)
{
	MoveToAngle(_openLimit, torque);
}

void Gripper::MoveToClosed()
{
	MoveToAngle(_closedLimit);
}

void Gripper::MoveToClosed(double torque)
{
	MoveToAngle(_closedLimit, torque);
}


double Gripper::MoveByAngle(double delta)
{
	_current += delta;

	Bump();
	return _current;
}

double Gripper::MoveByAngle(double delta, double torque)
{
	_current += delta;
	
	if ( fabs(_torque - torque) > CONTROL_RESOLUTION )
	{
		_torque = torque;
		_d_torque = true;
	}

	Bump();
	return _current;
}

void Gripper::MoveToAngle(double angle)
{
	_current = angle;

	Bump();
}

void Gripper::MoveToAngle(double angle, double torque)
{
	_current = angle;
	
	if ( fabs(_torque - torque) > CONTROL_RESOLUTION )
	{
		_torque = torque;
		_d_torque = true;
	}

	Bump();
}

double Gripper::Squeeze(double torque)
{
	int overTorque = 0;
	double tor = this->GetTorqueNow();
	double torlim = (torque*1.0);
	double tormov = (torque*1.2);
	double Astart=0.0;
	while ( (overTorque<50) && (_current<_closedLimit) )
	{
		MoveToAngle(_current+1.0, tormov);
		tor = this->GetTorqueNow();
		fprintf(stderr, "\'Load\': %0.2f\n", tor);
		if (tor > torlim)
		{
			if (overTorque==0)
			{
				Astart = _current;
			}
			overTorque++;
		}
		else
			overTorque = 0;

		usleep(50000);
	}
	if (overTorque>0)
		_current = (Astart+1.0);
	MoveToAngle(_current, torque);
	tor = this->GetTorqueNow();
	
	fprintf(stderr, "Stopped squeezing at %0.2f[d] and %0.2f[PWM]\n", _current, tor);
	return _current;
}

double Gripper::Spread(double torque)
{
	int overTorque = 0;
	double tor = this->GetTorqueNow();
	double torlim = (torque*-1.0);
	double tormov = (torque*1.2);
	double Astart=0.0;
	while ( (overTorque<50) && (_current>_openLimit) )
	{
		MoveToAngle(_current-1.0, tormov);
		tor = this->GetTorqueNow();
		fprintf(stderr, "\'Load\': %0.2f\n", tor);
		if (tor < torlim)
		{
			if (overTorque==0)
			{
				Astart = _current;
			}
			overTorque++;
		}
		else
			overTorque = 0;

		usleep(50000);
	}
	if (overTorque>0)
		_current = (Astart-1.0);
	MoveToAngle(_current, torque);
	tor = this->GetTorqueNow();
	
	fprintf(stderr, "Stopped spreading at %0.2f[d] and %0.2f[PWM]\n", _current, tor);
	return _current;
}

void Gripper::Process()
{
	if (_id==0)
		return;

	if(m_Joint.GetEnable(_id) == true)
	{
		m_Joint.SetAngle(_id, _current);

		if (_d_torque)
		{
			m_Joint.SetTorqueLim( _id, (int)(1023*_torque) );
			_d_torque = false;
		}
		else
		{
			m_Joint.SetTorqueLim( _id, JointData::TORQUE_DEFAULT );
		}
	}
}
