#include "IXM430Driver.h"



IXM430Driver::IXM430Driver(Returns& e_returns):
	IServoDriver(e_returns)
{
	addrOperatingMode = 11;
	addrCurrentLimit = 38;
	addrGoalCurrent = 102;
	addrTorqueEnable = 64;
	addrGoalPosition = 116;
	addrVelocityLimit = 44;
	addrGoalVelocity = 104;
	addrCurrPosition = 132;
	addrHWError = 70;
	addrMovingStatus = 123;
	addrIsMoving = 122;
	//For extended position, Min and Max positon limits can be used for custom registers
	addrHomeOffset = 20;
	addrMaxPosition = 48;
	addrMinPosition = 52;
	addrSecondaryId = 12;
	addrStatusLED = 65;
}

bool IXM430Driver::GetVelocity(uint16_t& v_velocity)
{
	uint32_t velocity_temp;
	if (!pConnection->ReadBytes(deviceId, servoName, addrVelocityLimit, velocity_temp))
	{
		eReturns.Throw(servoName, "Could Not Set Velocity");
		return false;
	}
	v_velocity = static_cast<uint16_t>(velocity_temp);

	return true;
}
bool IXM430Driver::SetVelocity(uint16_t v_velocity)
{
	//Set EEProm Velocity
	uint16_t temp_velocity;
	if (!GetVelocity(temp_velocity))
	{
		eReturns.Throw(servoName, "Could Not Set Velocity");
		return false;
	}

	if (temp_velocity != v_velocity)
	{
		//Make sure torque is disabled
		if (!TorqueEnable(false))
			return false;

		DebugOut("Velocity has not been preset.. setting\n");
		if (!pConnection->WriteBytes(deviceId, servoName, addrVelocityLimit, v_velocity))
		{
			eReturns.Throw(servoName, "Could Not Set Velocity");
			return false;
		}
	}

	//Set RAM velocity
	if (!pConnection->WriteBytes(deviceId, servoName, addrGoalVelocity, v_velocity))
	{
		eReturns.Throw(servoName, "Could Not Set Velocity");
		return false;
	}
	
	return true;
}

bool IXM430Driver::GetCurrentLimit(uint16_t& current_limit)
{
	uint32_t torque_limit_temp;
	if (!pConnection->ReadBytes(deviceId, servoName, addrCurrentLimit, torque_limit_temp, true))
	{
		eReturns.Throw(servoName, "Could Not Set Torque Limit");
		return false;
	}
	current_limit = static_cast<uint16_t>(torque_limit_temp);

	return true;
}
bool IXM430Driver::SetCurrentLimit(uint16_t current_limit)
{
	//Set EEProm Limit
	uint16_t temp_limit;
	if (!GetCurrentLimit(temp_limit))
	{
		eReturns.Throw(servoName, "Could Not Set Current Limit");
		return false;
	}

	if (temp_limit != current_limit)
	{
		//Make sure torque is disabled
		if (!TorqueEnable(false))
			return false;

		DebugOut("Current Limit has not been preset.. setting\n");
		if (!pConnection->WriteBytes(deviceId, servoName, addrCurrentLimit, current_limit, true))
		{
			eReturns.Throw(servoName, "Could Not Set Current Limit");
			return false;
		}
	}

	//Set RAM current_limit
	if (!pConnection->WriteBytes(deviceId, servoName, addrGoalCurrent, current_limit, true))
	{
		eReturns.Throw(servoName, "Could Not Set Current Limit");
		return false;
	}

	return true;
}

bool IXM430Driver::GetStatusLED(bool& led_status)
{
	uint8_t led_status_unsigned;
	if (!pConnection->ReadByte(deviceId, servoName, addrStatusLED, led_status_unsigned))
	{
		eReturns.Throw(servoName, "Could not Get init status");
		return false;
	}

	led_status = static_cast<bool>(led_status_unsigned);
	return true;
}
bool IXM430Driver::SetStatusLED(bool led_status)
{
	//Make sure torque is disabled
	if (!TorqueEnable(false))
		return false;

	//Set Status LED
	if (!pConnection->WriteByte(deviceId, servoName, addrStatusLED, static_cast<uint8_t>(led_status)))
	{
		eReturns.Throw(servoName, "Could not Set Status LED");
		return false;
	}

	DebugOut("Servo: %s Status LED set to %s\n", servoName, led_status ? "ON" : "OFF");
	return true;
}

bool IXM430Driver::GetOperatingMode(OperatingMode& operating_mode)
{
	// Change Operating Mode 
	uint8_t op_mode;
	if (!pConnection->ReadByte(deviceId, servoName, addrOperatingMode, op_mode))
	{
		eReturns.Throw(servoName, "Could not Set Operating Mode");
		return false;
	}
	operating_mode = static_cast<OperatingMode>(op_mode);

	return true;
}
bool IXM430Driver::SetOperatingMode(OperatingMode operating_mode)
{
	//Disable Dynamixel Torque
	if (!TorqueEnable(false))
		return false;

	// Change Operating Mode 
	if (!pConnection->WriteByte(deviceId, servoName, addrOperatingMode, operating_mode))
	{
		eReturns.Throw(servoName, "Could not Set Operating Mode");
		return false;
	}

	return true;
}

bool IXM430Driver::CheckPower()
{
	bool led_status;
	if (!GetStatusLED(led_status))
	{
		eReturns.Throw(servoName, "Could not Get LED status");
		return false;
	}

	if (!led_status)
	{
		eReturns.Throw(servoName, "Power may have tripped");
		return false;
	}
	return true;
}
bool IXM430Driver::WaitForStop()
{
	bool b_moving = true;
	for (int i = 0; i < 1000 && b_moving; i++) //Have some limiting counter
	{
		if (!IsMoving(b_moving))
		{
			eReturns.Throw(servoName, "Could Not Wait for Stop");
			return false;
		}
		pConnection->CommandDelay(50);
		if (!CheckPower())
		{
			return false;
		}
		//Check if Motor is Stalled
		///CommandDelay(50);
		///bool motor_stalled = false;
		///if (!CheckMotorStall(motor_stalled))
		///{
		///	return false;
		///}
		///if (motor_stalled)
		///{
		///	return false;
		///}
		pConnection->CommandDelay(50);
	}
	return true;
}
bool IXM430Driver::IsMoving(bool &b_moving)
{
	uint8_t moving_state;
	if (!pConnection->ReadByte(deviceId, servoName, addrIsMoving, moving_state))
		return false;

	b_moving = (moving_state == 1);

	return true;
}
bool IXM430Driver::CutTorque()
{
	if (!WaitForStop())
		return false;
	if (!TorqueEnable(false))
		return false;

	DebugOut("Servo: %s Torque Off\n", servoName);
	return true;
}

bool IXM430Driver::ResetEPMode()
{
	//Disable Dynamixel Torque
	if (!TorqueEnable(false))
		return false;

	if (!SetOperatingMode(IXM430Driver::OperatingMode::PositionControl)) //Reset position to value between 0 and 4096 by changing operating mode to PositionControl
		return false;

	if (!SetOperatingMode(IXM430Driver::OperatingMode::CurrentBasePositionControl)) //Change to extended position control
		return false;

	DebugOut("Servo: %s Reset Rev Zone\n", servoName);
	return true;
}
bool IXM430Driver::CheckMotorStall(bool& b_stalled)
{
	std::vector<MovingStatus> moving_status;
	if (!GetMovingStatus(moving_status))
		return false;

	b_stalled = false;
	for (auto& v_status : moving_status)
	{
		if (v_status == MovingStatus::FollowingError)
			b_stalled = true;
	}

	return true;
}
bool IXM430Driver::RotateUntilStall(bool b_cw)
{
	DebugOut("Servo: %s Rotating Until Stall Detected\n", servoName);

	//Get Position
	GetPosition();

	//Make Sure motor is stopped
	if (!WaitForStop())
		return false;

	//Make sure torque is enabled
	if (!TorqueEnable(true))
		return false;

	bool b_stalled = false;
	int v_position = storedPosition;
	for (int i = 0; i <= 2000, !b_stalled; i++) //Have some limiting counter
	{
		v_position = b_cw ? v_position + 30 : v_position - 30;
		//Set Position
		if (!MoveToPosition(v_position))
			return false;
		if (!CheckMotorStall(b_stalled))
			return false;
		if (i == 2000)
		{
			eReturns.Throw(servoName, "Could not hit limit during rotation until stall");
			return false;
		}
	}

	//Turn off servo so it doesn't finish rotation
	TorqueEnable(false);

	return true;
}
bool IXM430Driver::GetErrors(std::vector<ErrorCode>& error_codes)
{
	// Read present position
	uint8_t error_code;
	if (!pConnection->ReadByte(deviceId, servoName, addrHWError, error_code))
	{
		eReturns.Throw(servoName, "Could not Get errors");
		return false;
	}

	//Create a vector of error codes
	error_codes.clear();
	if (error_code & 0b00000001)
		error_codes.push_back(ErrorCode::InputVoltage);
	if (error_code & 0b00000100)
		error_codes.push_back(ErrorCode::Overheating);
	if (error_code & 0b00001000)
		error_codes.push_back(ErrorCode::EncoderError);
	if (error_code & 0b00010000)
		error_codes.push_back(ErrorCode::ElectricalShock);
	if (error_code & 0b00100000)
		error_codes.push_back(ErrorCode::Overload);

	return true;
}
bool IXM430Driver::GetMovingStatus(std::vector<MovingStatus>& moving_status)
{
	// Read present position
	uint8_t moving_byte;
	if (!pConnection->ReadByte(deviceId, servoName, addrMovingStatus, moving_byte))
	{
		eReturns.Throw(servoName, "Could not Get moving status");
		return false;
	}

	//Create a vector of error codes
	moving_status.clear();
	if (moving_byte & 0b00000001)
		moving_status.push_back(MovingStatus::InPosition);
	if (moving_byte & 0b00000010)
		moving_status.push_back(MovingStatus::ProfileOngoing);
	if (moving_byte & 0b00001000)
		moving_status.push_back(MovingStatus::FollowingError);

	return true;
}

bool IXM430Driver::MoveToPosition(int v_position)
{
	//Set Position
	if (!pConnection->WriteBytes(deviceId, servoName, addrGoalPosition, v_position))
	{
		eReturns.Throw(servoName, "Could Not Move to Position");
		return false;
	}

	return true;
}
bool IXM430Driver::TorqueEnable(bool b_enable)
{
	if (!pConnection->WriteByte(deviceId, servoName, addrTorqueEnable, b_enable ? 1 : 0))
	{
		eReturns.Throw(servoName, "Could Not Enable Torque");
		return false;
	}

	torqueEnabled = b_enable;

	return true;
}