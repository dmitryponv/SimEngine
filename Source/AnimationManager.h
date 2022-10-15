#pragma once

#include "FileManager.h"
#include <chrono>

class AnimationManager
{
public:
	AnimationManager(FileManager& file_manager, std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks);
	~AnimationManager();

	bool Init();

private:
	bool FrameCallback(std::shared_ptr<CallbackData::BlankData> callback_data);


	FileManager& fileManager;
	CALLBACKS& mCallbacks;
	std::shared_ptr<BlankContainer> osContainer;

	std::chrono::time_point<std::chrono::steady_clock> startTime;
};

