#include "RenderManager.h"


RenderManager::RenderManager(FileManager& file_manager, std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks):
	fileManager(file_manager),
	renderingInterface(os_container, v_callbacks),
	mCallbacks(v_callbacks)
{
}


RenderManager::~RenderManager()
{

}

bool RenderManager::Init()
{
	mCallbacks["Save_image_directory_change"] = std::bind(&RenderManager::SaveImageDCCallback, this, std::placeholders::_1); //CALLBACK INIT
	mCallbacks["Save_image_checkbox"] = std::bind(&RenderManager::SaveImageCBCallback, this, std::placeholders::_1); //CALLBACK INIT
	

	if (!renderingInterface.Init("MainRenderer","MainWindow"))
		return false;

	if (!renderingInterface.AddViewport("MainRenderer", "RightEye"))
		return false;
	if (!renderingInterface.AddViewport("MainRenderer", "LeftEye"))
		return false;
	
	Matrix4x4 camera_transform = Matrix4x4();
	if (!renderingInterface.SetupCamera("MainRenderer", "MainWindow", 0, 150.000, -150.000, 0, 0, 0, 0, 0, 1, (float)XM_PI / 2.0f, 1200.0F / 1600.0F, 0, camera_transform))
		return false;

	//if (!renderingInterface.SetupCamera("MainRenderer", "RightEye", 30.500,  -109.000, -34.890,		 30.500,  -109.100, -40, 	0, 0, 1, (float)XM_PI / 2.0f, 1200.0F / 1600.0F, 0, camera_transform))
	//	return false;
	//
	//if (!renderingInterface.SetupCamera("MainRenderer", "LeftEye", -30.500, -109.000, -34.890,		-30.500,  -109.100, -40, 	0, 0, 1, (float)XM_PI / 2.0f, 1200.0F / 1600.0F, 0, camera_transform))
	//	return false;

	if (!renderingInterface.SetupCamera("MainRenderer", "RightEye", 0, 0, 0,	0, 0, -1,	0, 1, 0, (float)XM_PI / 2.0f, 1200.0F / 1600.0F, 0, camera_transform))
		return false;
	
	if (!renderingInterface.SetupCamera("MainRenderer", "LeftEye",  0, 0, 0,	0, 0, -1,	0, 1, 0, (float)XM_PI / 2.0f, 1200.0F / 1600.0F, 0, camera_transform))
		return false;

	return true;
}


bool RenderManager::Run()
{
	MSG msg;
	bool done;
	
	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	done = false;
	//Measure framerate
	clock_t current_ticks, delta_ticks;
	clock_t fps = 0;

	while (!done)
	{
		current_ticks = clock();

		// Handle the windows messages.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if (msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			//First process animation
			std::string image_filename = "";
			//CALLBACK EVENT
			if (mCallbacks.find("Animation_frame") != mCallbacks.end())
			{
				Matrix4x4 input_matrix;
				std::shared_ptr<CallbackData::Frame> frame_data = std::make_shared<CallbackData::Frame>(input_matrix);
				mCallbacks.at("Animation_frame")(std::dynamic_pointer_cast<CallbackData::BlankData>(frame_data));

				//Move each camera to animation position
				if (!renderingInterface.MoveCamera("MainRenderer", "MainWindow", input_matrix))
					return false;

				//Generate Filename if capture is rolling
				if (saveDirectoryName != "" && saveImagesCheckbox && frame_data->playButtonClicked)
				{
					image_filename = to_string(frame_data->elapsedTime);
				}
			}
			
			//Render all cameras
			if (!renderingInterface.Run(saveDirectoryName, image_filename))
				done = true;
		}

		// Check if the user pressed escape and wants to quit.
		if (renderingInterface.IsEscapePressed() == true)
		{
			done = true;
		}

		//Framerate
		delta_ticks = clock() - current_ticks;
		if (delta_ticks > 0)
			fps = CLOCKS_PER_SEC / delta_ticks;
		//DebugOut("%ld\r\n", fps);
	}

	renderingInterface.Shutdown();
	return true;
}

bool RenderManager::SaveImageDCCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::Text> save_directory_name = std::dynamic_pointer_cast<CallbackData::Text>(callback_data);

	if (save_directory_name == nullptr)
		return false;
	
	saveDirectoryName = save_directory_name->itemText;

	return true;
}

bool RenderManager::SaveImageCBCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::CheckBox> save_images = std::dynamic_pointer_cast<CallbackData::CheckBox>(callback_data);

	if (save_images == nullptr)
		return false;

	saveImagesCheckbox = save_images->mChecked;

	return true;
}


