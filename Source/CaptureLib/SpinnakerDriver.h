#pragma once

#include "../Containers.h"
#include "../Includes/spinnaker/Spinnaker.h"
#include "../Includes/spinnaker/SpinGenApi/SpinnakerGenApi.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <assert.h>
#include <thread>
#include <mutex>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

class SpinnakerDriver
{
public:

	SpinnakerDriver(CALLBACKS& v_callbacks);

	~SpinnakerDriver();

	bool Init();

	bool AddViewport(std::string viewport_name, std::shared_ptr<WindowContainer> window_container, int camera_index);

private:

	// This function acquires and saves 10 images from a device.
	static bool AcquireImages(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice, std::string viewport_name);

	// This function prints the device information of the camera from the transport
	// layer; please see NodeMapInfo example for more in-depth comments on printing
	// device information from the nodemap.
	static bool PrintDeviceInfo(INodeMap& nodeMap);

	static void RunSingleCamera(CameraPtr pCam, std::string viewport_name);
	   
	CameraList camList; 
	SystemPtr mSystem;
	std::map<std::string, std::thread> mThreads;
	std::vector<std::string> viewportIndeces;
	static std::map<std::string, std::vector<uint8_t>> compressedImageData;
	CameraPtr pCam = nullptr;
	unsigned int numCameras;

	CALLBACKS& mCallbacks;

	//static std::map<std::string, ImagePtr> mImages;
	static std::map<std::string, std::shared_ptr<WindowContainer>> mViewports;
	static std::mutex* mMutex;

	bool DisplayImageCallback(std::shared_ptr<CallbackData::BlankData> callback_data);

	static HBITMAP ConvertCameraImgToBitmap(uint8_t* image_ptr, int dest_width, int dest_height, int b_pp, int num_channels);

	static bool CompressImage(ImagePtr image_ptr, int dest_width, int dest_height, std::string viewport_name);

	bool CaptureButtonCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
};

