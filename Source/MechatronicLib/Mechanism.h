#pragma once

#include <memory>
#include <map>
#include "KinematicChain.h"
#include "Loader.h"
#include "ServoControl.h"

//#ifdef DXTKAPP
#include "3Dclasses/IModel.h"
//#include "3DClasses/Model3D.h"
//#include "CollisionDetection.h"
//#elif defined DXUTAPP
//#include "3DClasses/DXUTModel.h"
//#include "CollisionDetection.h"
//#elif defined DXRAWAPP
//#include "../EyeCtrlRaw/Source/Rastertek/modelclass.h"
//#else
//#endif

#include "../Containers.h"

class Mechanism
{
public:
	Mechanism(Returns& e_returns, CALLBACKS& v_callbacks, LdrMechanism mechanism_ldr, std::shared_ptr<ServoControl> servo_control, std::shared_ptr<KinematicChain> parent_chain = nullptr);
	~Mechanism();

	virtual bool Init() = 0;
	
	//ADD FUNCTIONS
	bool AddBoundingRegion(std::string chain_name, std::shared_ptr<IModel> p_model = nullptr);
		
	//MOTION FUNCTIONS
	bool ActuateJoint(std::string chain_name, std::string joint_name, float v_value, bool& inside_bounding_region);
	bool GetJointPosition(std::string chain_name, std::string joint_name, float& v_value);
	bool ActuateEE(std::string chain_name, Matrix4x4 ee_world_mat, bool slv_iterative = false, bool solve = false);
	bool CutTorque();

	bool CalibrateAll();
	bool ZeroAll();

	std::vector<std::shared_ptr<IModel>> GetModels();

	bool GetEndMatrix(std::string chain_name, Matrix4x4& dir_cos_mat_joint);

	void Update();

	std::map<std::string, std::shared_ptr<KinematicChain>>  kinematicChains;

	std::string mechanismName;

protected:
	Returns& eReturns;
	std::shared_ptr<ServoControl> servoControl;

	std::shared_ptr<KinematicChain> parentChain = nullptr;

	CALLBACKS& mCallbacks;

#ifdef DXTKAPP
	std::vector<std::shared_ptr<CollisionDetection>> collisionDetectors;
#endif
};

