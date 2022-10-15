#include "MechanismCtrl.h"



MechanismCtrl::MechanismCtrl(CALLBACKS& v_callbacks, Returns& e_returns) :
	mCallbacks(v_callbacks),
	eReturns(e_returns)
{
}


MechanismCtrl::~MechanismCtrl()
{
}

bool MechanismCtrl::LoadMechanisms(LdrMechanisms mechanisms_ldr, std::shared_ptr<ServoControl> servo_control)
{
	for (auto& mechanism_ldr : mechanisms_ldr.ldrMechanisms)
	{
		DebugOut("Adding Mechanism %s to mechanism map\n", mechanism_ldr.mechanismName);
		if (mMechanisms.count(mechanism_ldr.mechanismName) != 0)
		{
			eReturns.Throw(mechanism_ldr.mechanismName, "Could not create mechanism: mechanism name already exists");
			return false;
		}
		//Add parent mechanism
		std::shared_ptr<KinematicChain> parent_chain = nullptr;
		if (mMechanisms.count(mechanism_ldr.parentMechanism) != 0)
		{
			auto parent_mechanism = mMechanisms.at(mechanism_ldr.parentMechanism);
			if (parent_mechanism->kinematicChains.at(mechanism_ldr.parentChain) != 0)
			{
				DebugOut("Adding parent %s to mechanism map\n", mechanism_ldr.parentChain);
				parent_chain = parent_mechanism->kinematicChains.at(mechanism_ldr.parentChain);
			}
		}

		//Create different mechanisms
		if(mechanism_ldr.mechanismType == "ChainLink")
		{
			std::shared_ptr<ChainLink> v_mechanism = std::make_shared<ChainLink>(eReturns, mCallbacks, mechanism_ldr, servo_control, parent_chain);
			mMechanisms.insert(std::map<std::string, std::shared_ptr<Mechanism>>::value_type(mechanism_ldr.mechanismName, v_mechanism));
		}

	}

	//Update all chains
	for (auto& v_mechanism : mMechanisms)
	{
		v_mechanism.second->Update();
	}

	return true;
}

bool MechanismCtrl::Unload()
{
	mMechanisms.clear();
	return true;
}

bool MechanismCtrl::Init()
{
	for each(std::pair<std::string, std::shared_ptr<Mechanism>> v_mechanism in mMechanisms)
	{
		if (!v_mechanism.second->Init())
		{
			eReturns.Throw(v_mechanism.first, "Could not init mechanism");
			return false;
		}
	}

	//Update all chains
	for (auto& v_mechanism : mMechanisms)
	{
		v_mechanism.second->Update();
	}

	return true;
}

bool MechanismCtrl::CalibrateAll()
{
	bool b_ret = true;
	// Create a map iterator and point to beginning of map
	std::map<std::string, std::shared_ptr<Mechanism>>::iterator it = mMechanisms.begin();

	// Iterate over the map using Iterator till end.
	while (it != mMechanisms.end())
	{
		b_ret &= it->second->CalibrateAll();
		it++;
	}

	//Update all chains
	for (auto& v_mechanism : mMechanisms)
	{
		v_mechanism.second->Update();
	}

	return b_ret;
}

bool MechanismCtrl::ZeroAll()
{
	bool b_ret = true;
	// Create a map iterator and point to beginning of map
	std::map<std::string, std::shared_ptr<Mechanism>>::iterator it = mMechanisms.begin();

	// Iterate over the map using Iterator till end.
	while (it != mMechanisms.end())
	{
		b_ret &= it->second->ZeroAll();
		it++;
	}

	//Update all chains
	for (auto& v_mechanism : mMechanisms)
	{
		v_mechanism.second->Update();
	}

	return b_ret;
}

std::vector<std::shared_ptr<IModel>> MechanismCtrl::GetModels()
{
	std::vector<std::shared_ptr<IModel>> return_models;

	// Create a map iterator and point to beginning of map
	//std::map<std::string, std::shared_ptr<Mechanism>>::iterator it = mMechanisms.begin();

	// Iterate over the map using Iterator till end.
	for(auto& v_mechanism : mMechanisms)
	{
		std::vector<std::shared_ptr<IModel>> temp_models = v_mechanism.second->GetModels();

		return_models.insert(std::end(return_models), std::begin(temp_models), std::end(temp_models));

		//it++;
	}

	return return_models;
}

bool MechanismCtrl::GetEndMatrix(std::string mechanism_name, std::string chain_name, Matrix4x4 & dir_cos_mat_joint)
{
	if (mMechanisms.count(mechanism_name) == 0)
	{
		eReturns.Throw(mechanism_name, "Could not get Joint Matrix: mechanism name doesn't exist");
		return false; //Returns false if the chain_name given already exists
	}

	//Get world matrix for joint
	return mMechanisms.at(mechanism_name)->GetEndMatrix(chain_name, dir_cos_mat_joint);
}

bool MechanismCtrl::ActuateJoint(std::string mechanism_name, std::string chain_name, std::string joint_name, float v_value, bool & inside_bounding_region)
{
	if (mMechanisms.count(mechanism_name) == 0)
	{
		eReturns.Throw(mechanism_name, "Could not actuate joint: mechanism name doesn't exist");
		return false; //Returns false if the chain_name given already exists
	}
	bool ret = true;
	//Actuate Joint
	ret &= mMechanisms.at(mechanism_name)->ActuateJoint(chain_name, joint_name, v_value, inside_bounding_region);

	//Update all chains
	for (auto& v_mechanism : mMechanisms)
	{
		v_mechanism.second->Update();
	}
	return ret;
}

bool MechanismCtrl::GetJointPosition(std::string mechanism_name, std::string chain_name, std::string joint_name, float & v_value)
{
	if (mMechanisms.count(mechanism_name) == 0)
	{
		eReturns.Throw(mechanism_name, "Could not get Joint position: mechanism name doesn't exist");
		return false; //Returns false if the chain_name given already exists
	}
	bool ret = true;
	//Actuate Joint
	ret &= mMechanisms.at(mechanism_name)->GetJointPosition(chain_name, joint_name, v_value);

	return ret;
}

bool MechanismCtrl::ActuateEE(std::string mechanism_name, std::string chain_name, Matrix4x4 ee_world_mat, bool slv_iterative, bool solve)
{
	if (mMechanisms.count(mechanism_name) == 0)
	{
		eReturns.Throw(mechanism_name, "Could not get Joint Matrix: mechanism name doesn't exist");
		return false; //Returns false if the chain_name given already exists
	}

	//Actuate EE
	bool ret = true;
	ret &= mMechanisms.at(mechanism_name)->ActuateEE(chain_name, ee_world_mat, slv_iterative, solve);
	
	//Update all chains
	for (auto& v_mechanism : mMechanisms)
	{
		for (auto& v_chain : v_mechanism.second->kinematicChains)
			v_chain.second->UpdateChain(false);
	}

	return ret;
}
