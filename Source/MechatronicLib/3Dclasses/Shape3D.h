#pragma once
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

class Shape3D
{
public:
	Shape3D();
	~Shape3D();

	//Setters
	void SetPosition(const Vector3& v_position);
	void SetOrigin(const Vector3& v_origin);
	void SetScale(const Vector3& v_scale);
	void SetTint(const Color& v_color);
	void SetAlpha(const float v_alpha);
	void SetRotation(const Vector3 v_rotation);

	//Load function
	void Load(ID3D11Device* p_device, ID3D11DeviceContext* device_context, std::string shape_type, const Vector3& v_position, const Vector3& v_rotation, const wchar_t * file_texture);

	//Render functions
	void Draw(ID3D11DeviceContext* m_p_immediate_context, const DirectX::XMMATRIX& m_view, const DirectX::XMMATRIX& m_world, const DirectX::XMMATRIX& m_perspective);

protected:
	ID3D11Resource*					m_pResource;
	ID3D11ShaderResourceView*	    m_pTexture;
	//UINT				m_Width;
	//UINT				m_Height;
	//RECT				m_SourceRect;

	Vector3				m_Position;
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

