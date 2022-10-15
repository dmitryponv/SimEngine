#pragma once
#include "Containers.h"
#include "CaptureLib/CameraInterface.h"

class CaptureManager
{
public:
	CaptureManager(std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks);
	~CaptureManager();

	bool Init();

private:

	CALLBACKS& mCallbacks;
	CameraInterface cameraInterface;
};