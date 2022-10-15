#include "CameraInterface.h"

CameraInterface::CameraInterface(std::shared_ptr<BlankContainer> os_container, CALLBACKS & v_callbacks):
	windowsContainer(std::dynamic_pointer_cast<WindowsContainer>(os_container)),
	mCallbacks(v_callbacks),
	spinnakerDriver(v_callbacks)
{
}

CameraInterface::~CameraInterface()
{
}

bool CameraInterface::Init()
{
	if(!spinnakerDriver.Init())
		return false;

	return true;
}

bool CameraInterface::AddViewport(int camera_index, std::string viewport_name)
{
	auto child_viewport = windowsContainer->GetWindow(viewport_name);
	if (child_viewport == nullptr)
		return false;

	spinnakerDriver.AddViewport(viewport_name, child_viewport, camera_index);

	return true;
}
