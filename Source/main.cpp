#include "Containers.h"
#include "ProgramBuilder.h"
#include <ctime>

//#define NTDDI_VERSION 0x600
//#define NTDDI_VISTA 0x600
//#pragma warning( disable : 4100 )

/*
Working with Callbacks:

Callbacks pass an abstract data type CallbackData::BlankData found in Containers.h
In order to pass data through callbacks, and existing dervied data type must be used or one must be created in CallbackData namespace
In order to set up a callback do the following, dont forget to include //CALLBACK ... comment so they are easy to find later 

1. Add callback function to callback map somewhere in the Init() function of that class
	mCallbacks["Animation_frame"] = std::bind(&AnimationManager::FrameCallback, this, std::placeholders::_1);   //CALLBACK INIT

2. Add a function to the callback and convert to derived data type
	bool AnimationManager::FrameCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
	{
		std::shared_ptr<CallbackData::Frame> frame_data = std::dynamic_pointer_cast<CallbackData::Frame>(callback_data);
		if (frame_data == nullptr)
			return false;
		//Process frame_data
		return true;
	}

3. In another class, find the callback, create a data type, fire the callback event
	//CALLBACK EVENT
	if (mCallbacks.find("Animation_frame") != mCallbacks.end())
	{
		Matrix4x4 input_matrix;
		std::shared_ptr<CallbackData::Frame> frame_data = std::make_shared<CallbackData::Frame>(input_matrix);
		mCallbacks.at("Animation_frame")(std::dynamic_pointer_cast<CallbackData::BlankData>(frame_data));
	}


*/

int WINAPI wWinMain(_In_ HINSTANCE h_instance, _In_opt_ HINSTANCE h_prev_instance, _In_ LPWSTR lp_cmd_line, _In_ int n_cmd_show) 
{
	
//#ifndef _DEBUG
//	time_t t = time(0);   // get time now
//	tm now;
//	errno_t err = localtime_s(&now, &t);
//	char date_time[27];
//	sprintf_s(date_time, 27, "DebugOut-%02d%02d%02d-%02d%02d%02d.txt",now.tm_year % 100, now.tm_mon, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
//	FILE *stream;
//	err |= freopen_s(&stream, date_time, "w", stdout);
//	if (err != 0)
//		fprintf(stdout, "error on freopen\n");
//#endif

	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	printf("Debugging Window:\n");

	std::shared_ptr<WindowsContainer> os_container = std::make_shared<WindowsContainer>();
	os_container->hInstance = h_instance;
	os_container->hPrevInstance = h_prev_instance;
	os_container->clientWidth = 3000;
	os_container->clientHeight = 1200;
	ProgramBuilder program_builder(os_container);
	
	if (!program_builder.Init()) return 0;
	
	return 1;
}