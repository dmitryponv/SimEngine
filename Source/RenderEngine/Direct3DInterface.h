#pragma once
#include <time.h>
#include "../Containers.h"
#include "Direct3D/Direct3DRenderer.h"
#include "Direct3D/Direct3DInput.h"
#include "Direct3D/DXUtil.h"

#include "Direct3D\Direct3DModel.h"

using namespace DirectX;

class Direct3DInterface
{
public:
	Direct3DInterface(const  std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks);
	~Direct3DInterface();

	bool Init(std::string renderer_name, std::string viewport_name);
	bool AddViewport(std::string renderer_name, std::string viewport_name);

	bool Run(std::string save_directory_name = "", std::string save_image_filename = "");
	void Shutdown();

	//bool CreateModel(std::string viewport_name, const WCHAR* texture_path, const char* mesh_path, Matrix4x4 origin_to_world); //TODO: remove, obsolete
	//bool DeleteModels(std::string viewport_name);

	bool SetupCamera(std::string renderer_name, std::string viewport_name, float p_x, float p_y, float p_z, float q_0, float q_1, float q_2, float q_3, float field_of_view, float screen_aspect);
	bool SetupCamera(std::string renderer_name, std::string viewport_name, float p_x, float p_y, float p_z, float tgt_x, float tgt_y, float tgt_z, float up_x, float up_y, float up_z, float field_of_view, float screen_aspect, float roll_angle, Matrix4x4 camera_transform);
	bool MoveCamera(std::string renderer_name, std::string viewport_name, Matrix4x4 new_move_matrix);

	bool Factory_GetModelCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool MoveCameraCallback(std::shared_ptr<CallbackData::BlankData> callback_data);

	bool IsEscapePressed()
	{
		return m_Input->IsEscapePressed();
	}

private:
	
	UINT		m_ClientWidth;
	UINT		m_ClientHeight;

	std::shared_ptr<WindowsContainer> windowsContainer;

	Direct3DInput* m_Input;
	
	HINSTANCE m_hinstance;

	CALLBACKS& mCallbacks;
};

