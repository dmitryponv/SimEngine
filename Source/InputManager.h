#pragma once
#include "Containers.h"


class InputManager
{
public:
	InputManager(std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks);
	~InputManager();


	bool Init();


private:
	bool MouseCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool KeyboardCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool MouseWheelCallback(std::shared_ptr<CallbackData::BlankData> callback_data);

	int mouseXposPrev = -1, mouseYposPrev = -1;

	std::shared_ptr<BlankContainer> osContainer;
	CALLBACKS& mCallbacks;
};

