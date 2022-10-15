#pragma once

#include <stdlib.h>
#include <stdio.h>

#include "../Containers.h"      
#include <chrono>
#include <memory>
#include <mutex>
#include "Serializer.h"

class IServoDriver
{
public:

	IServoDriver(Returns& e_returns) :
		eReturns(e_returns)
	{};

	~IServoDriver()
	{
	}

	//Abstract functions
	virtual bool InitServo(std::shared_ptr<Serializer> p_serializer, bool& servo_needs_cal) = 0;
	virtual bool UnInitServo() = 0;
	virtual bool SetSerialNumber(uint16_t serial_number) = 0;
	virtual bool GetSerialNumber(uint16_t& serial_number) = 0;
	virtual bool CutTorque() = 0; 
	virtual bool Calibrate() = 0;
	virtual bool MoveServoUnitsAbs(float new_position_units) = 0; //Moves a servo CW or CW by a certain number of units	
	virtual bool GetPosition(bool wait_for_stop = true) = 0;

	float GetServoPosUnits(){	return (static_cast<float>((clockWise ? -1 : 1)*storedPosition) / stepsPerUnit);}

	void SetConnection(std::shared_ptr<Connection> servo_connection)
	{
		pConnection = servo_connection;
	}

	std::string servoName;
	bool torqueEnabled = false;
	bool portConnected = false;
	
protected:

	bool TorqueEnable(bool b_enable);

	Returns& eReturns;

	int deviceId = 0;
	bool clockWise;
	float stepsPerUnit;

	int storedPosition = 0;
	
	uint16_t currentLimit;
	uint16_t maxVelocity;

	//RAM Addresses
	int addrTorqueEnable = -1;
	int addrGoalCurrent = -1;
	int addrGoalPosition = -1;
	int addrGoalVelocity = -1;
	int addrCurrPosition = -1;
	int addrHWError = -1;
	int addrIsMoving = -1;
	int addrMovingStatus = -1;
	int addrMaxPosition = -1;
	int addrMinPosition = -1;
	int addrStatusLED = -1;

	//EEProm Addresses
	int addrSecondaryId = -1;
	int addrHomeOffset = -1;
	int addrOperatingMode = -1;
	int addrCurrentLimit = -1;
	int addrVelocityLimit = -1;
	int addrMaxVoltageLimit = -1;

	//Variables
	int maxTorque;		
	int shutdownFlags;  
	int torqueLimit;   
	int tempHighLimit;      
	int voltageLowLimit;    
	int voltageHighLimit; 
	

	//Data storage
	std::shared_ptr<Serializer>	pSerializer;  //Serializer for saving servo parameters to a text file, if needed

	std::shared_ptr<Connection> pConnection;
};

