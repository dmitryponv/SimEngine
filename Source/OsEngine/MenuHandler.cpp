#include "MenuHandler.h"



MenuHandler::MenuHandler(CALLBACKS& v_callbacks):
	mCallbacks(v_callbacks)
{
}


MenuHandler::~MenuHandler()
{
}

bool MenuHandler::Init()
{
	mCallbacks["Browse_button"] = std::bind(&MenuHandler::BrowseFolderCallback, this, std::placeholders::_1); //CALLBACK INIT

	return true;
}

std::string MenuHandler::BrowseFolder(std::string saved_path)
{
	TCHAR path[MAX_PATH];

	const char * path_param = saved_path.c_str();

	BROWSEINFO bi = { 0 };
	bi.lpszTitle = (L"Browse for folder...");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	//bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)path_param;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		//get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);

		//free memory used
		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}

		std::wstring wStr = path;
		return std::string(wStr.begin(), wStr.end());
	}

	return "";
}

bool MenuHandler::BrowseFolderCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	auto ret_folder = BrowseFolder("C://");

	//CALLBACK EVENT
	if (mCallbacks.find("Save_image_directory_change") != mCallbacks.end())
	{
		std::shared_ptr<CallbackData::Text> frame_data = std::make_shared<CallbackData::Text>("Browse_folder", ret_folder);
		mCallbacks.at("Save_image_directory_change")(std::dynamic_pointer_cast<CallbackData::BlankData>(frame_data));
	}

	return true;
}
