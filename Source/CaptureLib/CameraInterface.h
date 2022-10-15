#pragma once
#include "..\Containers.h"
#include "SpinnakerDriver.h"

class CameraInterface
{
public:
	CameraInterface(std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks);
	~CameraInterface();

	bool Init();

	bool AddViewport(int camera_index, std::string viewport_name);

private:
	CALLBACKS& mCallbacks;

	std::shared_ptr<WindowsContainer> windowsContainer;
	SpinnakerDriver spinnakerDriver;
};