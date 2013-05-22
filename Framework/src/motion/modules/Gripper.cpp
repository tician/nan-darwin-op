
#include <stdio.h>
#include "MX28.h"
#include "CM730.h"
#include "MotionStatus.h"
#include "Gripper.h"

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
, _speed(0.25)
, _d_torque(false)
, _d_speed(false)
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
	int error = 0;

	if (_d_torque)
	{
		if (_torque < 0.0)
			_torque *= -1.0;
		if (_torque > 1.0)
			_torque = 1.0;
		if (m_Joint.GetEnable(_id) == true)
		    m_Joint.SetTorqueLim( id, (int)(1023*_torque) );
	}
	if (_d_speed)
	{
		if (_speed < 0.0)
			_speed *= -1.0;
		if (_speed > 1.0)
			_speed = 1.0;
		if(m_Joint.GetEnable(_id) == true)
		    m_Joint.SetSpeedLim( id, (int)(1023*_speed) );
	}
	_d_torque = false;
	_d_speed = false;

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
	if((value = ini->getd(section,	"speed_limit",		INVALID_VALUE)) != INVALID_VALUE)
		_speed = value;
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
	ini->put(section,	"speed_limit",		_speed);
	ini->put(section,	"torque_limit",		_torque);
	ini->put(section,	"neutral_position",	_neutral);
}

double Gripper::GetTorqueNow()
{
	MotionStatus::m_JointStatus

	return -1.0;
}

double Gripper::SetTorqueLimit(double torque)
{
	if ( abs(_torque - torque) > CONTROL_RESOLUTION )
	{
		_torque = torque;
		_d_torque = true;
	}

	Bump();
	return _torque;
}

double Gripper::SetSpeedLimit(double speed)
{
	if ( abs(_speed - speed) > CONTROL_RESOLUTION )
	{
		_speed = speed;
		_d_speed = true;
	}

	Bump();
	return _speed;
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
	
	if ( abs(_torque - torque) > CONTROL_RESOLUTION )
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
	
	if ( abs(_torque - torque) > CONTROL_RESOLUTION )
	{
		_torque = torque;
		_d_torque = true;
	}

	Bump();
}

void Gripper::Process()
{
	if (_id==0)
		return;


	if(m_Joint.GetEnable(_id) == true)
		m_Joint.SetAngle(_id, _current);
}