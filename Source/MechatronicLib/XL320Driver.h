#pragma once
#include "IServoDriver.h"

//  HOMING SEQUENCE FOR EXTENDED POSITION SERVO: (XM430EP)
//  
//              Rev Zone = -1                     Rev Zone = 0                     Rev Zone = 1
//  |--------------------------------||--------------------------------||--------------------------------|
//  -4096                             0                                 4096                          8192
//                                                   >|< Mechanical Position of Joint Center

//          Servo Starts in Zone  0   |-------------->|  Saved Home Offset
//          Servo Starts in Zone  1                   |<----------------|  New Home Offset
//  |------ Servo Starts in Zone -1 ----------------->|  New Home Offset    
//
//                              New Home Offset = Saved Home Offset - Rev zone * 4096

//  HOMING SEQUENCE FOR REGULAR POSITION SERVO: (XM430, AX12)
//  (there is none)
//  
//  1. User manually moves joint to necessary position. 
//  2. When user runs HomeServo function, it saves current position as Home Offset
//  3. Home Offset is used to indicate where the servo starts up

#include "Connection_DXL.h"

#define THREADING
#ifdef THREADING
#include <thread>
#endif

class XL320Driver :	public IServoDriver
{
public:
	XL320Driver(Returns& e_returns, int device_id, std::string servo_name, float steps_per_unit = 1.0F, bool clock_wise = true,
		uint16_t current_limit = 100, uint16_t max_velocity = 350, float min_range = -9.0F, float max_range = 9.0F);
#ifdef THREADING
	~XL320Driver()
	{
		if (moveThread.joinable())
			moveThread.join();
	}
#endif

	virtual bool InitServo(std::shared_ptr<Serializer> p_serializer, bool& servo_needs_cal) override;
	virtual bool UnInitServo() override;

	virtual bool Calibrate() override;
	virtual bool MoveServoUnitsAbs(float new_position_units) override;
	virtual bool GetPosition(bool wait_for_stop = true) override;
	virtual bool CutTorque() override;

	bool SetSerialNumber(uint16_t serial_number) override;
	bool GetSerialNumber(uint16_t& serial_number) override;

private:
	//Register Functions
	bool SetVelocity(uint16_t v_velocity);
	bool GetCurrentLimit(uint16_t& current_limit);
	bool SetCurrentLimit(uint16_t current_limit);
	bool SetStatusLED(bool led_status);
	bool GetStatusLED(bool& led_status);

	bool MoveToPosition(int v_position);
	bool TorqueEnable(bool b_enable);

	//Control Functions
	bool CheckPower();
	virtual bool WaitForStop();
	bool IsMoving(bool &b_moving);

	bool MoveServoUnitsRel(int move_rel_steps);

	int startRevZone = 0;
	int currentRevZone = 0;
	int homeOffset = 512; //Needs to be between 0 and 4096
	int readPositionWithoutOffset = 0; //Needs to be a multiple of 4096

	int minSteps = 0;
	int maxSteps = 0;

#ifdef THREADING
	//void ThreadedMoveUnitsAbs(float new_position_units);
	std::thread moveThread;
	bool moveThreadReturn = true;
#endif
};

