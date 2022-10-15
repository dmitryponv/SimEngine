#include "ChainLink.h"



ChainLink::ChainLink(Returns& e_returns, CALLBACKS& v_callbacks, LdrMechanism mechanism_ldr, std::shared_ptr<ServoControl> servo_control, std::shared_ptr<KinematicChain> parent_chain) :
	Mechanism(e_returns, v_callbacks, mechanism_ldr, servo_control, parent_chain)
{
	for (auto& v_chain : mechanism_ldr.Chains)
	{
		if (!AddChain(v_chain))
			eReturns.Throw(mechanism_ldr.mechanismName, "Could not create chain");
	}
}


ChainLink::~ChainLink()
{
}

bool ChainLink::Init()
{
	for each(std::pair<std::string, std::shared_ptr<KinematicChain>> chain in kinematicChains)
	{
		if (!chain.second->Init())
		{
			eReturns.Throw(chain.first, "Could not init chain");
			return false;
		}
	}
	return true;
}


bool ChainLink::AddChain(LdrChain chain_ldr)
{
	DebugOut("Adding Chain %s to chain map\n", chain_ldr.chainName);
	if (kinematicChains.count(chain_ldr.chainName) != 0)
	{
		eReturns.Throw(chain_ldr.chainName, "Could not create chain: chain name already exists");
		return false;
	}


	//Create the model	
	std::shared_ptr<IModel> base_model_3d = nullptr;// chain_ldr.modelFile.length() > 1 ? std::make_shared<IModel>() : nullptr;

	// CALLBACK EVENT
	if (chain_ldr.modelFile.length() > 1) //ensure that model wants to be loaded, otherwise it stays as IModel and doesn't render
	{
		if (mCallbacks.find("Factory_GetIModel") != mCallbacks.end())  //ensure that callback exists if not used for an embedded application
		{
			std::shared_ptr<CallbackData::ModelCtr> model_data = std::make_shared<CallbackData::ModelCtr>(chain_ldr.chainName + "Base", "MainRenderer");
			mCallbacks.at("Factory_GetIModel")(std::dynamic_pointer_cast<CallbackData::BlankData>(model_data));
			base_model_3d = model_data->iModel;

			Matrix4x4 inverse_chain_orientation_world = chain_ldr.chainLocation.GetTranslation() * chain_ldr.chainDirection.GetRotation();
			Matrix4x4::MatrixInv4x4(inverse_chain_orientation_world, inverse_chain_orientation_world);

			if (!base_model_3d->Load(inverse_chain_orientation_world, chain_ldr.modelFile))
				throw std::runtime_error("Model Load Error");
		}
	}

	std::shared_ptr<KinematicChain> kinematic_chain = std::make_shared<KinematicChain>(eReturns, chain_ldr, base_model_3d, parentChain);
	kinematicChains.insert(std::map<std::string, std::shared_ptr<KinematicChain>>::value_type(chain_ldr.chainName, kinematic_chain));

	for (auto& v_joint : chain_ldr.Joints)
	{
		//Add Joints
		AddJoint(chain_ldr.chainName, v_joint);
		int y_camera = v_joint.jointLocation.y;
		//if (min_y_camera > y_camera) min_y_camera = y_camera;
		//if (max_y_camera < y_camera) max_y_camera = y_camera;
	}

	kinematic_chain->UpdateChain();

	//Create Model for EE Positioner
	if (chain_ldr.useIK)
	{
		std::shared_ptr<IModel> joint_model_3d = std::make_shared<IModel>();

#ifndef EMBEDDED
		// CALLBACK EVENT
		if (mCallbacks.find("Factory_GetIModel") != mCallbacks.end())
		{
			std::shared_ptr<CallbackData::ModelCtr> model_data = std::make_shared<CallbackData::ModelCtr>(chain_ldr.chainName + "EEPositioner", "MainRenderer");
			mCallbacks.at("Factory_GetIModel")(std::dynamic_pointer_cast<CallbackData::BlankData>(model_data));
			joint_model_3d = model_data->iModel;
		}
#endif
		Matrix4x4 ee_orientation_world;
		//if (!kinematicChains.at(chain_ldr.chainName)->GetEndMatrix(ee_orientation_world))
		//	return false;
		//Matrix4x4::MatrixInv4x4(ee_orientation_world, ee_orientation_world);
		
		if (!joint_model_3d->Load(ee_orientation_world, "Media/CoordSystem.obj"))
			throw std::runtime_error("Model File does not exist");

		if (!kinematicChains.at(chain_ldr.chainName)->AddEEPositioner(joint_model_3d))
			return false;
	}

	//Add Bounding Frame		
	if (chain_ldr.boundingModel != "")
	{
		//Create a Model for bounding region. In winapp create an IModel
//////////TODO: Load proper model
		//std::shared_ptr<ShapeCluster3D> model_3d = std::make_shared<ShapeCluster3D>(std::static_pointer_cast<DXContainer>(modelContainer), "sphere", std::vector<Vector3>());
		std::shared_ptr<IModel> model_3d = std::make_shared<IModel>();

		//if (!model_3d->LoadOBJ(chain_ldr.boundingModel))
		//	throw std::runtime_error("Model File does not exist");

		AddBoundingRegion(chain_ldr.chainName, model_3d);
	}

	//Add Collision detection
#ifdef D3DAPP
	if (chain_ldr.collisionDetection)
	{
		auto models_3d = GetModels();
		std::shared_ptr<CollisionDetection> v_detector = std::make_shared<CollisionDetection>(std::static_pointer_cast<DXContainer>(modelContainer),
			std::static_pointer_cast<Model3D>(models_3d[0]),
			std::static_pointer_cast<Model3D>(models_3d[models_3d.size() - 1]));
		collisionDetectors.push_back(v_detector);
	}
#endif

	return true;
}

bool ChainLink::AddJoint(std::string chain_name, LdrJoint joint_ldr)
{
	if (kinematicChains.count(chain_name) == 0)
	{
		eReturns.Throw(chain_name, "Could not create joint: chain name doesn't exist");
		return false; //Returns false if the chain_name given already exists
	}

	std::shared_ptr<IServoDriver> attached_servo = nullptr;
	if (joint_ldr.servoId > 0 && joint_ldr.servoId < 251 && servoControl != nullptr)
	{
		std::string servo_name = chain_name + joint_ldr.jointName;
		if (!servoControl->CreateServo(servo_name, joint_ldr))
			return false;

		if (!servoControl->GetServo(servo_name, attached_servo))
		{
			eReturns.Throw(chain_name + joint_ldr.jointName, "Failed to Create Servo, servo name already exists");
			return false; //Does not create a servo if the joint_name given already exists
		}
	}


	//Create Model for Joint
	std::shared_ptr<IModel> joint_model_3d = nullptr;// joint_ldr.modelFile.length() > 1 ? std::make_shared<IModel>() : nullptr;

	if (joint_ldr.modelFile.length() > 1) //ensure that model wants to be loaded, otherwise it stays as IModel and doesn't render
	{
#ifndef EMBEDDED
		// CALLBACK EVENT
		if (mCallbacks.find("Factory_GetIModel") != mCallbacks.end())
		{
			std::shared_ptr<CallbackData::ModelCtr> model_data = std::make_shared<CallbackData::ModelCtr>(chain_name + joint_ldr.jointName, "MainRenderer");
			mCallbacks.at("Factory_GetIModel")(std::dynamic_pointer_cast<CallbackData::BlankData>(model_data));
			joint_model_3d = model_data->iModel;
		}
#endif

		if (joint_ldr.modelFile.length() > 1)
		{
			Matrix4x4 inverse_joint_orientation_world = joint_ldr.jointLocation.GetTranslation() * joint_ldr.jointDirection.GetRotation();
			Matrix4x4::MatrixInv4x4(inverse_joint_orientation_world, inverse_joint_orientation_world);

			if (!joint_model_3d->Load(inverse_joint_orientation_world, joint_ldr.modelFile))
				throw std::runtime_error("Model File does not exist");
		}
	}

	return kinematicChains.at(chain_name)->AddJoint(joint_ldr, joint_model_3d, attached_servo);
}