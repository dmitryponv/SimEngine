#pragma once
#include "RenderEngine/Direct3DInterface.h"
#include "FileManager.h"

class RenderManager
{
public:
	RenderManager(FileManager& file_manager, std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks);
	~RenderManager();

	bool Init();

	//bool CreateModel(std::string viewport_name, const WCHAR* texture_path, const char* mesh_path, Matrix4x4 origin_to_world);

	bool Run();
	
protected:
	
	UINT		m_ClientWidth;
	UINT		m_ClientHeight;
	
private:
	FileManager& fileManager;
	Direct3DInterface renderingInterface;
	std::string saveDirectoryName;
	bool saveImagesCheckbox = false;

	LPCWSTR m_applicationName;

	CALLBACKS& mCallbacks;

	//bool TargetDDCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool SaveImageDCCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool SaveImageCBCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
};

