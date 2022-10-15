#include "KinematicChain.h"


KinematicChain::KinematicChain(Returns& e_returns, LdrChain chain_ldr, std::shared_ptr<IModel> base_model, std::shared_ptr<KinematicChain> parent_chain) : 
	eReturns(e_returns), 
	parentChain(parent_chain)
{
	DebugOut("Adding Base %s to K chain\n", chain_ldr.chainName);

	if (chain_ldr.useIK)
		ikSolver = std::make_shared<IKSolver>(eReturns, chainJoints);

	chainName = chain_ldr.chainName;

	Matrix4x4 base_location = chain_ldr.chainLocation.GetTranslation() * chain_ldr.chainDirection.GetRotation();

	//Transform base by parent location
	if (parentChain != nullptr)
	{
		Matrix4x4 parent_last_joint;
		parentChain->GetEndMatrix(parent_last_joint);
		base_location = parent_last_joint * base_location;
	}

	//Create a Ldr for base joint
	LdrJoint base_ldr;
	base_ldr.jointName = "Base";

	//Create Base Joint
	auto base_joint = std::make_shared<KinematicJoint>(eReturns, base_location, base_location, chainName, base_ldr, base_model);
	chainJoints.push_back(base_joint);

	if (ikSolver != nullptr)
	{
		DebugOut("Adding Base %s to IK solver\n", chain_ldr.chainName);
		ikSolver->AddJoint(base_joint);
	}
}

bool KinematicChain::AddJoint(LdrJoint joint_ldr, std::shared_ptr<IModel> joint_model, std::shared_ptr<IServoDriver> attached_servo)
{
	DebugOut("Adding Joint %s to K Chain\n", joint_ldr.jointName);

	if (chainJointIndexMap.count(joint_ldr.jointName) != 0)
	{
		eReturns.Throw(chainName + joint_ldr.jointName, "Joint Name Already exists");
		return false;
	}

	//Crate a mat for Joint location
	Matrix4x4 joint_orientation_world = joint_ldr.jointLocation.GetTranslation() * joint_ldr.jointDirection.GetRotation();

	//Transform joint by parent location
	if (parentChain != nullptr)
	{
		Matrix4x4 parent_last_joint;
		parentChain->GetEndMatrix(parent_last_joint);
		joint_orientation_world = parent_last_joint * joint_orientation_world;
	}

	//Create shared_ptr for new joint
	std::shared_ptr<KinematicJoint> p_joint = std::make_shared<KinematicJoint>(eReturns, joint_orientation_world,
		chainJoints[chainJoints.size() - 1]->dirCosMatWorldCurrent, chainName, joint_ldr, joint_model, attached_servo);

	//Add new joint to the vector
	chainJoints.push_back(p_joint);
	//Add new joint to Index Map
	chainJointIndexMap.insert(std::map<std::string, int>::value_type(joint_ldr.jointName, chainJoints.size()-1));

	if (ikSolver != nullptr)
	{
		DebugOut("Adding Joint %s to IK solver\n", joint_ldr.jointName);
		ikSolver->AddJoint(p_joint);
	}
	return true;
}

bool KinematicChain::AddEEPositioner(std::shared_ptr<IModel> joint_model)
{
	DebugOut("Adding EE Positioner Chain\n");
	Matrix4x4 joint_orientation_world;
	GetEndMatrix(joint_orientation_world);
	LdrJoint joint_ldr;
	joint_ldr.jointName = "EE Positioner";
	joint_ldr.minRange = 0;
	joint_ldr.maxRange = 0;
	joint_ldr.prismaticJoint == true;
	//Create shared_ptr for new joint
	eePositioner = std::make_shared<KinematicJoint>(eReturns, joint_orientation_world,	chainJoints[chainJoints.size() - 1]->dirCosMatWorldCurrent, chainName, joint_ldr, joint_model, nullptr);

	return true;
}

bool KinematicChain::ActuateJoint(std::string joint_name, float v_value, bool& inside_bounding_region)
{
	auto p_joint = GetJointByName(joint_name);
	if (!p_joint)
	{
		eReturns.Throw(chainName + joint_name, "Could not Actuate: joint name doesn't exist");
		return false;
	}

	bool b_ret = p_joint->ActuateABS(v_value);
	//UpdateChain();

	//Bounding Region Check
	IModel::Vec3 out_intersection_point;
	inside_bounding_region = BoundingRegionCheck(out_intersection_point);

	if(inside_bounding_region)
		b_ret &= p_joint->UpdateServo(v_value);

	return b_ret;
}

bool KinematicChain::GetJointPosition(std::string joint_name, float & v_value)
{
	auto p_joint = GetJointByName(joint_name);
	if (!p_joint)
	{
		eReturns.Throw(chainName + joint_name, "Could not get Joint position: joint name doesn't exist");
		return false;
	}

	v_value = p_joint->jointActuation;

	return true;
}

bool KinematicChain::ActuateEE(Matrix4x4 ee_world_mat, bool slv_iterative, bool check_bounding_region, bool solve)
{
	if (ikSolver != nullptr)
	{
		//Adjust EE Positioner
		if (eePositioner != nullptr)
		{
			eePositioner->dirCosMatWorldCurrent = ee_world_mat;
			if (eePositioner->jointModel != nullptr)
				eePositioner->jointModel->SetWorldMatrix(eePositioner->dirCosMatWorldCurrent);
		}

		if (solve)
		{
			float mat_diff;

			std::vector<double> joint_motion;


			int m_it = chainJoints.size() - 1;
			auto var_vals = ikSolver->BuildCurrentVarValues();
			VARVALS v_soln = ikSolver->SolveMatrixItr(ee_world_mat, var_vals, m_it);

			//Actuate all joints in chain
			for (auto& v_joint : chainJoints)
			{
				if (v_joint->jointName != "Base")
				{
					float v_value = v_soln.at("{" + v_joint->jointName + "}");
					v_joint->ActuateABS(v_value);
				}
			}

			//UpdateChain(false);


			//Bounding Region Check and update servos, recursive Actuate EE
			//bool inside_bounding_region = true;
			//if (check_bounding_region)
			//{
			//	IModel::Vec3 out_intersection_point;
			//	inside_bounding_region = BoundingRegionCheck(out_intersection_point);
			//	if (!inside_bounding_region)
			//	{
			//		//Correct chain position to intersection point using recusion
			//		Matrix4x4 ee_world_mat_bounding_region = ee_world_mat;
			//		ee_world_mat_bounding_region.Get(0,3) = out_intersection_point.X;
			//		ee_world_mat_bounding_region.Get(1,3) = out_intersection_point.Y;  //TODO: figure out why Z and Y are negative
			//		ee_world_mat_bounding_region.Get(2,3) = out_intersection_point.Z;
			//		ActuateEE(ee_world_mat_bounding_region, false);
			//	}
			//}
			//
			////Update all servos in chain
			//if (inside_bounding_region)
			//{
			//	for (auto& v_joint : chainJoints)
			//	{
			//		if (v_joint->jointName != "Base")
			//		{
			//			float v_value = 0;// v_soln.at("{" + v_joint->jointName + "}");
			//			v_joint->UpdateServo(v_value);
			//		}
			//	}
			//}


#ifdef _DEBUG
	//Debug Print
	//MatrixMath::PrintMatrix(ee_world_mat, "Needed World matrix");
	//MatrixMath::PrintMatrix(chainJoints[m_it]->dirCosMatWorldCurrent, "Current World Matrix");
	//ikSolver->PrintMatrix(m_it, v_soln, "Inverse Solved Matrix");
	//ikSolver->PrintMatrix(m_it, var_vals, "Forward Solved Matrix");
	//DebugOut("\n\n");
#endif
		}
	}

	return true;
}

bool KinematicChain::CalibrateJoint(std::string joint_name)
{
	auto v_joint = GetJointByName(joint_name);
	if (!v_joint)
	{
		eReturns.Throw(chainName + joint_name, "Could not Home: joint name doesn't exist");
		return false;
	}
	return v_joint->CalibrateJoint();
}

void KinematicChain::UpdateChain(bool update_positioner)
{
	//Transform base by parent location
	if (parentChain != nullptr)
	{
		parentChain->GetEndMatrix(chainJoints[0]->dirCosMatWorldCurrent);
		if (chainJoints[0]->jointModel != nullptr)
			chainJoints[0]->jointModel->SetWorldMatrix(chainJoints[0]->dirCosMatWorldCurrent);
	}

	//Compute kinematic chain transformation
	for (int joint_id = 1; joint_id < chainJoints.size(); joint_id++)
	{
		if (chainJoints[joint_id] != nullptr)
		{
			//The Joint's current World Matrix is the last Chain dirCosMatChain multiplied by the Local matrix of the Joint
			chainJoints[joint_id]->dirCosMatWorldCurrent = chainJoints[joint_id - 1]->dirCosMatWorldCurrent * chainJoints[joint_id]->dirCosMatLocalCurrent;
			if (chainJoints[joint_id]->jointModel != nullptr)
				chainJoints[joint_id]->jointModel->SetWorldMatrix(chainJoints[joint_id]->dirCosMatWorldCurrent);
		}
	}

	//Update EE Positioner
	if (eePositioner != nullptr && update_positioner)
	{
		eePositioner->dirCosMatWorldCurrent = chainJoints[chainJoints.size()-1]->dirCosMatWorldCurrent;
		if (eePositioner->jointModel != nullptr)
			eePositioner->jointModel->SetWorldMatrix(eePositioner->dirCosMatWorldCurrent);
	}
}

bool KinematicChain::BoundingRegionCheck(IModel::Vec3& out_intersection_point)
{
	bool inside_bounding_region = true;
	if (boundingRegion != nullptr)
	{
		bool in_bounding_region;
		IModel::Vec3 end_effector_location;
		auto end_effector = chainJoints[chainJoints.size() - 1];
		end_effector_location.X = end_effector->dirCosMatWorldCurrent.Get(0,3);
		end_effector_location.Y = end_effector->dirCosMatWorldCurrent.Get(1,3);  //TODO: figure out why Z and Y are negative
		end_effector_location.Z = end_effector->dirCosMatWorldCurrent.Get(2,3);
		
		boundingRegion->CheckBoundingVolume(end_effector_location, in_bounding_region, out_intersection_point);

		if (!in_bounding_region)
		{
			eReturns.Throw(chainName, "Outside of Bounding Region");
			inside_bounding_region = false;
		}
	}
	return inside_bounding_region;
}

bool KinematicChain::GetEndMatrix(Matrix4x4& dir_cos_mat_joint)
{
	std::shared_ptr<KinematicJoint> v_joint = chainJoints[chainJoints.size() - 1];

	dir_cos_mat_joint = v_joint->dirCosMatWorldCurrent;
	return true;
}

bool KinematicChain::CutTorque()
{
	bool b_ret = true;
	for (int joint_id = 1; joint_id < chainJoints.size(); joint_id++)
	{		
		if (chainJoints[joint_id] != nullptr)
			if (!chainJoints[joint_id]->CutTorque())
				b_ret = false;
	}
	return b_ret;
}

bool KinematicChain::Init()
{
	bool b_ret = true;
	for (int joint_id = 1; joint_id < chainJoints.size(); joint_id++)
	{
		if (chainJoints[joint_id] != nullptr)
			if (!chainJoints[joint_id]->Init())
				b_ret = false;
	}
	//UpdateChain();
	return b_ret;
}

bool KinematicChain::CalibrateAllJoints()
{
	bool b_ret = true;
	for (int joint_id = 1; joint_id < chainJoints.size(); joint_id++)
	{
		if (chainJoints[joint_id] != nullptr)
			if (!chainJoints[joint_id]->CalibrateJoint())
				b_ret = false;
	}
	//UpdateChain();
	return b_ret;
}

bool KinematicChain::ZeroAllJoints()
{
	bool b_ret = true;
	for (int joint_id = 1; joint_id < chainJoints.size(); joint_id++)
	{
		if (chainJoints[joint_id] != nullptr)
		{
			if (!chainJoints[joint_id]->ActuateABS(0.0F))
				b_ret = false;
			if (!chainJoints[joint_id]->UpdateServo(0.0F))
				b_ret = false;
		}
	}
	UpdateChain();
	return b_ret;
}

bool KinematicChain::AddBoundingRegion(std::shared_ptr<IModel> p_model)
{
	//Get end effector location
	IModel::Vec3 end_effector_location;
	auto end_effector = chainJoints[chainJoints.size() - 1];
	end_effector_location.X = end_effector->dirCosMatWorldCurrent.Get(0,3);
	end_effector_location.Y = end_effector->dirCosMatWorldCurrent.Get(1,3);
	end_effector_location.Z = end_effector->dirCosMatWorldCurrent.Get(2,3);

	boundingRegion = std::make_shared<BoundingRegion>(eReturns, chainName, p_model, end_effector_location);
	return true;
}

std::shared_ptr<KinematicJoint> KinematicChain::GetJointByName(std::string joint_name)
{
	//Get a joint by its name
	if (chainJointIndexMap.count(joint_name) != 0)
	{
		std::shared_ptr<KinematicJoint> joint = nullptr;
		int joint_id = chainJointIndexMap.at(joint_name);
		joint = chainJoints[joint_id];
		return joint;
	}
}

std::vector<std::shared_ptr<IModel>> KinematicChain::GetModels()
{
	std::vector<std::shared_ptr<IModel>> return_models;
	
	for(auto& v_joint : chainJoints)
	{
		if(v_joint != nullptr)
			if (v_joint->jointModel != nullptr)
				return_models.push_back(v_joint->jointModel);
	}

	if (eePositioner != nullptr)
		return_models.push_back(eePositioner->jointModel);

	if (boundingRegion != nullptr)
		return_models.push_back(boundingRegion->GetModel());

	return return_models;
}