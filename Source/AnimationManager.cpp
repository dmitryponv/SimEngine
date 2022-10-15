#include "AnimationManager.h"



AnimationManager::AnimationManager(FileManager& file_manager, std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks):
	fileManager(file_manager),
	osContainer(os_container),
	mCallbacks(v_callbacks)
{
}


AnimationManager::~AnimationManager()
{
}

bool AnimationManager::Init()
{
	//Disable animations for now
	//mCallbacks["Animation_frame"] = std::bind(&AnimationManager::FrameCallback, this, std::placeholders::_1);   //CALLBACK INIT

	return true;
}

//CALLBACK FUNCTION
bool AnimationManager::FrameCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::Frame> frame_data = std::dynamic_pointer_cast<CallbackData::Frame>(callback_data);

	if (frame_data == nullptr)
		return false;


	auto end_time = std::chrono::steady_clock::now();
	frame_data->elapsedTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - startTime).count();
	
	return true;
}