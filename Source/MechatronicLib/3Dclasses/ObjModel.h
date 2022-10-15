#pragma once
#include "IModel.h"

#include "CommonStates.h"
#include "Model.h"
#include "Effects.h"
#include "PrimitiveBatch.h"
#include "VertexTypes.h"
#include "GeometricPrimitive.h"
#include "SpriteBatch.h"
#include "Helper.h"

#include "DXUtil.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"

#include <d3d11_1.h>
#include "ReadData.h"

using namespace DirectX::SimpleMath;

//struct Index3_32d
//{
//	uint32_t a;
//	uint32_t b;
//	uint32_t c;
//};
//struct Index3_16d
//{
//	uint16_t a;
//	uint16_t b;
//	uint16_t c;
//};



class ObjModel : public IModel
{
public:
	ObjModel();
	~ObjModel();

	//Setters
	void SetPosition(const Vector3& v_position);
	void SetOrigin(const Vector3& v_origin);
	void SetScale(const Vector3& v_scale);
	void SetTint(const Color& v_color);
	void SetAlpha(const float v_alpha);
	void SetRotation(const Vector3 v_rotation);
	virtual bool SetWorldMatrix(const MAT4X4 world_matrix) override;

	//Load function

	bool Load(ID3D11Device* p_device, ID3D11DeviceContext* device_context, MAT4X4 origin_transform, std::string file_mesh);

	bool LoadOBJ(ID3D11Device* p_device, ID3D11DeviceContext* device_context, std::string file_mesh); //Call this directly to Load OBJ

	//Render functions
	void Draw(ID3D11DeviceContext* m_p_immediate_context, const DirectX::XMMATRIX& m_view, const DirectX::XMMATRIX& m_perspective);

	//DirectX::XMVECTOR GetCenter(float &v_radius)
	//{
	//	v_radius = mModel->meshes[0]->boundingSphere.Radius;
	//	return DirectX::XMLoadFloat3(&mModel->meshes[0]->boundingSphere.Center);
	//}

	DirectX::XMMATRIX   worldMatrix;
	//int vertexCnt = 0;
	//std::vector<Vector3> vertexBuffer;
	//int indexCnt = 0;
	//std::vector<Index3_32d> indexBuffer;

protected:
	Color				mTint;
	float				mAlpha;
	DirectX::XMMATRIX   originMatrix;

	std::unique_ptr<DirectX::CommonStates>                                  mStates;
	//std::unique_ptr<DirectX::Model>											mModel;
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  mBatch;
	std::unique_ptr<DirectX::BasicEffect>									mBatchEffect;
	std::unique_ptr<DirectX::IEffectFactory>								mFxFactory;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>                               mBatchInputLayout;

	//void LoadVertexAndIndexBuffers(ID3D11Device* p_device, const wchar_t* file_mesh);

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;



	unsigned short cubeIndices[36] =
	{   0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		3, 2, 5,
		3, 5, 4,

		2, 1, 6,
		2, 6, 5,

		1, 7, 6,
		1, 0, 7,

		0, 3, 4,
		0, 4, 7 };
	// Draw the cube.
	DirectX::XMFLOAT3 cubeVertices[8][2] =
	{
		{ DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f) }, // +Y (top face)
		{ DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.5f, 0.5f,  0.5f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(-0.5f, 0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 1.0f, 1.0f) },

		{ DirectX::XMFLOAT3(-0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f) }, // -Y (bottom face)
		{ DirectX::XMFLOAT3(0.5f, -0.5f,  0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) },
	};

};

