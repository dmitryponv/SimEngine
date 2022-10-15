#pragma once
#include "IModel.h"

#include "CommonStates.h"
#include "Model.h"
#include "Effects.h"
#include "PrimitiveBatch.h"
#include "VertexTypes.h"
#include "GeometricPrimitive.h"
#include "SpriteBatch.h"

#include "DXUtil.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"

using namespace DirectX::SimpleMath;

class ShapeCluster3D : public IModel
{
public:
	
	ShapeCluster3D(std::shared_ptr<DXContainer> dx_ctr, std::string shape_type, std::vector<Vector3>& v_positions);// , const wchar_t * file_texture);
	~ShapeCluster3D();

	//Setters
	void SetOrigin(const Vector3& v_origin);
	void SetScale(const Vector3& v_scale);
	void SetTint(const Color& v_color);
	void SetAlpha(const float v_alpha);
	void SetRotation(const Vector3 v_rotation);


	//Render functions
	void Draw();

	bool LoadOBJ(std::string file_mesh); //Call this directly to Load OBJ

protected:
	ID3D11Resource*					m_pResource;

	std::shared_ptr<DXContainer> dxCtr;

	std::vector<Vector3> m_Positions = {};

	Vector3				m_Origin;
	Vector3				m_Scale;
	Color				m_Tint;
	float				m_Alpha;
	Vector3				m_Rotation;

	std::unique_ptr<DirectX::CommonStates>                                  m_States;
	std::unique_ptr<DirectX::GeometricPrimitive>							m_Shape;
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_Batch;
	std::unique_ptr<DirectX::BasicEffect>									m_BatchEffect;
	std::unique_ptr<DirectX::IEffectFactory>								m_FxFactory;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_BatchInputLayout;
};

