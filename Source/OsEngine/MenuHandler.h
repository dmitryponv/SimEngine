#pragma once

#include "../Containers.h"
#include <windows.h>
#include <shlobj.h>

class MenuHandler
{
public:
	MenuHandler(CALLBACKS& v_callbacks);
	~MenuHandler();

	bool Init();

	CALLBACKS& mCallbacks;

	std::string BrowseFolder(std::string saved_path);
	bool BrowseFolderCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
private:

};

