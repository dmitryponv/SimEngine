#pragma once
#include "Containers.h"
#include "RenderManager.h"
#include "OsManager.h"
#include "InputManager.h"
#include "FileManager.h"
#include "AnimationManager.h"
#include "ConfigManager.h"
#include "MachineManager.h"
#include "CaptureManager.h"

class ProgramBuilder
{
public:
	ProgramBuilder(std::shared_ptr<BlankContainer> os_container);
	~ProgramBuilder();

	bool Init();

	void CloseApp();
	
private:
	CALLBACKS mCallbacks;

	FileManager mFileManager;
	OsManager mOsManager;
	RenderManager mRenderManager;
	InputManager mInputManager;
	AnimationManager mAnimationManager;
	ConfigManager mConfigManager;
	MachineManager mMachineManager;
	CaptureManager mCaptureManager;

	float aspectRatio;
	
};
