#include "XM430EPDriver.h"

XM430EPDriver::XM430EPDriver(Returns& e_returns, int device_id, std::string servo_name, float steps_per_unit, bool clock_wise,
	uint16_t current_limit, uint16_t max_velocity, float min_range, float max_range) : IXM430Driver(e_returns)
{
	deviceId = device_id;
	servoName = servo_name;
	stepsPerUnit = steps_per_unit;
	clockWise = clock_wise;
	currentLimit = current_limit;
	maxVelocity = max_velocity;
	minSteps = static_cast<int>(min_range*steps_per_unit);
	maxSteps = static_cast<int>(max_range*steps_per_unit);
}

bool XM430EPDriver::InitServo(std::shared_ptr<Serializer> p_serializer, bool& servo_needs_cal)
{
	pSerializer = p_serializer;
	if (!portConnected)
	{
		eReturns.Throw(servoName, "InitServo: Com Port not Connected");
		return false;
	}

	OperatingMode operating_mode;
	GetOperatingMode(operating_mode);
	if (operating_mode != OperatingMode::CurrentBasePositionControl)
	{
		if (!SetOperatingMode(OperatingMode::CurrentBasePositionControl))
			return false;
	}

	//Turn ON Status LED
	if (!SetStatusLED(true))
	{
		eReturns.Throw(servoName, "Cannot Set LED status");
		return false;
	}

	//Get Home Offset from EEProm, needs to be done outside of mutex lock otherwise a deadlock occurs
	if (!GetHomeOffset(homeOffset))
	{
		servo_needs_cal = true;
		return false;
	}
	///May need to add a check if homeOffset is 0
	DebugOut("Servo: %s Home Offset:%6d units\n", servoName, homeOffset);

	//Read Serialization file
	bool serializer_error = false;
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
		serializer_error = true;
	}
	
	//Save Calibration values
	if (!pSerializer->SaveInt(servoName + "-HomeOffset(reference_only)", homeOffset))
		return false;

	GetPosition();
		
	//Adjust stored position to the current position always after GetPosition	
	AdjustStoredPosition();

	//If the serializer did not read correctly, save current positon
	if (serializer_error == true)
	{
		DebugOut("Servo: %s storing last saved position %6d\n", servoName, storedPosition);
		if (!pSerializer->SaveInt(servoName + "-StoredPosition", storedPosition))
			return false;
	}

	//Need need to check that the stored position is not beyond the limit of the servo
	if (storedPosition > (maxSteps + 100) || storedPosition < (minSteps - 100))
	{
		eReturns.Throw(servoName, "Servo Outside allowed position, position = " + std::to_string(storedPosition));
		servo_needs_cal = true;
		//return false;
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

bool XM430EPDriver::UnInitServo()
{
	//Servo will already wait for stop in MoveServoUnitsAbs thread GetPosition call
#ifdef THREADING
	//First make sure the move thread is stopped
	if (moveThread.joinable())
		moveThread.join();
#endif
	return true;
}

bool XM430EPDriver::Calibrate()
{
	if (!portConnected)
	{
		eReturns.Throw(servoName, "Calibrate: Com Port not Connected");
		return false;
	}

	if (!GetPosition())
		return false;
		
	homeOffset = static_cast<uint16_t>(readPositionWithoutOffset) % 4096; //Need to cast first in case it's negative

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
	}	
	
	//Set Home Offset to EEProm, needs to be done outside of mutex lock otehrwise a deadlock occurs
	if (!SetHomeOffset(homeOffset))
		return false;

	DebugOut("Servo: %s Offset is set\n", servoName);

	//Throw a warning if servo is not close to middle position, i.e. coupler may have been damaged or bad alignment
	if (homeOffset < 1000 || homeOffset > 3000)
	{
		eReturns.Throw(servoName, "Home offset is far from center, possible faulty coupler");
		return false;
	}
	
	return true;
}

bool XM430EPDriver::MoveServoUnitsAbs(float new_position_units)
{
	int abs_position_steps = static_cast<int>((clockWise ? -1 : 1)*new_position_units*stepsPerUnit);

	if (abs_position_steps < minSteps || abs_position_steps > maxSteps)
	{
		eReturns.Throw(servoName, "Move Abs: commanded position outside limit");
		return false;
	}

#ifdef THREADING
	if (moveThread.joinable())
		moveThread.join();
	moveThread = std::thread([=] {
#endif

		if (!portConnected)
		{
			eReturns.Throw(servoName, "Move Abs: Com Port not Connected");
			return false;
		}
		
		//Check position before starting move and adjust if servo is not in correct location
		if (!GetPosition(true))
			return false; //May be a problem with threaded operation
		
		if (!AdjustStoredPosition())
			return false; //Modify Stored Position using Read Position

		int move_rel_steps = abs_position_steps - storedPosition;

		if (move_rel_steps == 0)
		{
			DebugOut("Servo: %s hasnt moved, same position %6f\n", servoName, new_position_units);
			return true;
		}

		//Perform the relative move
		if (!MoveServoUnitsRel(move_rel_steps))
			return false;

		//Save Last Stored Position
		{
			std::unique_lock<std::mutex> lock(mMutex);
			storedPosition = abs_position_steps;
			DebugOut("Servo: %s storing last saved position %6d\n", servoName, storedPosition);
			if (!pSerializer->SaveInt(servoName + "-StoredPosition", storedPosition))
				return false;
		}

		//Wait for Move to Finish
		if (!GetPosition(true))
			return false;
		
		

#ifdef THREADING
		return true;
	});
#endif

	DebugOut("Servo: %s Moving to abs:%6f units\n", servoName, new_position_units);
	return true;
}

bool XM430EPDriver::MoveServoUnitsRel(int move_rel_steps)
{
	int new_position_steps = static_cast<int>(move_rel_steps) + readPositionWithoutOffset;

	if (!TorqueEnable(true))
		return false;

	if (!MoveToPosition(new_position_steps))
		return false;

	return true;
}

bool XM430EPDriver::AdjustStoredPosition()
{
	DebugOut("DEBUG Servo: %s storedPosition Before:%03i\n", servoName, storedPosition);
	DebugOut("DEBUG Servo: %s Read Position:%03i\n", servoName, readPositionWithoutOffset);
	//Adjust absolute position in case the servo did not move to correct location or was moved with power off
	uint16_t local_pos_read = static_cast<uint16_t>(readPositionWithoutOffset - static_cast<int>(homeOffset)) % 4096; //Adjust the read postion by home offset and crop it to 12 bits
	uint16_t local_pos_stored = static_cast<uint16_t>(storedPosition) % 4096; //Crop stored position to 12 bits
	//NOTE1: Converting a negative result to uint16_t is the same as writing (((storedPosition % 4096) + 4096) % 4096)
	//NOTE2: If() operations on uint16_t will NOT roll over to 65535 when subtraction result is negative
	if (local_pos_read < (local_pos_stored - 2048)) //Means servo was rotated less than half a rev forward
		storedPosition += static_cast<int>(local_pos_read - local_pos_stored + 4096);
	else if (local_pos_read < local_pos_stored) //Means servo was rotated less than half a rev back
		storedPosition += static_cast<int>(local_pos_read - local_pos_stored);
	else if (local_pos_read > (local_pos_stored + 2048)) //Means servo was rotated less than half a rev backward
		storedPosition += static_cast<int>(local_pos_read - local_pos_stored - 4096);
	else if (local_pos_read > local_pos_stored) //Means servo was rotated less than half a rev forward
		storedPosition += static_cast<int>(local_pos_read - local_pos_stored);

	DebugOut("DEBUG Servo: %s local_pos_read:%03i\n", servoName, local_pos_read);
	DebugOut("DEBUG Servo: %s local_pos_stored:%03i\n", servoName, local_pos_stored);
	DebugOut("DEBUG Servo: %s storedPosition after:%03i\n", servoName, storedPosition);
	DebugOut("DEBUG Servo: %s Home Offset:%03i\n", servoName, homeOffset);

	return true;
}

bool XM430EPDriver::GetPosition(bool wait_for_stop)
{
	if (!portConnected)
	{
		eReturns.Throw(servoName, "Get Position: Com Port not Connected");
		return false;
	}

	uint32_t position_unsigned;
	if (wait_for_stop)
		if (!WaitForStop())
			return false;

	// Read present position
	if (!pConnection->ReadBytes(deviceId, servoName, addrCurrPosition, position_unsigned))
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

bool XM430EPDriver::CutTorque()
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

bool XM430EPDriver::SetHomeOffset(uint16_t home_offset)
{
	//Make sure torque is disabled
	if (!TorqueEnable(false))
		return false;

	//Set Home Offset
	//Actual home offset is saved into max Position because home_offset register is not available
	if (!pConnection->WriteBytes(deviceId, servoName, addrMaxPosition, static_cast<uint32_t>(home_offset)))
	{
		eReturns.Throw(servoName, "Could not Set home offset");
		return false;
	}

	DebugOut("Servo: %s Saved Home Offset\n", servoName);
	return true;
}

bool XM430EPDriver::GetHomeOffset(uint16_t& home_offset)
{
	uint32_t home_offset_unsigned;
	if (!pConnection->ReadBytes(deviceId, servoName, addrMaxPosition, home_offset_unsigned))
	{
		eReturns.Throw(servoName, "Could not Get home offset");
		return false;
	}
	home_offset = static_cast<uint16_t>(home_offset_unsigned);

	DebugOut("Servo: %s Load Home Offset\n", servoName);
	return true;
}

bool XM430EPDriver::SetSerialNumber(uint16_t serial_number)
{
	if (serial_number > 4096 || serial_number == 0)
		return false;
	//Make sure torque is disabled
	if (!TorqueEnable(false))
		return false;

	//Set Home Offset
	//Actual home offset is saved into max Position because home_offset register is not available
	if (!pConnection->WriteBytes(deviceId, servoName, addrMinPosition, static_cast<uint32_t>(serial_number)))
	{
		eReturns.Throw(servoName, "Could not Set serial number");
		return false;
	}

	DebugOut("Servo: %s Saved serial number\n", servoName);
	return true;
}

bool XM430EPDriver::GetSerialNumber(uint16_t& serial_number)
{
	uint32_t serial_number_unsigned;
	if (!pConnection->ReadBytes(deviceId, servoName, addrMinPosition, serial_number_unsigned))
	{
		eReturns.Throw(servoName, "Could not Get serial number");
		return false;
	}
	serial_number = static_cast<uint16_t>(serial_number_unsigned);

	DebugOut("Servo: %s Load serial number\n", servoName);
	return true;
}