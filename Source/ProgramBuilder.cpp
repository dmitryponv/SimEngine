#include "ProgramBuilder.h"

ProgramBuilder::ProgramBuilder(std::shared_ptr<BlankContainer> os_container) :
	mFileManager(mCallbacks),
	mOsManager(mFileManager, os_container, mCallbacks),
	mRenderManager(mFileManager, os_container, mCallbacks),
	mInputManager(os_container, mCallbacks),
	mAnimationManager(mFileManager, os_container, mCallbacks),
	mConfigManager(mFileManager, os_container, mCallbacks),
	mMachineManager(os_container, mCallbacks),
	mCaptureManager(os_container, mCallbacks)
{	

}

ProgramBuilder::~ProgramBuilder()
{
}

bool ProgramBuilder::Init()
{
	if (!mInputManager.Init())
		return false;
	if (!mOsManager.Init())
		return false;
	if (!mRenderManager.Init())
		return false;
	if (!mAnimationManager.Init())
		return false;
	if (!mConfigManager.Init())
		return false;
	if (!mMachineManager.Init())
		return false;
	if (!mCaptureManager.Init())
		return false;

	//Init all windows callback
	// CALLBACK EVENT
	if (mCallbacks.find("TriggerInitMenuItemsCallback") != mCallbacks.end())
	{
		mCallbacks.at("TriggerInitMenuItemsCallback")(std::shared_ptr<CallbackData::BlankData>());
	}

#ifdef _DEBUG
	//Print out all callbacks
	for (auto& v_callback : mCallbacks)
	{
		wprintf(L"callback_name: %s\r\n", TOLPCWSTR(v_callback.first));
	}
#endif

	if (!mRenderManager.Run())
		return false;

	return true;
}


void ProgramBuilder::CloseApp()
{

}