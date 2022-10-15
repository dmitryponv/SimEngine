#include "ConfigManager.h"


ConfigManager::ConfigManager(FileManager & file_manager, std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks):
	mCallbacks(v_callbacks)
{
}

ConfigManager::~ConfigManager()
{
}

bool ConfigManager::Init()
{
	return true;
}
