#pragma once
#include <memory>

const bool FULL_SCREEN = true;
const bool VSYNC_ENABLED = true;

#include "Direct3DContext.h"
#include "Direct3DCamera.h"
#include "Direct3DModel.h"
#include "Direct3DRenderTexture.h"
#include "Direct3DLight.h"


class Direct3DRenderer
{
public:
	Direct3DRenderer();
	Direct3DRenderer(const Direct3DRenderer&);
	~Direct3DRenderer();

	bool Initialize(std::string viewport_name, std::shared_ptr<WindowContainer> window_container);
	bool AddSecondaryViewport(std::string viewport_name, std::shared_ptr<WindowContainer> window_container);
	void Shutdown();
	bool RenderScene(std::string viewport_name, const XMMATRIX& user_camera_move, std::vector<std::shared_ptr<IModel>> model_vect);
	bool RenderToTexture(std::string viewport_name, const XMMATRIX& user_camera_move, std::vector<std::shared_ptr<IModel>> model_vect, std::string save_image_filename, int nub_90_deg_rot_cw = 0);

	//bool CreateModel(const WCHAR* texture_path, const char* mesh_path, HWND&  v_hwnd, const XMMATRIX& world_matrix);
	//bool DeleteModels();
	bool AddCamera(std::string viewport_name)
	{
		Direct3DCamera* v_camera = new Direct3DCamera();
		if (!v_camera)
		{
			return false;
		}
		mCamera.insert(std::pair<std::string, Direct3DCamera*>(viewport_name, v_camera));
	}
	bool SetupCamera(std::string viewport_name, float p_x, float p_y, float p_z, float q_0, float q_1, float q_2, float q_3, float field_of_view, float screen_aspect);
	bool SetupCamera(std::string viewport_name, float p_x, float p_y, float p_z, float tgt_x, float tgt_y, float tgt_z, float up_x, float up_y, float up_z, float field_of_view, float screen_aspect, float roll_angle, const XMMATRIX&  camera_transform);
	bool MoveCamera(std::string viewport_name, const XMMATRIX&  new_move_matrix);
	   
	ID3D11Device* GetDevice()
	{
		return mDirect3dContext->GetDevice();
	}

	ID3D11DeviceContext* GetDeviceContext()
	{
		return mDirect3dContext->GetDeviceContext();
	}

	Direct3DCamera* GetCamera(std::string viewport_name)
	{
		if (mCamera.count(viewport_name) == 0)
			return NULL;

		return mCamera.at(viewport_name);
	}

	std::map<std::string, Direct3DCamera*> mCamera;

private:
	Direct3DContext* mDirect3dContext;
	Direct3DRenderTexture* mRenderTexture;

	Direct3DLight* mLight;

	int vpWidth, vpHeight;

	bool ExportBMP(uint8_t* pixel_data, int v_width, int v_height, std::string save_image_filename)
	{
		//Create folder if it doesnt exist
		std::string v_path = save_image_filename.substr(0, save_image_filename.find_last_of("/\\"));
		//if (!std::experimental::filesystem::is_directory(v_path) || !std::experimental::filesystem::exists(v_path)) 
		//{ // Check if src folder exists
		//	std::experimental::filesystem::create_directory(v_path); // create src folder
		//}
		std::wstring ws_tmp(v_path.begin(), v_path.end());

		CreateDirectory(ws_tmp.c_str(), NULL);

		FILE *f;
		int filesize = 54 + 4 * v_width*v_height; 

		unsigned char bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
		unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 32,0 };
		unsigned char bmppad[4] = { 0,0,0, 0 };

		bmpfileheader[2] = (unsigned char)(filesize);
		bmpfileheader[3] = (unsigned char)(filesize >> 8);
		bmpfileheader[4] = (unsigned char)(filesize >> 16);
		bmpfileheader[5] = (unsigned char)(filesize >> 24);

		bmpinfoheader[4] = (unsigned char)(v_width);
		bmpinfoheader[5] = (unsigned char)(v_width >> 8);
		bmpinfoheader[6] = (unsigned char)(v_width >> 16);
		bmpinfoheader[7] = (unsigned char)(v_width >> 24);
		bmpinfoheader[8] = (unsigned char)(v_height);
		bmpinfoheader[9] = (unsigned char)(v_height >> 8);
		bmpinfoheader[10] = (unsigned char)(v_height >> 16);
		bmpinfoheader[11] = (unsigned char)(v_height >> 24);

		f = fopen((save_image_filename + ".bmp").c_str(), "wb");
		fwrite(bmpfileheader, 1, 14, f);
		fwrite(bmpinfoheader, 1, 40, f);
		for (int i = 0; i < v_height; i++)
		{
			fwrite(pixel_data + (v_width*(v_height - i - 1) * 4), 4, v_width, f);
			fwrite(bmppad, 1, (4 - (v_width * 4) % 4) % 4, f);
		}		

		fclose(f);
		return true;
	}
};