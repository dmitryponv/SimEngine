#include "Direct3DRenderer.h"

Direct3DRenderer::Direct3DRenderer()
{
	mDirect3dContext = 0;
	//mCamera = 0;
	mRenderTexture = 0;
	mLight = 0;
}


Direct3DRenderer::Direct3DRenderer(const Direct3DRenderer& other)
{
}


Direct3DRenderer::~Direct3DRenderer()
{
}

bool Direct3DRenderer::SetupCamera(std::string viewport_name, float p_x, float p_y, float p_z, float q_0, float q_1, float q_2, float q_3, float field_of_view, float screen_aspect)
{
	if (mCamera.count(viewport_name) == 0)
		AddCamera(viewport_name);
	
	mCamera.at(viewport_name)->SetDefaultMatUsingQuaternion(p_x, p_y, p_z, XMLoadFloat4(&XMFLOAT4(q_0, q_1, q_2, q_3)));
	mCamera.at(viewport_name)->SetIntrinsics(field_of_view, screen_aspect);

	return true;
}

bool Direct3DRenderer::SetupCamera(std::string viewport_name, float p_x, float p_y, float p_z, float tgt_x, float tgt_y, float tgt_z, float up_x, float up_y, float up_z, float field_of_view, float screen_aspect, float roll_angle, const XMMATRIX&  camera_transform)
{
	if (mCamera.count(viewport_name) == 0)
		AddCamera(viewport_name);
		//return false;

	mCamera.at(viewport_name)->SetDefaultMatUsingLookAt(p_x, p_y, p_z, tgt_x, tgt_y, tgt_z, up_x, up_y, up_z, camera_transform);
	mCamera.at(viewport_name)->SetIntrinsics(field_of_view, screen_aspect, roll_angle);
	return true;
}

bool Direct3DRenderer::MoveCamera(std::string viewport_name, const XMMATRIX&  new_move_matrix)
{
	if (mCamera.count(viewport_name) == 0)
		return false;
	mCamera.at(viewport_name)->MoveCamera(new_move_matrix);
	return true;
}

bool Direct3DRenderer::Initialize(std::string viewport_name, std::shared_ptr<WindowContainer> window_container)
{
	bool result;

		
	// Create the Direct3D context.
	mDirect3dContext = new Direct3DContext;
	if(!mDirect3dContext)
	{
		return false;
	}

	// Initialize the Direct3D context.
	result = mDirect3dContext->Initialize(viewport_name, window_container, VSYNC_ENABLED, FULL_SCREEN);
	if(!result)
	{
		MessageBox(window_container->viewportWindow, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	AddCamera(viewport_name);

	// Create the render to texture object.
	mRenderTexture = new Direct3DRenderTexture;
	if(!mRenderTexture)
	{
		return false;
	}
	// Initialize the render to texture object.
	result = mRenderTexture->Initialize(mDirect3dContext->GetDevice(), window_container->viewportWidth, window_container->viewportHeight);
	if(!result)
	{
		return false;
	}
	
	// Create the light object.
	mLight = new Direct3DLight;
	if (!mLight)
	{
		return false;
	}
	// Initialize the light object.
	mLight->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	mLight->SetDirection(1.0f, -1.0f, 1.0f);

	//
	//// Create the reflection shader object.
	//mReflectionShader = new Direct3DReflectionShader;
	//if(!mReflectionShader)
	//{
	//	return false;
	//}
	//
	//// Initialize the reflection shader object.
	//result = mReflectionShader->Initialize(mDirect3dContext->GetDevice(), v_hwnd);
	//if(!result)
	//{
	//	MessageBox(*v_hwnd, L"Could not initialize the reflection shader object.", L"Error", MB_OK);
	//	return false;
	//}

	vpWidth = window_container->viewportWidth;
	vpHeight = window_container->viewportHeight;

	return true;
}

bool Direct3DRenderer::AddSecondaryViewport(std::string viewport_name, std::shared_ptr<WindowContainer> window_container)
{
	// Initialize the Direct3D context.
	bool result = mDirect3dContext->CreateNewSwapChain(viewport_name, &(window_container->viewportWindow));
	if (!result)
	{
		MessageBox(window_container->viewportWindow, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}
	return true;
}


void Direct3DRenderer::Shutdown()
{
	// Release the light object.
	if (mLight)
	{
		delete mLight;
		mLight = 0;
	}
	
	// Release the render to texture object.
	if(mRenderTexture)
	{
		mRenderTexture->Shutdown();
		delete mRenderTexture;
		mRenderTexture = 0;
	}

	// Release the camera object.
	for(auto& v_camera : mCamera)
		if(v_camera.second)
		{
			delete v_camera.second;
			v_camera.second = 0;
		}

	// Release the D3D object.
	if(mDirect3dContext)
	{
		mDirect3dContext->Shutdown();
		delete mDirect3dContext;
		mDirect3dContext = 0;
	}

	return;
}

bool Direct3DRenderer::RenderToTexture(std::string viewport_name, const XMMATRIX& user_camera_move, std::vector<std::shared_ptr<IModel>> model_vect, std::string save_image_filename, int nub_90_deg_rot_cw)
{
	bool result;

	// Clear the buffers to begin the scene.
	mDirect3dContext->BeginScene(viewport_name, 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	//mCamera->Render();
	// Get the world, view, and projection matrices from the camera and d3d objects.
	// Initialize the world matrix to the identity matrix.

	if (mCamera.find(viewport_name) != mCamera.end())
	{
		auto v_camera = mCamera.at(viewport_name);

		XMMATRIX world_matrix, view_matrix, move_matrix, cam_roll, projection_matrix, reflection_matrix;
		v_camera->GetProjectionMatrix(projection_matrix);
		v_camera->GetViewMatrix(view_matrix);
		v_camera->GetMoveToPointMatrix(move_matrix);
		v_camera->GetCamRollMatrix(cam_roll);

		view_matrix = cam_roll * move_matrix * user_camera_move * view_matrix;



		// Set the render target to be the render to texture.
		mRenderTexture->SetRenderTarget(mDirect3dContext->GetDeviceContext(), mDirect3dContext->GetDepthStencilView());
		// Clear the render to texture.
		mRenderTexture->ClearRenderTarget(mDirect3dContext->GetDeviceContext(), mDirect3dContext->GetDepthStencilView(), 0.0f, 0.0f, 0.0f, 1.0f);
		// Use the camera to calculate the reflection matrix.
		//mCamera->RenderReflection(-1.5f);

		//Render all teh models
		for (auto& v_model : model_vect)
		{
			std::shared_ptr<Direct3DModel> dx_model = std::dynamic_pointer_cast<Direct3DModel>(v_model);
			dx_model->GetWorldMatrix(world_matrix);

			// Multiply the world matrix by the rotation.
			//worldMatrix = XMMatrixRotationY(rotation);

			dx_model->Render();
			//dx_model->textureShader->Render(mDirect3dContext->GetDeviceContext(), dx_model->GetIndexCount(), world_matrix, view_matrix, projection_matrix, dx_model->GetTexture());
			if (!dx_model->lightShader->Render(mDirect3dContext->GetDeviceContext(), dx_model->GetIndexCount(), world_matrix, view_matrix, projection_matrix, dx_model->GetTexture(), mLight->GetDirection(), mLight->GetDiffuseColor()))
				return false;
			//v_model->textureShader->Render(mDirect3dContext->GetDeviceContext(), v_model->mModel->GetIndexCount(), world_matrix, view_matrix, projection_matrix, v_model->mModel->GetTexture(), XMFLOAT3(1.0F, 0.0F, 0.0F), XMFLOAT4(1.0F,1.0F,1.0F,1.0F));
		}

		DirectX::ScratchImage v_temp_image;
		ID3D11Resource* v_resource;
		mRenderTexture->GetShaderResourceView()->GetResource(&v_resource);
		HRESULT v_result = DirectX::CaptureTexture(mDirect3dContext->GetDevice(), mDirect3dContext->GetDeviceContext(), v_resource, v_temp_image);

		DirectX::ScratchImage v_rot_temp_image;
		v_result = DirectX::FlipRotate(v_temp_image.GetImages(), v_temp_image.GetImageCount(), v_temp_image.GetMetadata(), TEX_FR_ROTATE180, v_rot_temp_image);
		if (FAILED(v_result))
			return false;

		ExportBMP(v_rot_temp_image.GetPixels(), v_rot_temp_image.GetImages()->width, v_rot_temp_image.GetImages()->height, save_image_filename);
		//v_result = DirectX::SaveToTGAFile(*v_temp_image.GetImage(0, 0, 0), L"temp.tga");
		//v_result = DirectX::SaveToDDSFile(v_temp_image.GetImages(), v_temp_image.GetImageCount(), v_temp_image.GetMetadata(), DirectX::DDS_FLAGS_NONE, L"TEMPS.dds");
		//v_result = DirectX::SaveToTGAFile(*v_temp_image.GetImages(), L"TEMPS.tga");


		// Get the camera reflection view matrix instead of the normal view matrix.
		//reflectionViewMatrix = mCamera->GetReflectionViewMatrix();


		// Initialize the world matrix to the identity matrix.
		//worldMatrix = XMMatrixIdentity();

		//TODO: move matrix multiplication to a separate class
		// Setup the projection matrix.
		//const float screenAspect = (float)vpWidth / (float)vpHeight;
		// Create the projection matrix for 3D rendering.
		//projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, 0.1F, 1000.0F);

		// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
		//mModel->Render(mDirect3dContext->GetDeviceContext());

		// Render the model using the texture shader and the reflection view matrix.
		//mTextureShader->Render(mDirect3dContext->GetDeviceContext(), mModel->GetIndexCount(), worldMatrix, reflectionViewMatrix, 
		//						projectionMatrix, mModel->GetTexture());

		// Reset the render target back to the original back buffer and not the render to texture anymore.
		mDirect3dContext->SetBackBufferRenderTarget(viewport_name);
	}

	return true;
}

bool Direct3DRenderer::RenderScene(std::string viewport_name, const XMMATRIX& user_camera_move, std::vector<std::shared_ptr<IModel>> model_vect)
{

	// Clear the buffers to begin the scene.
	mDirect3dContext->BeginScene(viewport_name, 0.0f, 0.0f, 0.0f, 1.0f);
	
	if (mCamera.find(viewport_name) != mCamera.end())
	{
		auto v_camera = mCamera.at(viewport_name);

		XMMATRIX world_matrix, view_matrix, move_matrix, cam_roll, projection_matrix, reflection_matrix;
		// Get the world, view, and projection matrices from the camera and d3d objects.
		v_camera->GetProjectionMatrix(projection_matrix);
		v_camera->GetViewMatrix(view_matrix);
		v_camera->GetMoveToPointMatrix(move_matrix);
		v_camera->GetCamRollMatrix(cam_roll);
		
		if (viewport_name == "MainWindow")
			view_matrix = cam_roll * move_matrix * user_camera_move * view_matrix;
		else
			view_matrix = cam_roll * move_matrix * view_matrix;

		for (auto& v_model : model_vect)
		{
			if (v_model != nullptr)
			{
				std::shared_ptr<Direct3DModel> dx_model = std::dynamic_pointer_cast<Direct3DModel>(v_model);
				dx_model->GetWorldMatrix(world_matrix);

				// Multiply the world matrix by the rotation.
				//worldMatrix = XMMatrixRotationY(rotation);

				dx_model->Render();

				if (!dx_model->lightShader->Render(mDirect3dContext->GetDeviceContext(), dx_model->GetIndexCount(), world_matrix, view_matrix, projection_matrix, dx_model->GetTexture(), mLight->GetDirection(), mLight->GetDiffuseColor()))
					return false;
			}
		}
	}

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	//mModel->Render(mDirect3dContext->GetDeviceContext());
	//
	//// Render the model with the texture shader.
	//result = mTextureShader->Render(mDirect3dContext->GetDeviceContext(), mModel->GetIndexCount(), worldMatrix, viewMatrix,
	//								 projectionMatrix, mModel->GetTexture());
	//if(!result)
	//{
	//	return false;
	//}

	// Get the world matrix again and translate down for the floor model to render underneath the cube.
	//mDirect3dContext->GetWorldMatrix(worldMatrix);
	//worldMatrix = XMMatrixTranslation(0.0f, -1.5f, 0.0f);
	//
	//// Get the camera reflection view matrix.
	//reflectionMatrix = mCamera->GetReflectionViewMatrix();
	//
	//// Put the floor model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	////mFloorModel->Render(mDirect3dContext->GetDeviceContext());
	//
	//// Render the floor model using the reflection shader, reflection texture, and reflection view matrix.
	//result = mReflectionShader->Render(mDirect3dContext->GetDeviceContext(), mFloorModel->GetIndexCount(), worldMatrix, viewMatrix,
	//									projectionMatrix, mFloorModel->GetTexture(), mRenderTexture->GetShaderResourceView(), 
	//									reflectionMatrix);

	// Present the rendered scene to the screen.
	mDirect3dContext->EndScene(viewport_name);

	return true;
}