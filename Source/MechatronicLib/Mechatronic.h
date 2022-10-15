#pragma once

#include <memory>
#include <map>
#include "ServoControl.h"
#include "Loader.h"
#include "MechanismCtrl.h"
#include "../Containers.h"

class Mechatronic
{
public:
	Mechatronic(CALLBACKS& v_callbacks):servoControl(std::make_shared<ServoControl>(eReturns)), mechanismCtrl(v_callbacks, eReturns) {}
	~Mechatronic() {}

	bool Connect(const char* com_port, uint16_t& serial_number, bool& needs_calibration, bool& needs_serial_number);
	bool Disconnect();

	bool CutTorque();

	//Mechanism functions
	bool LoadMechanisms(LdrMechanisms mechanisms_ldr);


	//Simplified interface for non-mechanical applications
	bool AddServo(std::string chain_name, LdrJoint joint_ldr);
	bool CalibrateServo(std::string chain_name, std::string joint_name);
	bool ActuateServo(std::string chain_name, std::string joint_name, float v_value);
	
	//Additional Functions
	bool GetEndMatrix(std::string mechanism_name, std::string chain_name, Matrix4x4& dir_cos_mat_joint);
	std::vector<std::shared_ptr<IModel>> GetModels();
	Returns& GetReturns() { return eReturns; }
	bool SetSerialNumber(uint16_t serial_number) { return servoControl->SetSerialNumber(serial_number); }	
	bool CalibrateAll();
	bool ZeroAll();
	bool Unload()
	{
		mechanismCtrl.Unload();
		if(servoControl != nullptr)	servoControl->Unload();
		return true;
	}
	
	bool ActuateJoint(std::string mechanism_name, std::string chain_name, std::string joint_name, float v_value, bool& inside_bounding_region);
	bool GetJointPosition(std::string mechanism_name, std::string chain_name, std::string joint_name, float& v_value);

	bool ActuateEE(std::string mechanism_name, std::string chain_name, Matrix4x4 ee_world_mat, bool slv_iterative = false, bool solve = false);
	//MECHATRONIC_API bool CalibrateJoint(std::string mechanism_name, std::string chain_name, std::string joint_name);
	
	bool servosConnected = false;

private:
	std::shared_ptr<ServoControl> servoControl;
	MechanismCtrl mechanismCtrl;

	Returns eReturns;
};

