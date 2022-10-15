#include "SpinnakerDriver.h"

//std::map<std::string, Spinnaker::ImagePtr> SpinnakerDriver::mImages = std::map<std::string, Spinnaker::ImagePtr>();
std::map<std::string, std::shared_ptr<WindowContainer>> SpinnakerDriver::mViewports = std::map<std::string, std::shared_ptr<WindowContainer>>();
std::mutex* SpinnakerDriver::mMutex = new std::mutex();
std::map<std::string, std::vector<uint8_t>> SpinnakerDriver::compressedImageData = std::map<std::string, std::vector<uint8_t>>();

SpinnakerDriver::SpinnakerDriver(CALLBACKS & v_callbacks):
	mCallbacks(v_callbacks)
{
}

SpinnakerDriver::~SpinnakerDriver()
{
	pCam = nullptr;
	// Clear camera list before releasing system
	camList.Clear();

	// Release system
	mSystem->ReleaseInstance();
}

bool SpinnakerDriver::Init()
{
	// Since this application saves images in the current folder
// we must ensure that we have permission to write to this folder.
// If we do not have permission, fail right away.

	mCallbacks["DisplayCameraImage"] = std::bind(&SpinnakerDriver::DisplayImageCallback, this, std::placeholders::_1); //CALLBACK INIT
	mCallbacks["Capture_button"] = std::bind(&SpinnakerDriver::CaptureButtonCallback, this, std::placeholders::_1);   //CALLBACK INIT

	FILE* tempFile = fopen("test.txt", "w+");
	if (tempFile == nullptr)
	{
		std::cout << "Failed to create file in current folder.  Please check "
			"permissions."
			<< std::endl;
		std::cout << "Press Enter to exit..." << std::endl;
		getchar();
		return false;
	}
	fclose(tempFile);
	remove("test.txt");

	// Print application build information
	std::cout << "Application build date: " << __DATE__ << " " << __TIME__ << std::endl << std::endl;

	mSystem = System::GetInstance();
	// Print out current library version
	const LibraryVersion spinnakerLibraryVersion = mSystem->GetLibraryVersion();
	std::cout << "Spinnaker library version: " << spinnakerLibraryVersion.major << "." << spinnakerLibraryVersion.minor
		<< "." << spinnakerLibraryVersion.type << "." << spinnakerLibraryVersion.build << std::endl
		<< std::endl;
	camList = mSystem->GetCameras();
	numCameras = camList.GetSize();

	std::cout << "Number of cameras detected: " << numCameras << std::endl << std::endl;

	// Finish if there are no cameras
	if (numCameras == 0)
	{
		// Clear camera list before releasing system
		camList.Clear();

		// Release system
		mSystem->ReleaseInstance();

		std::cout << "Not enough cameras!" << std::endl;

		return true;
	}

	//
	// Create shared pointer to camera
	//
	// *** NOTES ***
	// The CameraPtr object is a shared pointer, and will generally clean itself
	// up upon exiting its scope. However, if a shared pointer is created in the
	// same scope that a system object is explicitly released (i.e. this scope),
	// the reference to the shared point must be broken manually.
	//
	// *** LATER ***
	// Shared pointers can be terminated manually by assigning them to nullptr.
	// This keeps releasing the system from throwing an exception.
	//
	

	//
	// Release reference to the camera
	//
	// *** NOTES ***
	// Had the CameraPtr object been created within the for-loop, it would not
	// be necessary to manually break the reference because the shared pointer
	// would have automatically cleaned itself up upon exiting the loop.
	//
	
	//std::cout << std::endl << "Done! Press Enter to exit..." << std::endl;
	//getchar();

	return true;
}

bool SpinnakerDriver::AddViewport(std::string viewport_name, std::shared_ptr<WindowContainer> window_container, int camera_index)
{
	//Add HWND to viewports
	if (mViewports.find(viewport_name) != mViewports.end())
		return false;
	mViewports.emplace(std::make_pair(viewport_name, window_container));

	//Add to index
	if (viewportIndeces.size() <= camera_index)
		viewportIndeces.resize(camera_index + 1);
	viewportIndeces[camera_index] = viewport_name;

	//Add compressed image data arrays
	if (compressedImageData.find(viewport_name) != compressedImageData.end())
		return false;
	compressedImageData.emplace(std::make_pair(viewport_name, std::vector<uint8_t>(window_container->viewportWidth * window_container->viewportHeight*3)));

	return true;
}

bool SpinnakerDriver::AcquireImages(CameraPtr pCam, INodeMap& nodeMap, INodeMap& nodeMapTLDevice, std::string viewport_name)
{
	std::cout << std::endl << std::endl << "*** IMAGE ACQUISITION ***" << std::endl << std::endl;

	try
	{
		//
		// Set acquisition mode to continuous
		//
		// *** NOTES ***
		// Because the example acquires and saves 10 images, setting acquisition
		// mode to continuous lets the example finish. If set to single frame
		// or multiframe (at a lower number of images), the example would just
		// hang. This would happen because the example has been written to
		// acquire 10 images while the camera would have been programmed to
		// retrieve less than that.
		//
		// Setting the value of an enumeration node is slightly more complicated
		// than other node types. Two nodes must be retrieved: first, the
		// enumeration node is retrieved from the nodemap; and second, the entry
		// node is retrieved from the enumeration node. The integer value of the
		// entry node is then set as the new value of the enumeration node.
		//
		// Notice that both the enumeration and the entry nodes are checked for
		// availability and readability/writability. Enumeration nodes are
		// generally readable and writable whereas their entry nodes are only
		// ever readable.
		//
		// Retrieve enumeration node from nodemap
		CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
		if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
		{
			std::cout << "Unable to set acquisition mode to continuous (enum retrieval). Aborting..." << std::endl << std::endl;
			return false;
		}

		// Retrieve entry node from enumeration node
		CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
		if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
		{
			std::cout << "Unable to set acquisition mode to continuous (entry retrieval). Aborting..." << std::endl << std::endl;
			return false;
		}

		// Retrieve integer value from entry node
		const int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();

		// Set integer value from entry node as new value of enumeration node
		ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

		std::cout << "Acquisition mode set to continuous..." << std::endl;

#ifdef _DEBUG
		std::cout << std::endl << std::endl << "*** DEBUG ***" << std::endl << std::endl;

		// If using a GEV camera and debugging, should disable heartbeat first to prevent further issues
		//if (DisableHeartbeat(nodeMap, nodeMapTLDevice) != 0)
		//{
		//	return -1;
		//}

		std::cout << std::endl << std::endl << "*** END OF DEBUG ***" << std::endl << std::endl;
#endif

		//
		// Begin acquiring images
		//
		// *** NOTES ***
		// What happens when the camera begins acquiring images depends on the
		// acquisition mode. Single frame captures only a single image, multi
		// frame captures a set number of images, and continuous captures a
		// continuous stream of images. Because the example calls for the
		// retrieval of 10 images, continuous mode has been set.
		//
		// *** LATER ***
		// Image acquisition must be ended when no more images are needed.
		//
		pCam->BeginAcquisition();

		std::cout << "Acquiring images..." << std::endl;

		//
		// Retrieve device serial number for filename
		//
		// *** NOTES ***
		// The device serial number is retrieved in order to keep cameras from
		// overwriting one another. Grabbing image IDs could also accomplish
		// this.
		//
		gcstring deviceSerialNumber("");
		CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceSerialNumber");
		if (IsAvailable(ptrStringSerial) && IsReadable(ptrStringSerial))
		{
			deviceSerialNumber = ptrStringSerial->GetValue();

			std::cout << "Device serial number retrieved as " << deviceSerialNumber << "..." << std::endl;
		}
		std::cout << std::endl;

		// Retrieve, convert, and save images
		const unsigned int k_numImages = 1;

		for (unsigned int imageCnt = 0; imageCnt < k_numImages; imageCnt++)
		{
			try
			{
				//
				// Retrieve next received image
				//
				// *** NOTES ***
				// Capturing an image houses images on the camera buffer. Trying
				// to capture an image that does not exist will hang the camera.
				//
				// *** LATER ***
				// Once an image from the buffer is saved and/or no longer
				// needed, the image must be released in order to keep the
				// buffer from filling up.
				//
				ImagePtr pResultImage = pCam->GetNextImage(1000);

				//
				// Ensure image completion
				//
				// *** NOTES ***
				// Images can easily be checked for completion. This should be
				// done whenever a complete image is expected or required.
				// Further, check image status for a little more insight into
				// why an image is incomplete.
				//
				if (pResultImage->IsIncomplete())
				{
					// Retrieve and print the image status description
					std::cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus())
						<< "..." << std::endl
						<< std::endl;
				}
				else
				{
					//
					// Print image information; height and width recorded in pixels
					//
					// *** NOTES ***
					// Images have quite a bit of available metadata including
					// things such as CRC, image status, and offset values, to
					// name a few.
					//
					const size_t width = pResultImage->GetWidth();

					const size_t height = pResultImage->GetHeight();

					std::cout << "Grabbed image " << imageCnt << ", width = " << width << ", height = " << height << std::endl;

					//
					// Convert image to mono 8
					//
					// *** NOTES ***
					// Images can be converted between pixel formats by using
					// the appropriate enumeration value. Unlike the original
					// image, the converted one does not need to be released as
					// it does not affect the camera buffer.
					//
					// When converting images, color processing algorithm is an
					// optional parameter.
					//
					//std::lock_guard<std::mutex> guard(*mMutex);

					CompressImage(pResultImage, mViewports[viewport_name]->viewportWidth, mViewports[viewport_name]->viewportHeight, viewport_name);

					//convertedImage->Save((viewport_name + " " + filename.str()).c_str());
				}

				//
				// Release image
				//
				// *** NOTES ***
				// Images retrieved directly from the camera (i.e. non-converted
				// images) need to be released in order to keep from filling the
				// buffer.
				//
				pResultImage->Release();

				std::cout << std::endl;
			}
			catch (Spinnaker::Exception& e)
			{
				std::cout << "Error: " << e.what() << std::endl;
				pCam->EndAcquisition();
				return false;
			}
		}

		//
		// End acquisition
		//
		// *** NOTES ***
		// Ending acquisition appropriately helps ensure that devices clean up
		// properly and do not need to be power-cycled to maintain integrity.
		//

		pCam->EndAcquisition();
	}
	catch (Spinnaker::Exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
		pCam->EndAcquisition();
		return false;
	}

	return true;
}

bool SpinnakerDriver::PrintDeviceInfo(INodeMap & nodeMap)
{
    std::cout << std::endl << "*** DEVICE INFORMATION ***" << std::endl << std::endl;

    try
    {
        FeatureList_t features;
        const CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
        if (IsAvailable(category) && IsReadable(category))
        {
            category->GetFeatures(features);

            for (auto it = features.begin(); it != features.end(); ++it)
            {
                const CNodePtr pfeatureNode = *it;
                std::cout << pfeatureNode->GetName() << " : ";
                CValuePtr pValue = static_cast<CValuePtr>(pfeatureNode);
                std::cout << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
                std::cout << std::endl;
            }
        }
        else
        {
            std::cout << "Device control information not available." << std::endl;
			return false;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        std::cout << "Error: " << e.what() << std::endl;
		return false;
    }

    return true;
}

void SpinnakerDriver::RunSingleCamera(CameraPtr pCam, std::string viewport_name)
{
	try
	{
		// Retrieve TL device nodemap and print device information
		INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();

		PrintDeviceInfo(nodeMapTLDevice);

		// Initialize camera
		pCam->Init();

		// Retrieve GenICam nodemap
		INodeMap& nodeMap = pCam->GetNodeMap();

		// Acquire images
		AcquireImages(pCam, nodeMap, nodeMapTLDevice, viewport_name);

		// Deinitialize camera
		pCam->DeInit();
	}
	catch (Spinnaker::Exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}

	return;
}

//CALLBACK FUNCTION
bool SpinnakerDriver::DisplayImageCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	for (auto& v_viewport : mViewports)
	{
		std::string viewport_name = v_viewport.first;
		HWND v_window = v_viewport.second->viewportWindow;
		int dest_width = v_viewport.second->viewportWidth;
		int dest_height = v_viewport.second->viewportHeight;

		PAINTSTRUCT     ps;
		HDC				hdc;
		HDC				hdcMem;
		BITMAP			bitmap;
		HBITMAP			hBitmap;
		HGDIOBJ         oldBitmap;

		if (compressedImageData.find(viewport_name) == compressedImageData.end())
			return false;
		uint8_t* data = compressedImageData[viewport_name].data();

		hBitmap = ConvertCameraImgToBitmap(data, dest_width, dest_height, 8, 3);
		if (hBitmap != nullptr)
		{
			hdc = BeginPaint(v_window, &ps);
			hdcMem = CreateCompatibleDC(hdc);
			oldBitmap = SelectObject(hdcMem, hBitmap);
			GetObject(hBitmap, sizeof(bitmap), &bitmap);
			BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
			SelectObject(hdcMem, oldBitmap);
			DeleteDC(hdcMem);
			EndPaint(v_window, &ps);
		}
	}
	return true;
}

HBITMAP SpinnakerDriver::ConvertCameraImgToBitmap(uint8_t* image_ptr, int dest_width, int dest_height, int b_pp, int num_channels)
{	
	auto headerInfo = BITMAPINFOHEADER{};
	ZeroMemory(&headerInfo, sizeof(headerInfo));
	
	headerInfo.biSize = sizeof(headerInfo);
	headerInfo.biWidth = dest_width;
	headerInfo.biHeight = -(dest_height); // negative otherwise it will be upsidedown
	headerInfo.biPlanes = 1;// must be set to 1 as per documentation
	
	headerInfo.biBitCount = num_channels * b_pp;
	headerInfo.biCompression = BI_RGB;
	
	auto bitmapInfo = BITMAPINFO{};
	ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));
	
	bitmapInfo.bmiHeader = headerInfo;
	bitmapInfo.bmiColors->rgbBlue = 0;
	bitmapInfo.bmiColors->rgbGreen = 0;
	bitmapInfo.bmiColors->rgbRed = 0;
	bitmapInfo.bmiColors->rgbReserved = 0;
		
	auto dc = GetDC(nullptr);
	if (dc == nullptr)
		return nullptr;
	auto bmp = CreateDIBitmap(dc,
		&headerInfo,
		CBM_INIT,
		(void*)image_ptr,
		&bitmapInfo,
		DIB_RGB_COLORS);
	return bmp;
}

bool SpinnakerDriver::CompressImage(ImagePtr image_ptr, int dest_width, int dest_height, std::string viewport_name)
{
	int v_width = image_ptr->GetWidth();
	int v_height = image_ptr->GetHeight();
	uint8_t* data_ptr = (uint8_t*)(image_ptr->GetData());

	int stride = image_ptr->GetStride();

	if (compressedImageData.find(viewport_name) == compressedImageData.end())
		return false;
	uint8_t* data = compressedImageData[viewport_name].data();

	//Scale the image
	for (int x = 0; x < dest_width; x++)
		for (int y = 0; y < dest_height; y++)
		{
			int x_src = x * v_width / dest_width;
			int y_src = y * v_height / dest_height;
			if (x_src < v_width && y_src < v_height)
			{	
				data[3*(x + dest_width * y)+0] = data_ptr[x_src + stride * y_src];
				data[3*(x + dest_width * y)+1] = data_ptr[x_src + stride * y_src];
				data[3*(x + dest_width * y)+2] = data_ptr[x_src + stride * y_src];
			}
		}

	return true;
}

//CALLBACK FUNCTION
bool SpinnakerDriver::CaptureButtonCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	// Run example on each camera
	for (unsigned int camera_index = 0; camera_index < numCameras; camera_index++)
	{
		// Select camera
		pCam = camList.GetByIndex(camera_index);

		std::cout << std::endl << "Running example for camera " << camera_index << "..." << std::endl;

		// Run example
		//result = result | RunSingleCamera(pCam);

		if (viewportIndeces[camera_index] == "") //Viewport not created, continue running
			return true;

		std::string viewport_name = viewportIndeces[camera_index];

		if (mThreads.find(viewport_name) != mThreads.end())
			if(mThreads[viewport_name].joinable())
			mThreads[viewport_name].join(); //wait on thread to finish

		//if (mViewports.find(viewport_name) != mViewports.end())
		mThreads[viewport_name] = std::thread(RunSingleCamera, pCam, viewport_name);

		std::cout << "Camera " << camera_index << " example complete..." << std::endl << std::endl;
	}

	for (auto& v_thread : mThreads)
		if (v_thread.second.joinable())
			v_thread.second.join();

	//CALLBACK EVENT
	if (mCallbacks.find("RedrawWindowCallback") != mCallbacks.end())
		mCallbacks.at("RedrawWindowCallback")(std::shared_ptr<CallbackData::BlankData>());
	return true;
}