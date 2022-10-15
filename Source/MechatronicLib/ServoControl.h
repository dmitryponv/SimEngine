// ServoControl.h - Contains declaration of Function class  
#pragma once  

#include "Connection_DXL.h"
#include "Connection_PI.h"
#include "XL320Driver.h"
#include "XM430EPDriver.h"
#include "PI_Driver.h"
#include "../Containers.h"
#include "Loader.h"
#include <memory>
#include <map>
#include "Serializer.h"

#define SERVOMAP std::map<std::string, std::shared_ptr<IServoDriver>>

// This class is exported from the MathLibrary.dll  
class ServoControl
{
public:
	//Constructor
	ServoControl(Returns& e_returns):
		eReturns(e_returns)
	{
		pConnection_DXL = std::make_shared<Connection_DXL>(e_returns);
		pConnection_PI = std::make_shared<Connection_PI>(e_returns);
	}
		
	bool Connect(const char* com_port, uint16_t& serial_number, bool& needs_calibration, bool& needs_serial_number);
	bool Disconnect();

	//Initializes a servo to default configuration for operation
	bool CreateServo(std::string servo_name, LdrJoint joint_ldr);

	bool GetServo(std::string servo_name, std::shared_ptr<IServoDriver>& p_servo);

	bool SetSerialNumber(uint16_t serial_number);
	bool GetSerialNumber(uint16_t& serial_number);

	bool Unload()
	{
		mServos.clear();
		pSerializer = nullptr;
		ConnectDXL = false;
		ConnectPI = false;
		return true;
	}

private:
	Returns& eReturns;

	bool ConnectDXL = false;
	bool ConnectPI = false;

	const int defaultBaudRate = 57600; //Register 4     Value 1000000 = 1

	//Create a map of servos by name
	SERVOMAP  mServos;

	std::shared_ptr<Connection_DXL> pConnection_DXL;
	std::shared_ptr<Connection_PI> pConnection_PI;

	std::shared_ptr<Serializer>	pSerializer;

	bool SetConnection();
	bool InitServos(bool& needs_calibration);
};