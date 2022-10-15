#pragma once
#include "IServoDriver.h"
#include "Connection_DXL.h"

class IXM430Driver :
	public IServoDriver
{
public:

	enum OperatingMode
	{
		VelocityControl = 1,
		PositionControl = 3,
		ExtendedPositionControl = 4,
		CurrentBasePositionControl = 5,
		PWMControl = 16
	};
	enum ErrorCode
	{
		Overload,
		InputVoltage,
		Overheating,
		EncoderError,
		ElectricalShock
	};
	enum MovingStatus
	{
		TrapezoidalVelocityProfile,
		TriangularVelocityProfile,
		RectangularVelocityProfile2,
		RectangularVelocityProfile1,
		FollowingError,
		ProfileOngoing,
		InPosition
	};

	IXM430Driver(Returns& e_returns);

	//Register Functions
	bool GetVelocity(uint16_t& v_velocity);
	bool SetVelocity(uint16_t v_velocity);
	bool GetCurrentLimit(uint16_t& current_limit);
	bool SetCurrentLimit(uint16_t current_limit);
	bool GetTorqueGoal(uint16_t& torque_goal);
	bool SetTorqueGoal(uint16_t torque_goal);
	bool SetStatusLED(bool led_status);
	bool GetStatusLED(bool& led_status);
	bool GetOperatingMode(OperatingMode& operating_mode);
	bool SetOperatingMode(OperatingMode operating_mode);

	//Control Functions
	bool CheckPower();
	virtual bool WaitForStop();
	bool IsMoving(bool &b_moving);
	virtual bool CutTorque() override;

	bool ResetEPMode(); //UNUSED
	bool CheckMotorStall(bool& b_stalled); //UNUSED
	bool RotateUntilStall(bool b_cw); //UNUSED
	bool GetErrors(std::vector<ErrorCode>& error_codes); //UNUSED
	bool GetMovingStatus(std::vector<MovingStatus>& moving_status); //UNUSED

	bool MoveToPosition(int v_position);
	bool TorqueEnable(bool b_enable);
};