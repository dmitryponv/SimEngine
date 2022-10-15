#include "MachineManager.h"


MachineManager::MachineManager(std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks):
	mMechatronic(v_callbacks),
	mCallbacks(v_callbacks)
{
	mCallbacks["Config_Select_selchange"] = std::bind(&MachineManager::ConfigSelectCallback, this, std::placeholders::_1);   //CALLBACK INIT
	mCallbacks["Config_Select_dropdown"] = std::bind(&MachineManager::ConfigDDCallback, this, std::placeholders::_1);   //CALLBACK INIT
	mCallbacks["Config_button"] = std::bind(&MachineManager::ConfigButtonCallback, this, std::placeholders::_1);   //CALLBACK INIT
	mCallbacks["GetMachineModels"] = std::bind(&MachineManager::GetModelsCallback, this, std::placeholders::_1);   //CALLBACK INIT
	mCallbacks["MoveJointCallback"] = std::bind(&MachineManager::MoveJointCallback, this, std::placeholders::_1);   //CALLBACK INIT

	mCallbacks["SetSerialNumberCallback"] = std::bind(&MachineManager::SetSerialNumberCallback, this, std::placeholders::_1);   //CALLBACK INIT
	mCallbacks["COMPort_Dropdown_selchange"] = std::bind(&MachineManager::SetSelectedComPortCallback, this, std::placeholders::_1);   //CALLBACK INIT

	mCallbacks["Connect_button"] = std::bind(&MachineManager::ConnectServosCallback, this, std::placeholders::_1);   //CALLBACK INIT
	mCallbacks["Calibrate_button"] = std::bind(&MachineManager::CalibrateServosCallback, this, std::placeholders::_1);   //CALLBACK INIT
	mCallbacks["Zero_button"] = std::bind(&MachineManager::ZeroServosCallback, this, std::placeholders::_1);   //CALLBACK INIT

	mCallbacks["PositionEECallback"] = std::bind(&MachineManager::PositionEECallback, this, std::placeholders::_1);   //CALLBACK INIT
}

MachineManager::~MachineManager()
{
}

bool MachineManager::Init()
{
	//TEST
	//Loader v_loader("ObjEyeChainConfig.xml");
	//mechanismsLdr = v_loader.LoadMechanisms();
	//
	//mMechatronic.Unload();
	//mMechatronic.LoadMechanisms(mechanismsLdr);

	return true;
}

//CALLBACK FUNCTION
bool MachineManager::ConfigSelectCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::ComboBoxEntry> combo_data = std::dynamic_pointer_cast<CallbackData::ComboBoxEntry>(callback_data);

	if (combo_data == nullptr)
		return false;

	configName = combo_data->comboText;

	return true;
}

//CALLBACK FUNCTION
bool MachineManager::ConfigDDCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::ComboBoxList> combo_data = std::dynamic_pointer_cast<CallbackData::ComboBoxList>(callback_data);

	if (combo_data == nullptr)
		return false;

	WIN32_FIND_DATA search_data;
	memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
	HANDLE handle = FindFirstFile(L"Inputs/*", &search_data);
	while (handle != INVALID_HANDLE_VALUE)
	{
		//cout << "\n" << search_data.cFileName;
		if (std::wcsstr(search_data.cFileName, L".xml") != NULL) //if it has xml extention
		{
			std::wstring w_s(search_data.cFileName);
			combo_data->comboItems.push_back(w_s);
		}
		if (FindNextFile(handle, &search_data) == FALSE)
			break;
	}

	return true;
}

//CALLBACK FUNCTION
bool MachineManager::ConfigButtonCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::Button> combo_data = std::dynamic_pointer_cast<CallbackData::Button>(callback_data);

	if (combo_data == nullptr)
		return false;
	
	Loader v_loader(configName);
	mechanismsLdr = v_loader.LoadMechanisms();

	mMechatronic.Unload();

	//Set camera distance
	// CALLBACK EVENT
	if (mCallbacks.find("MoveCamera") != mCallbacks.end())
	{
		Matrix4x4 new_cam_location = Matrix4x4::CreateRotX(90) * Matrix4x4::CreateTranslation(0, mechanismsLdr.cameraDistance, mechanismsLdr.cameraDistance);
			//= Matrix4x4::CreateView(
			//Vector4(0, 2 * mechanismsLdr.cameraDistance, 2 * mechanismsLdr.cameraDistance), //position
			//Vector4(0, 0, 0), //target
			//Vector4(0, 1, 0)); //up
		std::shared_ptr<CallbackData::Matrix> camera_data = std::make_shared<CallbackData::Matrix>("","MainRenderer","MainWindow", new_cam_location);
		mCallbacks.at("MoveCamera")(std::dynamic_pointer_cast<CallbackData::BlankData>(camera_data));
	}

	// CALLBACK EVENT
	if (mCallbacks.find("CreateChainControls") != mCallbacks.end())
	{
		std::shared_ptr<CallbackData::MechanismData> model_data = std::make_shared<CallbackData::MechanismData>(std::make_shared<LdrMechanisms>(mechanismsLdr));
		mCallbacks.at("CreateChainControls")(std::dynamic_pointer_cast<CallbackData::BlankData>(model_data));
	}

	mMechatronic.LoadMechanisms(mechanismsLdr);
	
	if(mMechatronic.servosConnected == true)
	{
		if (!mMechatronic.Disconnect())
			return false;

		// CALLBACK EVENT
		if (mCallbacks.find("Connect_button_change_text") != mCallbacks.end())
		{
			std::shared_ptr<CallbackData::Text> model_data = std::make_shared<CallbackData::Text>("Connect_button", "Connect");
			mCallbacks.at("Connect_button_change_text")(std::dynamic_pointer_cast<CallbackData::BlankData>(model_data));
		}
	}

	return true;
}


//CALLBACK FUNCTION
bool MachineManager::GetModelsCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::ModelVecCtr> model_data = std::dynamic_pointer_cast<CallbackData::ModelVecCtr>(callback_data);

	if (model_data == nullptr)
		return false;

	//Get Models only for Renderer Name
	std::vector<std::shared_ptr<IModel>> return_models = mMechatronic.GetModels();
	
	std::vector<std::shared_ptr<IModel>> sorted_models;
	for (auto& v_model : return_models)
		if (v_model->rendererName == model_data->rendererName)
			sorted_models.push_back(v_model);

	model_data->iModels = sorted_models;
	return true;
}

//CALLBACK FUNCTION
bool MachineManager::MoveJointCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::JointMotion> joint_data = std::dynamic_pointer_cast<CallbackData::JointMotion>(callback_data);

	if (joint_data == nullptr)
		return false;
	
	bool inside_bounding_region;

	if (joint_data->prismaticJoint)
		mMechatronic.ActuateJoint(joint_data->mechanismName, joint_data->chainName, joint_data->jointName, joint_data->mNumber, inside_bounding_region);
	else
		mMechatronic.ActuateJoint(joint_data->mechanismName, joint_data->chainName, joint_data->jointName, joint_data->mNumber/57.2958F, inside_bounding_region);

	return true;
}

//CALLBACK FUNCTION
bool MachineManager::ConnectServosCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::Text> com_port = std::dynamic_pointer_cast<CallbackData::Text>(callback_data);

	//if (com_port == nullptr)
	// 	return false;

	uint16_t serial_number;
	bool needs_calibration;
	bool needs_serial_number;


	if (mMechatronic.servosConnected == false)
	{
		if (selectedComPort.find("COM") != std::string::npos)
			if (!mMechatronic.Connect(selectedComPort.c_str(), serial_number, needs_calibration, needs_serial_number))
				return false;

		// CALLBACK EVENT
		if (mCallbacks.find("Connect_button_change_text") != mCallbacks.end())
		{
			std::shared_ptr<CallbackData::Text> model_data = std::make_shared<CallbackData::Text>("Connect_button", "Disconnect");
			mCallbacks.at("Connect_button_change_text")(std::dynamic_pointer_cast<CallbackData::BlankData>(model_data));
		}

		// CALLBACK EVENT
		if (mCallbacks.find("Serial_edit_change_text") != mCallbacks.end())
		{
			std::shared_ptr<CallbackData::Text> model_data = std::make_shared<CallbackData::Text>("Serial_edit", std::to_string(serial_number));
			mCallbacks.at("Serial_edit_change_text")(std::dynamic_pointer_cast<CallbackData::BlankData>(model_data));
		}

		//Change JoinControl Values
		{
			auto it = mCallbacks.begin();
			while (it != mCallbacks.end())
			{
				if (it->first.find("ChangeCtrl_") != std::string::npos)
				{
					//Split string by character '*'
					std::stringstream full_name(it->first);
					std::vector<std::string> v_seglist;
					std::string v_segment;
					while (std::getline(full_name, v_segment, '*'))
					{
						v_seglist.push_back(v_segment);
					}
					if (v_seglist.size() == 4)
					{
						std::string mechanism_name = v_seglist[1];
						std::string chain_name = v_seglist[2];
						std::string joint_name = v_seglist[3];

						float joint_position;
						if (mMechatronic.GetJointPosition(mechanism_name, chain_name, joint_name, joint_position))
						{
							std::shared_ptr<CallbackData::Float> float_data = std::make_shared < CallbackData::Float>(joint_position);
							it->second(std::dynamic_pointer_cast<CallbackData::BlankData>(float_data));
						}
					}
				}
				it++;
			}
		}
	}
	else
	{
		if (!mMechatronic.Disconnect())
			return false;

		// CALLBACK EVENT
		if (mCallbacks.find("Connect_button_change_text") != mCallbacks.end())
		{
			std::shared_ptr<CallbackData::Text> model_data = std::make_shared<CallbackData::Text>("Connect_button", "Connect");
			mCallbacks.at("Connect_button_change_text")(std::dynamic_pointer_cast<CallbackData::BlankData>(model_data));
		}
	}

	return true;
}

//CALLBACK FUNCTION
bool MachineManager::CalibrateServosCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	if (mMechatronic.servosConnected)
		if (!mMechatronic.CalibrateAll())
			return false;

	ZeroServosCallback(std::shared_ptr<CallbackData::BlankData>()); //Need to re-zero everything after calibration

	return true;
}

//CALLBACK FUNCTION
bool MachineManager::ZeroServosCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	//if (mMechatronic.servosConnected)
	{
		auto it = mCallbacks.begin();
		while (it != mCallbacks.end())
		{
			if (it->first.find("ChangeCtrl_") != std::string::npos)
			{
				std::shared_ptr<CallbackData::Float> float_data = std::make_shared < CallbackData::Float>(0);
				it->second(std::dynamic_pointer_cast<CallbackData::BlankData>(float_data));
			}
			it++;
		}		
	}
	if (!mMechatronic.ZeroAll())
		return false;

	return true;
}

bool MachineManager::PositionEECallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::EEMotion> joint_data = std::dynamic_pointer_cast<CallbackData::EEMotion>(callback_data);

	//std::string ee_control_callback = "GetEE_" + joint_data->mechanismName + "*" + joint_data->chainName + "*EEControl";

	Matrix4x4 ee_position = Matrix4x4::Compose(Euler(joint_data->xPos, joint_data->yPos, joint_data->zPos, joint_data->xRot/57.2958, joint_data->yRot/57.2958, joint_data->zRot/57.2958));
	if (!mMechatronic.ActuateEE(joint_data->mechanismName, joint_data->chainName, ee_position, false, true))
		return false;

	return true;
}

//CALLBACK FUNCTION
bool MachineManager::SetSerialNumberCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::Int> serial_number = std::dynamic_pointer_cast<CallbackData::Int>(callback_data);

	if (serial_number == nullptr)
		return false;

	if (!mMechatronic.SetSerialNumber(serial_number->mNumber))
		return false;

	return true;
}

//CALLBACK FUNCTION
bool MachineManager::SetSelectedComPortCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::ComboBoxEntry> com_port = std::dynamic_pointer_cast<CallbackData::ComboBoxEntry>(callback_data);

	if (com_port == nullptr)
		return false;

	selectedComPort = com_port->comboText;

	return true;
}
