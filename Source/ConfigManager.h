#pragma once
#include "FileManager.h"
#include "MechatronicLib\Loader.h"

class ConfigManager
{
public:
	ConfigManager(FileManager& file_manager, std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks);
	~ConfigManager();

	bool Init();

private:

	CALLBACKS& mCallbacks;
};

