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
//	MotionManager::GetInstance()->AddModule((MotionModule*)Gripper::GetLeft());
    LinuxMotionTimer *motion_timer = new LinuxMotionTimer(MotionManager::GetInstance());
    motion_timer->Start();

	std::cout << "My ID is: " << Gripper::GetRight()->GetID() << std::endl;
	Gripper::GetRight()->Initialize();
	sleep(2);
//	std::cout << "My ID is: " << Gripper::GetLeft()->GetID() << std::endl;
//	Gripper::GetLeft()->Initialize();
//	sleep(2);

	MotionStatus::m_CurrentJoints.SetEnableBody(false);
	MotionStatus::m_CurrentJoints.SetEnable(JointData::ID_R_GRIPPER, true);
//	MotionStatus::m_CurrentJoints.SetEnable(JointData::ID_L_GRIPPER, true);
	MotionManager::GetInstance()->SetEnable(true);
	/////////////////////////////////////////////////////////////////////

	Gripper::GetRight()->m_Joint.SetEnableBody(false);
	Gripper::GetRight()->m_Joint.SetEnable(JointData::ID_R_GRIPPER, true);
//	Gripper::GetLeft()->m_Joint.SetEnableBody(false);
//	Gripper::GetLeft()->m_Joint.SetEnable(JointData::ID_L_GRIPPER, true);

	Gripper::GetRight()->m_Joint.SetPGain(JointData::ID_R_GRIPPER, 8);

    while(1)
    {
/*    	std::cout << "Moving to neutral\n";
        Gripper::GetLeft()->MoveToNeutral(.3);
        sleep(2);
        std::cout << "\'Load\': " << Gripper::GetLeft()->GetTorqueNow() << std::endl;
    	std::cout << "Moving to open\n";
        Gripper::GetLeft()->MoveToOpen(1.0);
        sleep(2);
        std::cout << "\'Load\': " << Gripper::GetLeft()->GetTorqueNow() << std::endl;
    	std::cout << "Moving to closed\n";
        Gripper::GetLeft()->MoveToClosed(.01);
        sleep(2);
        std::cout << "\'Load\': " << Gripper::GetLeft()->GetTorqueNow() << std::endl;
*/
/*
    	std::cout << "Moving to neutral\n";
        Gripper::GetRight()->MoveToNeutral(.3);
        sleep(2);
        std::cout << "\'Load\': " << Gripper::GetRight()->GetTorqueNow() << std::endl;
    	std::cout << "Moving to open\n";
        Gripper::GetRight()->MoveToOpen(1.0);
        sleep(2);
        std::cout << "\'Load\': " << Gripper::GetRight()->GetTorqueNow() << std::endl;
    	std::cout << "Moving to closed\n";
        Gripper::GetRight()->MoveToClosed(.01);
        sleep(2);
        std::cout << "\'Load\': " << Gripper::GetRight()->GetTorqueNow() << std::endl;
*/
/*
        double countChocula = 0.0;
        while(countChocula<15.0)
        {
            Gripper::GetRight()->MoveToAngle(countChocula);
	        std::cout << "\'Load\': " << Gripper::GetRight()->GetTorqueNow() << std::endl;
            countChocula += 1.0;
            usleep(100000);
        }
        while(countChocula>-80.0)
        {
            Gripper::GetRight()->MoveToAngle(countChocula);
	        std::cout << "\'Load\': " << Gripper::GetRight()->GetTorqueNow() << std::endl;
            countChocula -= 1.0;
            usleep(100000);
        }
        while(countChocula<0.0)
        {
            Gripper::GetRight()->MoveToAngle(countChocula);
	        std::cout << "\'Load\': " << Gripper::GetRight()->GetTorqueNow() << std::endl;
            countChocula += 1.0;
            usleep(100000);
        }
*/
		Gripper::GetRight()->MoveToOpen();
		sleep(5);
		Gripper::GetRight()->Squeeze(0.2);
		sleep(5);
		Gripper::GetRight()->Spread(0.2);
		sleep(5);

    }

    return 0;
}
