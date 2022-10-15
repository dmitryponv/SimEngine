#pragma once
#include "Containers.h"
#include "OsEngine/WindowsInterface.h"
#include "OsEngine/MenuHandler.h"
#include "FileManager.h"
#include "MechatronicLib/Loader.h"

class OsManager
{
public:
	OsManager(FileManager& file_manager, std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks);
	~OsManager();

	bool Init();

	bool CreateChainControlsCallback(std::shared_ptr<CallbackData::BlankData> callback_data);

	int rendererWidth = 1600;
	int rendererHeight = 1200;

private:

	CALLBACKS& mCallbacks;
	FileManager& fileManager;
	WindowsInterface osInterface;
	MenuHandler menuHandler;
};

