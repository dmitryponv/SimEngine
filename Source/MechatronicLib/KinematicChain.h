#pragma once
#include <vector>
#include <memory>
#include "IServoDriver.h"
#include "KinematicJoint.h"
#include "../Containers.h"
#include "IKSolver.h"
#include "3Dclasses/IModel.h"
#include "Loader.h"
#include "BoundingRegion.h"

class KinematicChain
{
public:
	KinematicChain(Returns& e_returns, LdrChain chain_ldr, std::shared_ptr<IModel> base_model = nullptr, std::shared_ptr<KinematicChain> parent_chain = nullptr);

	~KinematicChain() {}

	bool AddJoint(LdrJoint joint_ldr, std::shared_ptr<IModel> joint_model, std::shared_ptr<IServoDriver> attached_servo);

	bool ActuateJoint(std::string joint_name, float v_value, bool& inside_bounding_region);
	bool GetJointPosition(std::string joint_name, float& v_value);

	bool ActuateEE(Matrix4x4 ee_world_mat, bool slv_iterative, bool check_bounding_region = false, bool solve = false);

	bool CalibrateJoint(std::string joint_name);
		
	bool GetEndMatrix(Matrix4x4& dir_cos_mat_joint);

	bool CutTorque();

	bool Init();

	bool CalibrateAllJoints();
	bool ZeroAllJoints();

	bool AddBoundingRegion(std::shared_ptr<IModel> p_model);

	std::vector<std::shared_ptr<IModel>> GetModels();
	void UpdateChain(bool update_positioner = true);

	bool UseIK()
	{
		return (ikSolver != nullptr);
	}

	bool AddEEPositioner(std::shared_ptr<IModel> joint_model);

	std::string chainName;

private:
	Returns& eReturns;


	std::vector<std::shared_ptr<KinematicJoint>> chainJoints;
	std::map<std::string, int> chainJointIndexMap;

	std::shared_ptr<KinematicJoint> eePositioner = nullptr;
	std::shared_ptr<IKSolver> ikSolver = nullptr;

	std::shared_ptr<KinematicJoint> GetJointByName(std::string joint_name);

	std::shared_ptr<BoundingRegion> boundingRegion;


	bool BoundingRegionCheck(IModel::Vec3& out_intersection_point);

	std::shared_ptr<KinematicChain> parentChain;
};

