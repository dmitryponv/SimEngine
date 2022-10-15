#include "XL320Driver.h"

XL320Driver::XL320Driver(Returns& e_returns, int device_id, std::string servo_name, float steps_per_unit, bool clock_wise,
	uint16_t current_limit, uint16_t max_velocity, float min_range, float max_range) : IServoDriver(e_returns)
{
	deviceId = device_id;
	servoName = servo_name;
	stepsPerUnit = steps_per_unit;
	clockWise = clock_wise;
	currentLimit = current_limit;
	maxVelocity = max_velocity;
	minSteps = static_cast<int>(min_range*steps_per_unit);
	maxSteps = static_cast<int>(max_range*steps_per_unit);

	addrOperatingMode = 2;
	addrCurrentLimit = 15;
	addrGoalCurrent = 35;
	addrTorqueEnable = 24;
	addrGoalPosition = 30;
	addrGoalVelocity = 32;
	addrCurrPosition = 37;
	addrHWError = 50;
	addrIsMoving = 49;
	//For extended position, Min and Max positon limits can be used for custom registers
	addrMaxPosition = 8;
	addrMinPosition = 6;
	addrStatusLED = 25;
	addrMaxVoltageLimit = 14;
}

bool XL320Driver::InitServo(std::shared_ptr<Serializer> p_serializer, bool& servo_needs_cal)
{
	pSerializer = p_serializer;
	if (!portConnected)
	{
		eReturns.Throw(servoName, "Com Port not Connected");
		return false;
	}

	//Turn ON Status LED
	if (!SetStatusLED(true))
	{
		eReturns.Throw(servoName, "Cannot Set LED status");
		return false;
	}

	//Read Serialization file
	if (pSerializer == nullptr)
	{
		eReturns.Throw(servoName, "No Serialization file");
		servo_needs_cal = true;
		return false;
	}
	else if (!pSerializer->LoadInt(servoName + "-StoredPosition", storedPosition))
	{
		//Do not error out if stored position is not loaded, just request calibration
		eReturns.Throw(servoName, "Serialization file does not have StoredPosition saved");
		servo_needs_cal = true;
		return true;
	}
	else if (!pSerializer->LoadInt(servoName + "-HomeOffset", homeOffset))
	{
		//Do not error out if stored position is not loaded, just request calibration
		eReturns.Throw(servoName, "Serialization file does not have homeOffset saved");
		servo_needs_cal = true;
		return true;
	}
	else
		servo_needs_cal = false;
	
	GetPosition();

	//Need need to check that the stored position is not beyond the limit of the servo
	if (storedPosition > (maxSteps + 100) || storedPosition < (minSteps - 100))
	{
		eReturns.Throw(servoName, "Servo Outside allowed position, position = " + std::to_string(storedPosition));
		servo_needs_cal = true;
		return false;
	}	

	//Set Current Limit and Max Velocity
	if (!SetCurrentLimit(currentLimit))
	{
		eReturns.Throw(servoName, "Could Not Set Current Limit");
		return false;
	}
	if (!SetVelocity(currentLimit))
	{
		eReturns.Throw(servoName, "Could Not Set Velocity");
		return false;
	}

	DebugOut("Servo: %s Initialized\n", servoName);
	return true;
}

bool XL320Driver::UnInitServo()
{
#ifdef THREADING
	//First make sure the move thread is stopped
	if (moveThread.joinable())
		moveThread.join();
#endif
	return true;
}

bool XL320Driver::Calibrate()
{
	if (!portConnected)
	{
		eReturns.Throw(servoName, "Com Port not Connected");
		return false;
	}

	if (!GetPosition())
		return false;
		
	homeOffset = readPositionWithoutOffset; //Need to cast first in case it's negative

	{//Save Parameters to a file
		std::unique_lock<std::mutex> lock(mMutex); 
		if (pSerializer == nullptr)
		{
			eReturns.Throw(servoName, "No Serialization file");
			return false;
		}
		storedPosition = 0;
		if (!pSerializer->SaveInt(servoName + "-StoredPosition", storedPosition))
			return false;

		if (!pSerializer->SaveInt(servoName + "-HomeOffset", homeOffset))
			return false;
	}	

	DebugOut("Servo: %s Offset is set\n", servoName);

	//Throw a warning if servo is not close to middle position, i.e. coupler may have been damaged or bad alignment
	if (homeOffset < 300 || homeOffset > 800)
	{
		eReturns.Throw(servoName, "Home offset is far from center, possible faulty coupler");
		return false;
	}
	
	return true;
}

bool XL320Driver::MoveServoUnitsAbs(float new_position_units)
{
	int abs_position_steps = static_cast<int>((clockWise ? -1 : 1)*new_position_units*stepsPerUnit);

#ifdef THREADING
	if (moveThread.joinable())
		moveThread.join();
	moveThread = std::thread([=] {
#endif

		if (!portConnected)
		{
			eReturns.Throw(servoName, "Com Port not Connected");
			return false;
		}
		
		//Check position before starting move and adjust if servo is not in correct location
		if (!GetPosition(true))
			return false; //May be a problem with threaded operation
		
		storedPosition = readPositionWithoutOffset -homeOffset;

		int move_rel_steps = abs_position_steps - storedPosition;

		if (move_rel_steps == 0)
		{
			DebugOut("Servo: %s hasnt moved, same position %6f\n", servoName, new_position_units);
			return true;
		}

		//Perform the relative move
		if (!MoveServoUnitsRel(move_rel_steps))
			return false;

		//Wait for Move to Finish
		if (!GetPosition(true))
			return false;
		
		//Save Last Stored Position
		{
			std::unique_lock<std::mutex> lock(mMutex);
			storedPosition = abs_position_steps;
			if (!pSerializer->SaveInt(servoName + "-StoredPosition", storedPosition))
				return false;
		}

#ifdef THREADING
		return true;
	});
#endif

	DebugOut("Servo: %s Moving to abs:%6f units\n", servoName, new_position_units);
	return true;
}

bool XL320Driver::MoveServoUnitsRel(int move_rel_steps)
{
	int new_position_steps = static_cast<int>(move_rel_steps) + readPositionWithoutOffset;

	if (!TorqueEnable(true))
		return false;

	if (!MoveToPosition(new_position_steps))
		return false;

	return true;
}

bool XL320Driver::GetPosition(bool wait_for_stop)
{
	if (!portConnected)
	{
		eReturns.Throw(servoName, "Com Port not Connected");
		return false;
	}

	uint32_t position_unsigned;
	if (wait_for_stop)
		if (!WaitForStop())
			return false;

	// Read present position
	if (!pConnection->ReadBytes(deviceId, servoName, addrCurrPosition, position_unsigned, true))
	{
		eReturns.Throw(servoName, "Could Not Get Position");
		return false;
	}

	{//Make thread safe
		std::unique_lock<std::mutex> lock(mMutex);
		//Convert to Signed Position
		readPositionWithoutOffset = static_cast<int>(position_unsigned);

		DebugOut("Servo: %s [ID:%03d] Read Position:%03i\n", servoName, deviceId, readPositionWithoutOffset);
	}
	
	return true;
}

bool XL320Driver::CutTorque()
{
#ifdef THREADING
	//First make sure the move thread is stopped
	if (moveThread.joinable())
		moveThread.join();
#endif
	if (!WaitForStop())
		return false;
	if (!TorqueEnable(false))
		return false;

	DebugOut("Servo: %s Torque Off\n", servoName);
	return true;
}

bool XL320Driver::CheckPower()
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
bool XL320Driver::WaitForStop()
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
		pConnection->CommandDelay(50);
	}
	return true;
}
bool XL320Driver::IsMoving(bool &b_moving)
{
	uint8_t moving_state;
	if (!pConnection->ReadByte(deviceId, servoName, addrIsMoving, moving_state))
		return false;

	b_moving = (moving_state == 1);

	return true;
}

bool XL320Driver::SetSerialNumber(uint16_t serial_number)
{
	if (serial_number > 10 || serial_number == 0)
		return false;
	//Make sure torque is disabled
	if (!TorqueEnable(false))
		return false;

	//Set Home Offset
	//Actual home offset is saved into max Position because home_offset register is not available
	if (!pConnection->WriteByte(deviceId, servoName, addrMaxVoltageLimit, static_cast<uint8_t>(serial_number+80)))
	{
		eReturns.Throw(servoName, "Could not Set serial number");
		return false;
	}

	DebugOut("Servo: %s Saved serial number\n", servoName);
	return true;
}

bool XL320Driver::GetSerialNumber(uint16_t& serial_number)
{
	uint8_t serial_number_unsigned;
	if (!pConnection->ReadByte(deviceId, servoName, addrMaxVoltageLimit, serial_number_unsigned))
	{
		eReturns.Throw(servoName, "Could not Get serial number");
		return false;
	}
	serial_number = static_cast<uint16_t>(serial_number_unsigned - 80);

	DebugOut("Servo: %s Load serial number\n", servoName);
	return true;
}
bool XL320Driver::SetVelocity(uint16_t v_velocity)
{
	//Set RAM velocity
	if (!pConnection->WriteBytes(deviceId, servoName, addrGoalVelocity, v_velocity, true))
	{
		eReturns.Throw(servoName, "Could Not Set Velocity");
		return false;
	}

	return true;
}

bool XL320Driver::GetCurrentLimit(uint16_t& current_limit)
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
bool XL320Driver::SetCurrentLimit(uint16_t current_limit)
{
	//Set EEProm Limit
	uint16_t temp_limit;
	if (!GetCurrentLimit(temp_limit))
	{
		eReturns.Throw(servoName, "Could Not Set Velocity");
		return false;
	}

	if (temp_limit != current_limit)
	{
		//Make sure torque is disabled
		if (!TorqueEnable(false))
			return false;

		DebugOut("Velocity has not been preset.. setting\n");
		if (!pConnection->WriteBytes(deviceId, servoName, addrCurrentLimit, current_limit, true))
		{
			eReturns.Throw(servoName, "Could Not Set Velocity");
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


bool XL320Driver::GetStatusLED(bool& led_status)
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
bool XL320Driver::SetStatusLED(bool led_status)
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

bool XL320Driver::MoveToPosition(int v_position)
{
	//Set Position
	if (!pConnection->WriteBytes(deviceId, servoName, addrGoalPosition, v_position))
	{
		eReturns.Throw(servoName, "Could Not Move to Position");
		return false;
	}

	return true;
}
bool XL320Driver::TorqueEnable(bool b_enable)
{
	if (!pConnection->WriteByte(deviceId, servoName, addrTorqueEnable, b_enable ? 1 : 0))
	{
		eReturns.Throw(servoName, "Could Not Enable Torque");
		return false;
	}

	torqueEnabled = b_enable;

	return true;
}