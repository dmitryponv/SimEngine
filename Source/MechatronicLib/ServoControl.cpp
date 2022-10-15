#include "ServoControl.h"

bool ServoControl::Connect(const char* com_port, uint16_t& serial_number, bool& needs_calibration, bool& needs_serial_number)
{
	if (pConnection_DXL->ConnectPort(com_port, defaultBaudRate))
	{
		if (!SetConnection())
			return false;

		//Get Serial Number from all servos
		if (!GetSerialNumber(serial_number))
		{
			needs_serial_number = true;
			needs_calibration = true;
		}
		else
		{
			//Open/Create a Serializer file
			char v_txt[8];
			sprintf_s(v_txt, 8, "SER%04d", serial_number);
			pSerializer = std::make_shared<Serializer>(eReturns, v_txt);

			//Init each servo
			needs_serial_number = false;
			needs_calibration = false;
			if (!InitServos(needs_calibration))
				return false;

			if (needs_calibration)
				pSerializer->ClearFile();
		}
		return true;
	}
	else
		return false;
}


bool ServoControl::Disconnect()
{
	typedef SERVOMAP::iterator it_type;
	for (it_type p_iterator = mServos.begin(); p_iterator != mServos.end(); p_iterator++)
	{
		///if(p_iterator->second->portConnected)
		///	p_iterator->second->GetPosition(); //Make sure to store the last known position before disconnecting servos
		DebugOut("Servo: %s UNINIT\n", p_iterator->second->servoName);
		if (!p_iterator->second->UnInitServo())
		{
			eReturns.Throw(p_iterator->second->servoName, "Could not UnInit Servo");
			return false;
		}
		p_iterator->second->portConnected = false;
	}
	DebugOut("All Servos have UnInit, disconnecting");
	bool ret = pConnection_DXL->DisconnectPort();
	pSerializer = nullptr;
	return ret;
}

bool ServoControl::CreateServo(std::string servo_name, LdrJoint joint_ldr)
{
	if (mServos.count(servo_name) != 0)
	{
		eReturns.Throw(servo_name, "Servo Name already exists");
		return false;
	}

	std::shared_ptr<IServoDriver> servo = nullptr;
	if (joint_ldr.servoType == "XM430EP")
	{
		servo = std::make_shared<XM430EPDriver>(eReturns, joint_ldr.servoId, servo_name, joint_ldr.stepsPerUnit,
			joint_ldr.clockWise, joint_ldr.currLimit, joint_ldr.maxVelocity, joint_ldr.minRange, joint_ldr.maxRange);
		ConnectDXL = true;
	}
	else if (joint_ldr.servoType == "XL320")
	{
		servo = std::make_shared<XL320Driver>(eReturns, joint_ldr.servoId, servo_name, joint_ldr.stepsPerUnit,
			joint_ldr.clockWise, joint_ldr.currLimit, joint_ldr.maxVelocity, joint_ldr.minRange, joint_ldr.maxRange);
		ConnectDXL = true;
	}
	else if (joint_ldr.servoType == "PI")
	{
		servo = std::make_shared<PI_Driver>(eReturns, joint_ldr.servoId, servo_name, joint_ldr.stepsPerUnit,
			joint_ldr.clockWise, joint_ldr.currLimit, joint_ldr.maxVelocity, joint_ldr.minRange, joint_ldr.maxRange);
		ConnectPI = true;
	}
	//else if (joint_ldr.servoType == "AX12W")
	//	servo = std::make_shared<AX12Driver>(eReturns, joint_ldr.servoId, servo_name, joint_ldr.stepsPerUnit,
	//		joint_ldr.clockWise, joint_ldr.currLimit, joint_ldr.maxVelocity, joint_ldr.minRange, joint_ldr.maxRange); //TODO: Add Other Servo Drivers

	else
	{
		eReturns.Throw(servo_name, "Attempted to create unknown servo type");
		return false;
	}
	
	mServos.insert(SERVOMAP::value_type(servo_name, servo));
	return true;
}

bool ServoControl::GetServo(std::string servo_name, std::shared_ptr<IServoDriver>& p_servo)
{
	if (mServos.count(servo_name) == 0)
	{
		eReturns.Throw(servo_name, "Could not get servo, servo name doesn't exists");
		return false;
	}
	p_servo = mServos.at(servo_name);
	return true;
}


bool ServoControl::SetConnection()
{
	//Connect each servo with a loop
	typedef SERVOMAP::iterator it_type;
	for (it_type p_iterator = mServos.begin(); p_iterator != mServos.end(); p_iterator++)
	{
		p_iterator->second->SetConnection(pConnection_DXL);
		p_iterator->second->portConnected = true;
	}
	return true;
}

bool ServoControl::InitServos(bool& needs_calibration)
{
	typedef SERVOMAP::iterator it_type;
	for (it_type p_iterator = mServos.begin(); p_iterator != mServos.end(); p_iterator++)
	{
		bool servo_needs_cal = false;
		if (!p_iterator->second->InitServo(pSerializer, servo_needs_cal))
		{
			eReturns.Throw(p_iterator->second->servoName, "Could not Init Servo");
			return false;
		}
		else
			needs_calibration |= servo_needs_cal;
	}
	return true;
}

bool ServoControl::SetSerialNumber(uint16_t serial_number)
{
	bool ret = true;
	typedef SERVOMAP::iterator it_type;
	for (it_type p_iterator = mServos.begin(); p_iterator != mServos.end(); p_iterator++)
	{
		if (p_iterator->second->portConnected)
		{
			if (!p_iterator->second->SetSerialNumber(serial_number))
				ret = false;
		}
		else
		{
			eReturns.Throw("ServoControl", "Not All Ports connected");
			return false;
		}
	}

	if (ret == true)
	{
		char v_txt[8];
		sprintf_s(v_txt, 8, "SER%04d", serial_number);
		pSerializer->RenameFile(v_txt);
	}

	return ret;
}

bool ServoControl::GetSerialNumber(uint16_t& serial_number)
{
	uint16_t serial_num_last = 0;
	typedef SERVOMAP::iterator it_type;
	for (it_type p_iterator = mServos.begin(); p_iterator != mServos.end(); p_iterator++)
	{
		if (p_iterator->second->portConnected)
		{
			uint16_t serial_num_temp = 0;
			if (!p_iterator->second->GetSerialNumber(serial_num_temp))
				return false;
			else if (serial_num_temp == 0)
				return false;
			else if (serial_num_last == 0)
				serial_num_last = serial_num_temp; //First save the last as the current S/N
			else if (serial_num_last != serial_num_temp)
				return false; //Finally check if all serial numbers are equal
		}
		else
		{
			eReturns.Throw("ServoControl", "Not All Ports connected");
			return false;
		}
	}
	serial_number = serial_num_last;

	if (serial_number > 4096 || serial_number < 1)
	{
		eReturns.Throw("ServoControl", "Serial Number is outside of the 1..4096 range");
		return false;
	}

	return true;
}
