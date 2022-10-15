/**
    (c)2015-2020 Physik Instrumente (PI) GmbH & Co. KG
    Software products that are provided by PI are subject to the General Software License Agreement of 
    Physik Instrumente (PI) GmbH & Co. KG and may incorporate and/or make use of third-party software components. 
    For more information, please read the General Software License Agreement and the Third Party Software Note linked below.

    General Software License Agreement :
    http://www.physikinstrumente.com/download/EULA_PhysikInstrumenteGmbH_Co_KG.pdf
    Third Party Software Note :
    http://www.physikinstrumente.com/download/TPSWNote_PhysikInstrumenteGmbH_Co_KG.pdf
**/

/**
    !! MOTION WARNING !!

    This sample connects to a controller using a connection dialog.
    It shows how to determine min/max position limits, and how to do absolute and relative moves.
    The positioner has to be referenced before starting this sample.
    
**/
#pragma once
#include "IServoDriver.h"

#include <iostream>
#include <string>
#include <time.h>
#include <vector>

#include "PI_sdk/PI_GCS2_DLL.h"
#include "PI_sdk/PI_GCS2_DLL_PF.h"


#include "Connection_PI.h"

#define THREADING
#ifdef THREADING
#include <thread>
#endif




class PI_Driver : public IServoDriver
{
public:

	PI_Driver(Returns& e_returns, int device_id, std::string servo_name, float steps_per_unit = 1.0F, bool clock_wise = true,
		uint16_t current_limit = 100, uint16_t max_velocity = 350, float min_range = -9.0F, float max_range = 9.0F);
#ifdef THREADING
	~PI_Driver()
	{
		if (moveThread.joinable())
			moveThread.join();
	}
	//void ThreadedMoveUnitsAbs(float new_position_units);
	std::thread moveThread;
	bool moveThreadReturn = true;
#endif

	virtual bool InitServo(std::shared_ptr<Serializer> p_serializer, bool& servo_needs_cal) override;
	virtual bool UnInitServo() override;

	virtual bool Calibrate() override;
	virtual bool MoveServoUnitsAbs(float new_position_units) override;
	virtual bool GetPosition(bool wait_for_stop = true) override;
	virtual bool CutTorque() override;

	bool SetSerialNumber(uint16_t serial_number) override;
	bool GetSerialNumber(uint16_t& serial_number) override;
	
	double range; 
	double minPosLimit;
	double maxPosLimit;

	enum ConnectionType
	{
		Dialog,
		RS232,
		TCPIP,
		USB
	};

	//int OpenConnection(ConnectionType type, std::string hostName = "localhost");
	//void PrintControllerIdentification(int iD);
	//void CloseConnectionWithComment(int iD, std::string comment);

	double GetMinPositionLimit(int ID, std::string axis);
	double GetMaxPositionLimit(int ID, std::string axis);
	bool WaitForMotionDone(int ID, std::string axis);
	//void SetServoState(int ID, std::string axis, BOOL state);
	//void ReportError(int iD);
	//int unit_test();
};