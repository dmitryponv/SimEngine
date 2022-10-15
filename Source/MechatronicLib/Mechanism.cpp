#include "Mechanism.h"



Mechanism::Mechanism(Returns& e_returns, CALLBACKS& v_callbacks, LdrMechanism mechanism_ldr, std::shared_ptr<ServoControl> servo_control, std::shared_ptr<KinematicChain> parent_chain) :
	eReturns(e_returns), 
	mCallbacks(v_callbacks),
	servoControl(servo_control),
	parentChain(parent_chain),
	mechanismName(mechanism_ldr.mechanismName)
{
}

Mechanism::~Mechanism()
{
}

bool Mechanism::AddBoundingRegion(std::string chain_name, std::shared_ptr<IModel> p_model)
{
	if (kinematicChains.count(chain_name) == 0)
	{
		eReturns.Throw(chain_name, "Could not create boundign region: chain name doesn't exist");
		return false; //Returns false if the chain_name given already exists
	}

	DebugOut("Adding Bouding Region to chain: %s \n", chain_name);

	//Get world matrix for joint
	return kinematicChains.at(chain_name)->AddBoundingRegion(p_model);
}

//std::shared_ptr<IModel> Mechanism::GetBoundingModel(std::string chain_name)
//{
//	if (kinematicChains.count(chain_name) == 0)
//	{
//		eReturns.Throw(chain_name, "Could not get bounding region model: chain name doesn't exist");
//		return nullptr; //Returns false if the chain_name given already exists
//	}
//
//	if (kinematicChains.count(chain_name) == 0)
//	{
//		eReturns.Throw(chain_name, "Could not get Joint Matrix: chain name doesn't exist");
//		return nullptr; //Returns false if the chain_name given already exists
//	}
//
//	//Get world matrix for joint
//	return kinematicChains.at(chain_name)->GetBoundingModel();
//}

bool Mechanism::ActuateJoint(std::string chain_name, std::string joint_name, float v_value, bool& inside_bounding_region)
{
	if (kinematicChains.count(chain_name) == 0)
	{
		eReturns.Throw(chain_name, "Could not actuate Joint: chain name doesn't exist");
		return false; //Returns false if the chain_name given already exists
	}

	//Actuate joint
	return kinematicChains.at(chain_name)->ActuateJoint(joint_name, v_value, inside_bounding_region);
}

bool Mechanism::GetJointPosition(std::string chain_name, std::string joint_name, float& v_value)
{
	if (kinematicChains.count(chain_name) == 0)
	{
		eReturns.Throw(chain_name, "Could not get Joint position: chain name doesn't exist");
		return false; //Returns false if the chain_name given already exists
	}

	//Actuate joint
	return kinematicChains.at(chain_name)->GetJointPosition(joint_name, v_value);
}

bool Mechanism::ActuateEE(std::string chain_name, Matrix4x4 ee_world_mat, bool slv_iterative, bool solve)
{
	if (kinematicChains.count(chain_name) == 0)
	{
		eReturns.Throw(chain_name, "Could not actuate End Effector: chain name doesn't exist");
		return false; //Returns false if the chain_name given already exists
	}

	//Actuate joint
	bool ret = kinematicChains.at(chain_name)->ActuateEE(ee_world_mat, slv_iterative, true, solve);
	return ret;
}

bool Mechanism::CutTorque()
{
	bool b_ret = true;
	for each(std::pair<std::string, std::shared_ptr<KinematicChain>> chain in kinematicChains)
	{
		if (!chain.second->CutTorque())
		{
			eReturns.Throw(chain.first, "Could not cut torque on chain");
			b_ret |= false;
		}
	}

	return b_ret;
}

bool Mechanism::CalibrateAll()
{
	bool b_ret = true;
	std::map<std::string, std::shared_ptr<KinematicChain>>::iterator it = kinematicChains.begin();
	// Iterate over the map using Iterator till end.
	while (it != kinematicChains.end())
	{
		b_ret &= it->second->CalibrateAllJoints();
		it++;
	}
	return b_ret;
}

bool Mechanism::ZeroAll()
{
	bool b_ret = true;
	std::map<std::string, std::shared_ptr<KinematicChain>>::iterator it = kinematicChains.begin();
	// Iterate over the map using Iterator till end.
	while (it != kinematicChains.end())
	{
		b_ret &= it->second->ZeroAllJoints();
		it++;
	}
	return b_ret;
}

std::vector<std::shared_ptr<IModel>> Mechanism::GetModels()
{
	std::vector<std::shared_ptr<IModel>> return_models;

	// Create a map iterator and point to beginning of map
	std::map<std::string, std::shared_ptr<KinematicChain>>::iterator it = kinematicChains.begin();

	// Iterate over the map using Iterator till end.
	while (it != kinematicChains.end())
	{
		std::vector<std::shared_ptr<IModel>> temp_models = it->second->GetModels();

		return_models.insert(std::end(return_models), std::begin(temp_models), std::end(temp_models));

		it++;
	}

	return return_models;
}

bool Mechanism::GetEndMatrix(std::string chain_name, Matrix4x4 & dir_cos_mat_joint)
{
	if (kinematicChains.count(chain_name) == 0)
	{
		eReturns.Throw(chain_name, "Could not get Joint Matrix: chain name doesn't exist");
		return false; //Returns false if the chain_name given already exists
	}

	//Get world matrix for joint
	return kinematicChains.at(chain_name)->GetEndMatrix(dir_cos_mat_joint);
}

void Mechanism::Update()
{
	for (auto& v_chain : kinematicChains)
	{
		v_chain.second->UpdateChain();

		if (v_chain.second->UseIK()) //Update EE Position
		{
			std::string ee_control_callback = "ChangeEE_" + mechanismName + "*" + v_chain.second->chainName + "*EEControl";
			Matrix4x4 ee_position;
			if (!v_chain.second->GetEndMatrix(ee_position))
				continue;

			Euler v_pos = Matrix4x4::Decompose(ee_position);

			// CALLBACK EVENT	
			if (mCallbacks.find(ee_control_callback) != mCallbacks.end())
			{
				std::shared_ptr<CallbackData::EEMotion> joint_data = std::make_shared<CallbackData::EEMotion>(mechanismName, v_chain.second->chainName, v_pos.X, v_pos.Y, v_pos.Z, v_pos.aX*57.2958, v_pos.aY*57.2958, v_pos.aZ*57.2958);
				mCallbacks.at(ee_control_callback)(std::dynamic_pointer_cast<CallbackData::BlankData>(joint_data));
			}

			//Set camera to look through end effector
			// CALLBACK EVENT
			if (mCallbacks.find("MoveCamera") != mCallbacks.end() && v_chain.first != "MainWindow")
			{
				Matrix4x4::MatrixInv4x4(ee_position, ee_position);
				std::shared_ptr<CallbackData::Matrix> camera_data = std::make_shared<CallbackData::Matrix>("", "MainRenderer", v_chain.first, ee_position);
				mCallbacks.at("MoveCamera")(std::dynamic_pointer_cast<CallbackData::BlankData>(camera_data));
			}

		}
	}
}
