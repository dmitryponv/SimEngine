#include "Direct3DInterface.h"



Direct3DInterface::Direct3DInterface(std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks):
	windowsContainer(std::dynamic_pointer_cast<WindowsContainer>(os_container)),
	mCallbacks(v_callbacks)
{
	m_ClientWidth = windowsContainer->clientWidth;
	m_ClientHeight = windowsContainer->clientHeight;

	mCallbacks["Factory_GetIModel"] = std::bind(&Direct3DInterface::Factory_GetModelCallback, this, std::placeholders::_1);    //CALLBACK INIT
	mCallbacks["MoveCamera"] = std::bind(&Direct3DInterface::MoveCameraCallback, this, std::placeholders::_1);    //CALLBACK INIT
}


Direct3DInterface::~Direct3DInterface()
{
}

bool Direct3DInterface::Init(std::string renderer_name, std::string viewport_name)
{
	auto child_viewport = windowsContainer->GetWindow(viewport_name);
	if (child_viewport == nullptr)
		return false;

	// Enable run-time memory check for debug builds.
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	// Get the instance of this application.
	m_hinstance = GetModuleHandle(NULL);

	// Create the input object.  This object will be used to handle reading the keyboard input from the user.
	m_Input = new Direct3DInput(mCallbacks);
	if (!m_Input)
	{
		return false;
	}

	// Initialize the input object.
	bool result = m_Input->Initialize(windowsContainer);
	if (!result)
	{
		MessageBox(child_viewport->viewportWindow, L"Could not initialize the input object.", L"Error", MB_OK);
		return false;
	}

	std::shared_ptr<Direct3DRenderer> direct_3d_graphics = std::make_shared<Direct3DRenderer>();

	// Initialize the graphics object.
	result = direct_3d_graphics->Initialize(viewport_name, child_viewport);
	if (!result)
	{
		return false;
	}
	
	windowsContainer->AddRenderer(renderer_name, direct_3d_graphics);

	return result;
}

bool Direct3DInterface::AddViewport(std::string renderer_name, std::string viewport_name)
{
	auto v_renderer = windowsContainer->GetRenderer(renderer_name);
	if (v_renderer == nullptr)
		return false;

	auto child_viewport = windowsContainer->GetWindow(viewport_name);
	if (child_viewport == nullptr)
		return false;

	v_renderer->AddSecondaryViewport(viewport_name, child_viewport);

	return true;
}



bool Direct3DInterface::Run(std::string save_directory_name, std::string save_image_filename)  //MAIN LOOP
{
	for (auto const& v_renderer : windowsContainer->viewportRenderer)
	{
		// Do the input frame processing.
		if (!m_Input->Frame())
			return false;

		//Collect all models
		std::shared_ptr<CallbackData::ModelVecCtr> model_data = std::make_shared<CallbackData::ModelVecCtr>(v_renderer.first);
		//CALLBACK EVENT
		if (mCallbacks.find("GetMachineModels") != mCallbacks.end())
		{
			mCallbacks.at("GetMachineModels")(std::dynamic_pointer_cast<CallbackData::BlankData>(model_data));
		}

		for (auto& v_viewport : v_renderer.second->mCamera) //Just get the name of each viewport from the Camera Map
		{
			std::string viewport_name = v_viewport.first;

			if (windowsContainer->viewportWindows.find(viewport_name) != windowsContainer->viewportWindows.end())
			{
				auto cam_matrix = TO_XMMATRIX(windowsContainer->viewportWindows.at(viewport_name)->camMoveMatrix);
				

				// Finally render the graphics to the screen.
				if (!v_renderer.second->RenderScene(viewport_name, cam_matrix, model_data->iModels))
					return false;

				//Exporting images in correct orientation
				if (save_image_filename != "" && v_renderer.first != "D0")
				{
					int nub_90_deg_rot_cw = 2;
					//Pre-rotate images for export
					//if (v_renderer.first == "MainWindow" || v_renderer.first == "HT2")
					//	nub_90_deg_rot_cw = -1;
					//else if (v_renderer.first == "HT1" || v_renderer.first == "HT3")
					//	nub_90_deg_rot_cw = 1;
					std::string temp_image_filename = save_directory_name + "\\" + v_renderer.first + "\\" + save_image_filename;

					if (!v_renderer.second->RenderToTexture(viewport_name, cam_matrix, model_data->iModels, temp_image_filename, nub_90_deg_rot_cw))
						return false;
				}
			}
		}
	}
	return true;
}
	


void Direct3DInterface::Shutdown()
{
	for (auto const& v_renderer : windowsContainer->viewportRenderer)
	{
		// Release the graphics object.
		if (v_renderer.second)
		{
			v_renderer.second->Shutdown();
		}
	}
	windowsContainer->viewportRenderer.clear();

	// Release the input object.
	if (m_Input)
	{
		m_Input->Shutdown();
		delete m_Input;
		m_Input = 0;
	}

	// Shutdown the window.
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	return;
}

//bool Direct3DInterface::CreateModel(std::string viewport_name, const WCHAR* texture_path, const char * mesh_path, Matrix4x4 origin_to_world)
//{
//	auto v_renderer = windowsContainer->GetRenderer(viewport_name);
//	auto v_viewport = windowsContainer->GetViewport(viewport_name);
//	origin_to_world.mElements[3] /= 1000;
//	origin_to_world.mElements[7] /= 1000;
//	origin_to_world.mElements[11] /= 1000;
//	return v_renderer->CreateModel(texture_path, mesh_path, v_viewport->viewportWindow, TO_XMMATRIX(origin_to_world));
//}
//
//bool Direct3DInterface::DeleteModels(std::string viewport_name)
//{
//	auto v_renderer = windowsContainer->GetRenderer(viewport_name);
//	return v_renderer->DeleteModels();
//}

bool Direct3DInterface::SetupCamera(std::string renderer_name, std::string viewport_name, float p_x, float p_y, float p_z, float q_0, float q_1, float q_2, float q_3, float field_of_view, float screen_aspect)
{
	auto v_renderer = windowsContainer->GetRenderer(viewport_name);
	if (!v_renderer->SetupCamera(viewport_name, p_x, p_y, p_z, q_0, q_1, q_2, q_3, field_of_view, screen_aspect))
		return false;
	
	return true;
}

bool Direct3DInterface::SetupCamera(std::string renderer_name, std::string viewport_name, float p_x, float p_y, float p_z, float tgt_x, float tgt_y, float tgt_z, float up_x, float up_y, float up_z, float field_of_view, float screen_aspect, float roll_angle, Matrix4x4 camera_transform)
{
	auto v_renderer = windowsContainer->GetRenderer(renderer_name);
	if (v_renderer == nullptr)
		return false;
	//Convert to meters
	printf("\r\nOrig View Matrix %s\r\n", viewport_name);
	if(!v_renderer->SetupCamera(viewport_name, p_x, p_y, p_z, tgt_x, tgt_y, tgt_z, up_x, up_y, up_z, field_of_view, screen_aspect, roll_angle, TO_XMMATRIX(camera_transform)))
		return false;

	//Print view Matrix
	//DirectX::XMMATRIX view_matrix;
	//v_renderer->GetCamera()->GetViewMatrix(view_matrix);
	//Utility::PrintMatrix(viewport_name, view_matrix);

	return true;
}

bool Direct3DInterface::MoveCamera(std::string renderer_name, std::string viewport_name, Matrix4x4 new_move_matrix)
{
	//Convert to meters
	new_move_matrix.mElements[3] /= 1000;
	new_move_matrix.mElements[7] /= 1000;
	new_move_matrix.mElements[11] /= 1000;
	auto v_renderer = windowsContainer->GetRenderer(viewport_name);
	if (v_renderer == nullptr)
		return false;
	//Convert to meters
	if (!v_renderer->MoveCamera(viewport_name, TO_XMMATRIX(new_move_matrix)))
		return false;
	return true;
}

//CALLBACK FUNCTION
bool Direct3DInterface::Factory_GetModelCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::ModelCtr> item_data = std::dynamic_pointer_cast<CallbackData::ModelCtr>(callback_data);

	if (item_data == nullptr)
		return false;

	auto v_renderer = windowsContainer->GetRenderer(item_data->rendererName);
	if (v_renderer == nullptr)
		return false;

	ID3D11Device* p_device = v_renderer->GetDevice();
	ID3D11DeviceContext* p_device_context = v_renderer->GetDeviceContext();

	// Create the model container object.
	std::shared_ptr<Direct3DModel> v_model = std::make_shared<Direct3DModel>(p_device, p_device_context, item_data->modelName);
	v_model->rendererName = item_data->rendererName;
	item_data->iModel = v_model;
	
	return true;
}

//CALLBACK FUNCTION
bool Direct3DInterface::MoveCameraCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::Matrix> item_data = std::dynamic_pointer_cast<CallbackData::Matrix>(callback_data);

	if (item_data == nullptr)
		return false;

	auto v_renderer = windowsContainer->GetRenderer(item_data->rendererName);
	if (v_renderer == nullptr)
		return false;

	if (!v_renderer->MoveCamera(item_data->viewportName, TO_XMMATRIX(item_data->mMatrix)))
		return false;

	windowsContainer->camMoveMatrix = Matrix4x4();
	
	return true;
}

