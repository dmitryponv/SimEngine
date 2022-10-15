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

using namespace DirectX::SimpleMath;

struct Index3_32d
{
	uint32_t a;
	uint32_t b;
	uint32_t c;
};
struct Index3_16d
{
	uint16_t a;
	uint16_t b;
	uint16_t c;
};


class Model3D : public IModel
{
public:
	Model3D(std::shared_ptr<DXContainer> dx_ctr, std::string model_name = "") :
		dxCtr(dx_ctr),
		modelName(model_name),
		mAlpha(0.0F)
	{
	}

	~Model3D()
	{
		//Memory::SafeRelease(m_pResource);
	}

	//Cloning functions
	Model3D(const Model3D&, std::shared_ptr<DXContainer> dx_ctr) : dxCtr(dx_ctr)
	{

	}
	virtual IModel* Clone()
	{
		return new Model3D(*this, dxCtr);
	}

	//Setters
	void SetPosition(const Vector3& v_position);
	void SetOrigin(const Vector3& v_origin);
	void SetScale(const Vector3& v_scale);
	void SetTint(const Color& v_color);
	void SetAlpha(const float v_alpha);
	void SetRotation(const Vector3 v_rotation);
	virtual bool SetWorldMatrix(const MAT4X4 world_matrix) override;

	//Load function

	virtual bool Load(MAT4X4 origin_transform, std::string file_mesh) override;
	void LoadSDKMESH(ID3D11Device* p_device, ID3D11DeviceContext* device_context, const wchar_t* file_mesh);
	void LoadVBO(ID3D11Device* p_device, ID3D11DeviceContext* device_context, const wchar_t* file_mesh);
	void LoadCMO(ID3D11Device* p_device, ID3D11DeviceContext* device_context, const wchar_t* file_mesh);


	//Render functions
	void UpdateEffects(float fog_dist) override;
	virtual void Draw() override;

	DirectX::XMVECTOR GetCenter(float &v_radius)
	{
		v_radius = mModel->meshes[0]->boundingSphere.Radius;
		return DirectX::XMLoadFloat3(&mModel->meshes[0]->boundingSphere.Center);
	}

	DirectX::XMMATRIX   worldMatrix;
	int vertexCnt = 0;
	std::vector<Vector3> vertexBuffer;
	int indexCnt = 0;
	std::vector<Index3_32d> indexBuffer;
	
protected:
	Color				mTint;
	float				mAlpha;
	DirectX::XMMATRIX   originMatrix;

	std::string modelName;

	std::unique_ptr<DirectX::CommonStates>                                  mStates;
	std::unique_ptr<DirectX::Model>											mModel;
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  mBatch;
	std::unique_ptr<DirectX::BasicEffect>									mBatchEffect;
	std::unique_ptr<DirectX::IEffectFactory>								mFxFactory;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>                               mBatchInputLayout;	

	std::shared_ptr<DXContainer> dxCtr;

	void LoadVertexAndIndexBuffers(ID3D11Device* p_device, const wchar_t* file_mesh);
};

