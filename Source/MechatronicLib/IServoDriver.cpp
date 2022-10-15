#include "IServoDriver.h"


bool IServoDriver::TorqueEnable(bool b_enable)
{
	torqueEnabled = b_enable;

	return true;
}