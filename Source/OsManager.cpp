#include "OsManager.h"

OsManager::OsManager(FileManager& file_manager, std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks):
	mCallbacks(v_callbacks),
	fileManager(file_manager),
	osInterface(os_container, v_callbacks),
	menuHandler(v_callbacks)
{
	mCallbacks["CreateChainControls"] = std::bind(&OsManager::CreateChainControlsCallback, this, std::placeholders::_1);   //CALLBACK INIT
}


OsManager::~OsManager()
{
}

bool OsManager::Init()
{
	if (!osInterface.Init())
		return false;

	if (!menuHandler.Init())
		return false;


	RECT v_position;
	v_position.left = 0;
	v_position.top = 0;
	v_position.right = rendererWidth;
	v_position.bottom = rendererHeight;
	if (!osInterface.AddViewport("MainWindow", v_position))
		return false;

	v_position.left = 2000;
	v_position.top = 0;
	v_position.right = 500;
	v_position.bottom = 500;
	if (!osInterface.AddViewport("RightEye", v_position))
		return false;

	v_position.left = 2000;
	v_position.top = 600;
	v_position.right = 500;
	v_position.bottom = 500;
	if (!osInterface.AddViewport("LeftEye", v_position))
		return false;

	v_position.left = 2600;
	v_position.top = 0;
	v_position.right = 400;
	v_position.bottom = 300;
	if (!osInterface.AddViewport("RightCam", v_position))
		return false;

	v_position.left = 2600;
	v_position.top = 600;
	v_position.right = 400;
	v_position.bottom = 300;
	if (!osInterface.AddViewport("LeftCam", v_position))
		return false;
	
	v_position.left = rendererWidth;
	v_position.top = 0;
	v_position.right = 100;
	v_position.bottom = 200;
	if (!osInterface.AddDropdown("Config_Select", v_position, fileManager.GetTargetNames()))
		return false;

	v_position.left = rendererWidth;
	v_position.top = 30;
	v_position.right = 100;
	v_position.bottom = 300;
	if (!osInterface.AddDropdown("COMPort_Dropdown", v_position, fileManager.GetComPorts()))
		return false;
	
	v_position.left = rendererWidth+100;
	v_position.top = 0;
	v_position.right = 100;
	v_position.bottom = 30;
	if (!osInterface.AddButton("Config_button", v_position, "Change Config"))
		return false;

	v_position.left = rendererWidth+100;
	v_position.top = 30;
	v_position.right = 50;
	v_position.bottom = 30;
	if (!osInterface.AddButton("Cam1_button", v_position, "CAM1"))
		return false;
	
	v_position.left = rendererWidth+150;
	v_position.top = 30;
	v_position.right = 50;
	v_position.bottom = 30;
	if (!osInterface.AddButton("Cam2_button", v_position, "CAM2"))
		return false;
	
	v_position.left = rendererWidth;
	v_position.top = 60;
	v_position.right = 100;
	v_position.bottom = 30;
	if (!osInterface.AddButton("Connect_button", v_position, "Connect"))
		return false;
	
	v_position.left = rendererWidth;
	v_position.top = 90;
	v_position.right = 100;
	v_position.bottom = 30;
	if (!osInterface.AddButton("Calibrate_button", v_position, "Calibrate"))
		return false;
	
	v_position.left = rendererWidth;
	v_position.top = 130;
	v_position.right = 50;
	v_position.bottom = 30;
	if (!osInterface.AddLabel("Unused_label_1", v_position, "Serial#"))
		return false;
	
	v_position.left = rendererWidth+50;
	v_position.top = 130;
	v_position.right = 50;
	v_position.bottom = 30;
	if (!osInterface.AddEdit("Serial_edit", v_position, ""))
		return false;

	v_position.left = rendererWidth+100;
	v_position.top = 130;
	v_position.right = 50;
	v_position.bottom = 30;
	if (!osInterface.AddButton("SetSerial_button", v_position, "Set"))
		return false;

	v_position.left = rendererWidth+20;
	v_position.top = 160;
	v_position.right = 80;
	v_position.bottom = 30;
	if (!osInterface.AddButton("Zero_button", v_position, "Zero Joints"))
		return false;

	v_position.left = rendererWidth+100;
	v_position.top = 160;
	v_position.right = 80;
	v_position.bottom = 30;
	if (!osInterface.AddButton("MoveAll_button", v_position, "Move All"))
		return false;


	v_position.left = rendererWidth + 200;
	v_position.top = 0;
	v_position.right = 80;
	v_position.bottom = 30;
	if (!osInterface.AddButton("Capture_button", v_position, "Capture"))
		return false;

	return true;
}

//CALLBACK FUNCTION
bool OsManager::CreateChainControlsCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::MechanismData> mechanism_ldr = std::dynamic_pointer_cast<CallbackData::MechanismData>(callback_data);

	if (mechanism_ldr == nullptr)
		return false;


	osInterface.RemoveJointControls();


	//CREATE SLIDER/UPDOWN CONTROLS FOR EACH JOINT
	RECT v_position;
	v_position.left = rendererWidth;
	v_position.top = 250;
	v_position.right = 0;
	v_position.bottom = 0;

	RECT v_position2;
	v_position2.left = rendererWidth+200;
	v_position2.top = 250;
	v_position2.right = 0;
	v_position2.bottom = 0;
	int mechanism_range = 0;
	for (auto& v_mechanism : mechanism_ldr->mMechanisms->ldrMechanisms)
	{
		for (auto& v_chain : v_mechanism.Chains)
		{
			for (auto& v_joint : v_chain.Joints)
			{
				if (!v_joint.staticJoint)
				{
					v_position.left = rendererWidth;
					std::string menu_item_name = v_mechanism.mechanismName + v_chain.chainName + v_joint.jointName;
					if (!osInterface.AddJointControl(menu_item_name, v_position, v_mechanism.mechanismName, v_chain.chainName, v_joint.jointName, v_joint.prismaticJoint, (int)v_joint.minRange, (int)v_joint.maxRange))
						return false;
					v_position.top += 50;
					mechanism_range += (int)v_joint.maxRange;
					//jointControls.emplace_back(std::make_shared<JointControl>(eyeCtrl, v_mechanism.mechanismName, v_chain.chainName,
					//	v_joint.jointName, v_joint.prismaticJoint, (int)v_joint.minRange, (int)v_joint.maxRange));
				}
			}
			if (v_chain.useIK)
			{
				std::string menu_item_name = v_mechanism.mechanismName + v_chain.chainName + "EE";
				if (!osInterface.AddEEControl(menu_item_name, v_position2, v_mechanism.mechanismName, v_chain.chainName, (int)-mechanism_range, (int)mechanism_range))
					return false;
				v_position2.top += 250;
			}
		}
	}

	//for (int i = 0; i< jointControls.size(); i++)
	//	jointControls[i]->CreateControl(m_hAppWnd, &m_hAppInstance, 600, 200 + i * 50);
	//
	//for (int i = 0; i< eeControls.size(); i++)
	//	eeControls[i]->CreateControl(m_hAppWnd, &m_hAppInstance, 0, 520 + i * 30);
	//
	//eyeCtrl.UpdateEEControls();
	return true;
}