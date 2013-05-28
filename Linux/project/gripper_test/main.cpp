#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <iostream>

#include "LinuxDARwIn.h"

#define U2D_DEV_NAME        "/dev/ttyUSB0"

using namespace Robot;

void change_current_dir()
{
    char exepath[1024] = {0};
    if(readlink("/proc/self/exe", exepath, sizeof(exepath)) != -1)
        chdir(dirname(exepath));
}

int main(void)
{
    printf( "\n===== Gripper Class Test for DARwIn =====\n\n");

    change_current_dir();

	//////////////////// Framework Initialize ////////////////////////////
	LinuxCM730 linux_cm730(U2D_DEV_NAME);
	CM730 cm730(&linux_cm730);
//    cm730.DEBUG_PRINT = true;
	if(MotionManager::GetInstance()->Initialize(&cm730) == false)
	{
		printf("Fail to initialize Motion Manager!\n");
			return 0;
	}
	MotionManager::GetInstance()->AddModule((MotionModule*)Gripper::GetRight());
	MotionManager::GetInstance()->AddModule((MotionModule*)Gripper::GetLeft());
    LinuxMotionTimer *motion_timer = new LinuxMotionTimer(MotionManager::GetInstance());

//	std::cout << "My ID is: " << Gripper::GetRight()->GetID() << std::endl;
	Gripper::GetRight()->Initialize();
	Gripper::GetLeft()->Initialize();

    motion_timer->Start();
	sleep(5);

	MotionStatus::m_CurrentJoints.SetEnableBody(false);
	MotionStatus::m_CurrentJoints.SetEnable(JointData::ID_R_GRIPPER, true);
	MotionStatus::m_CurrentJoints.SetEnable(JointData::ID_L_GRIPPER, true);
	MotionManager::GetInstance()->SetEnable(true);
	/////////////////////////////////////////////////////////////////////

	Gripper::GetRight()->m_Joint.SetEnableBody(false);
	Gripper::GetRight()->m_Joint.SetEnable(JointData::ID_R_GRIPPER, true);

	Gripper::GetRight()->m_Joint.SetPGain(JointData::ID_R_GRIPPER, 8);

	Gripper::GetLeft()->m_Joint.SetEnableBody(false);
	Gripper::GetLeft()->m_Joint.SetEnable(JointData::ID_L_GRIPPER, true);

    while(1)
    {
		Gripper::GetRight()->Squeeze(0.2);
		std::cout << "Present angle (R): " << Gripper::GetRight()->GetAngleNow() << std::endl;
		Gripper::GetLeft()->Squeeze(0.2);
		std::cout << "Present angle (L): " << Gripper::GetLeft()->GetAngleNow() << std::endl;
		sleep(5);

		Gripper::GetRight()->Spread(0.3);
		std::cout << "Present angle (R): " << Gripper::GetRight()->GetAngleNow() << std::endl;
		Gripper::GetLeft()->Spread(0.3);
		std::cout << "Present angle (L): " << Gripper::GetLeft()->GetAngleNow() << std::endl;
		sleep(5);
		
    }

    return 0;
}
