#include "CaptureManager.h"

CaptureManager::CaptureManager(std::shared_ptr<BlankContainer> os_container, CALLBACKS & v_callbacks) :
	mCallbacks(v_callbacks),
	cameraInterface(os_container, v_callbacks)
{
}

CaptureManager::~CaptureManager()
{
}

bool CaptureManager::Init()
{	
	if (!cameraInterface.AddViewport(0, "RightCam"))
		return false;
	if (!cameraInterface.AddViewport(1, "LeftCam"))
		return false;

	if (!cameraInterface.Init())
		return false;
}
