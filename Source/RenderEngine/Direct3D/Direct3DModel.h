#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <fstream>
using namespace std;
using namespace DirectX;

#include <memory>
#include "Direct3DTexture.h"
#include "../../Includes/tiny_obj_loader.h"
#include <wrl.h>

//#include "../../Includes/DXUT/SDKmesh.h"
//#include "../../Includes/DXUT/SDKmisc.h"

#include "../../MechatronicLib/3Dclasses/IModel.h"
//#include "Direct3DTextureShader.h"
#include "Direct3DLightShader.h"

class Direct3DModel : public IModel
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

	//struct ModelType
	//{
	//	float x, y, z;
	//	float tu, tv;
	//	float nx, ny, nz;
	//};

public:
	//Direct3DModel(const XMMATRIX& world_matrix);
	Direct3DModel(ID3D11Device* p_device, ID3D11DeviceContext* p_device_context, std::string model_name);
	~Direct3DModel();

	//bool Initialize(const WCHAR*, const char*);
	void Shutdown();
	void Render();

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

	void GetWorldMatrix(XMMATRIX&);

	bool SetWorldMatrix(Matrix4x4& world_matrix)
	{
		DirectX::XMMATRIX m_world = DirectX::XMMATRIX( // 0-degree Z-rotation
			world_matrix.Get(0,0), world_matrix.Get(1,0), world_matrix.Get(2,0), world_matrix.Get(3,0),
			world_matrix.Get(0,1), world_matrix.Get(1,1), world_matrix.Get(2,1), world_matrix.Get(3,1),
			world_matrix.Get(0,2), world_matrix.Get(1,2), world_matrix.Get(2,2), world_matrix.Get(3,2),
			world_matrix.Get(0,3), world_matrix.Get(1,3), world_matrix.Get(2,3), world_matrix.Get(3,3)
		);

		//Adjust By default values
		worldMatrix = XMMatrixMultiply(originMatrix, m_world);

		return true;
	}

	//std::shared_ptr<Direct3DTextureShader> textureShader;
	std::shared_ptr<Direct3DLightShader> lightShader;

	bool Load(Matrix4x4 matrix_transformation, std::string file_mesh);

private:
	bool InitializeBuffers();
	void ShutdownBuffers();
	void RenderBuffers();

	bool LoadTexture(const WCHAR*, bool has_texture);
	void ReleaseTexture();


	bool LoadModelPlain(std::string);
	bool LoadModelOBJ(std::string, bool& has_texture);
	void ReleaseModel();

private:

	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;


	ID3D11Buffer *mVertexBuffer, *mIndexBuffer;
	int mVertexCount, mIndexCount;
	Direct3DTexture* mTexture;

	DirectX::XMMATRIX originMatrix;
	DirectX::XMMATRIX worldMatrix;

	std::vector<VertexType> pVertices;
	std::vector<unsigned long> pIndices;
};