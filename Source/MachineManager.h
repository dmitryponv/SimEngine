#pragma once
#include "Containers.h"
#include "MechatronicLib\Mechatronic.h"

class MachineManager
{
public:
	MachineManager(std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks);
	~MachineManager();

	bool Init();



private:


	CALLBACKS& mCallbacks;
	bool ConfigSelectCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool ConfigDDCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool ConfigButtonCallback(std::shared_ptr<CallbackData::BlankData> callback_data);

	bool GetModelsCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool MoveJointCallback(std::shared_ptr<CallbackData::BlankData> callback_data);


	bool SetSerialNumberCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool SetSelectedComPortCallback(std::shared_ptr<CallbackData::BlankData> callback_data);

	bool ConnectServosCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool CalibrateServosCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool ZeroServosCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool ActuateEECallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool PositionEECallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	
	Mechatronic mMechatronic;
	LdrMechanisms mechanismsLdr;

	std::string selectedComPort = "";
	std::string configName = "";
};

