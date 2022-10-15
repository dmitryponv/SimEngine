#pragma once
#include <memory>
#include <map>
#include "Mechanism.h"
#include "Loader.h"
#include "ServoControl.h"
#include "ChainLink.h"

#include "../Containers.h"

class MechanismCtrl
{
public:
	MechanismCtrl(CALLBACKS& v_callbacks, Returns& e_returns);
	~MechanismCtrl();

	bool LoadMechanisms(LdrMechanisms mechanisms_ldr, std::shared_ptr<ServoControl> servo_control);
	bool Unload();

	bool Init();

	bool CalibrateAll();
	bool ZeroAll();

	std::vector<std::shared_ptr<IModel>> GetModels();

	bool GetEndMatrix(std::string mechanism_name, std::string chain_name, Matrix4x4& dir_cos_mat_joint);

	bool ActuateJoint(std::string mechanism_name, std::string chain_name, std::string joint_name, float v_value, bool& inside_bounding_region);
	bool GetJointPosition(std::string mechanism_name, std::string chain_name, std::string joint_name, float& v_value);
	
	bool ActuateEE(std::string mechanism_name, std::string chain_name, Matrix4x4 ee_world_mat, bool slv_iterative = false, bool solve = false);

private:
	Returns& eReturns;
	CALLBACKS& mCallbacks;
	std::map<std::string, std::shared_ptr<Mechanism>> mMechanisms;
};

