#include "Shape3D.h"
#include "DDSTextureLoader.h"


Shape3D::Shape3D() :
	m_pResource(nullptr),
	m_pTexture(nullptr),
	m_Alpha(0.0F)
{
}

Shape3D::~Shape3D()
{
	Memory::SafeRelease(m_pTexture);
	Memory::SafeRelease(m_pResource);
}

void Shape3D::SetPosition(const Vector3& v_position)
{
	m_Position = v_position;
}

void Shape3D::SetOrigin(const Vector3& v_origin)
{
	m_Origin = v_origin;
}

void Shape3D::SetScale(const Vector3& v_scale)
{
	m_Scale = v_scale;
}

void Shape3D::SetTint(const Color& v_color)
{
	m_Tint = v_color;
}

void Shape3D::SetAlpha(const float v_alpha)
{
	m_Alpha = v_alpha;
}

void Shape3D::SetRotation(const Vector3 v_rotation)
{
	m_Rotation = v_rotation;
}

void Shape3D::Load(ID3D11Device* p_device, ID3D11DeviceContext* device_context, std::string shape_type, const Vector3& v_position, const Vector3& v_rotation, const wchar_t * file_texture)
{
	m_Position = v_position;
	m_Rotation = v_rotation;
	m_Scale = Vector3(1, 1, 1);
	m_Alpha = 1.0f;
	m_Tint = DirectX::Colors::White.v;

	m_States.reset(new DirectX::CommonStates(p_device));
	auto m_fx = new DirectX::EffectFactory(p_device);
	m_fx->SetDirectory(L"media");
	m_FxFactory.reset(m_fx);
	m_Batch.reset(new DirectX::PrimitiveBatch<DirectX::VertexPositionColor>(device_context));

	m_BatchEffect.reset(new DirectX::BasicEffect(p_device));
	m_BatchEffect->SetVertexColorEnabled(true);

	{
		void const* shader_byte_code;
		size_t byte_code_length;

		m_BatchEffect->GetVertexShaderBytecode(&shader_byte_code, &byte_code_length);

		HR(p_device->CreateInputLayout(DirectX::VertexPositionColor::InputElements,
			DirectX::VertexPositionColor::InputElementCount,
			shader_byte_code, byte_code_length,
			m_BatchInputLayout.ReleaseAndGetAddressOf()));
	}

	HR(DirectX::CreateDDSTextureFromFile(p_device, file_texture, &m_pResource, &m_pTexture));
	
	if(shape_type == "cylinder")
		m_Shape = DirectX::GeometricPrimitive::CreateCylinder(device_context, 4.f, 180);
	else if (shape_type == "cube")
		m_Shape = DirectX::GeometricPrimitive::CreateCube(device_context, 4.f, 180);
	else
		m_Shape = DirectX::GeometricPrimitive::CreateSphere(device_context, 4.f, 180);
}

void Shape3D::Draw(ID3D11DeviceContext* m_p_immediate_context, const DirectX::XMMATRIX& m_view, const DirectX::XMMATRIX& m_world, const DirectX::XMMATRIX& m_perspective)
{
	m_BatchEffect->SetView(m_view);
	m_BatchEffect->SetWorld(DirectX::XMMatrixIdentity());
	
	//Set View Orientation
	DirectX::XMFLOAT4X4 m_orientation_transform_3D = DirectX::XMFLOAT4X4( // 0-degree Z-rotation
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	DirectX::XMMATRIX m_projection = DirectX::XMMatrixMultiply(m_perspective, DirectX::XMLoadFloat4x4(&m_orientation_transform_3D));

	DirectX::XMVECTOR q_id = DirectX::XMQuaternionIdentity();
	const DirectX::XMVECTORF32 v_scale = { m_Scale.x, m_Scale.y, m_Scale.z };
	const DirectX::XMVECTORF32 v_translate = { m_Position.x, m_Position.y, m_Position.z };

	//SPINNING EFFECT
	DirectX::XMMATRIX m_local = XMMatrixMultiply(m_world, DirectX::XMMatrixTranslation(m_Origin.x, m_Origin.y, m_Origin.z));
	DirectX::XMVECTOR v_rotate = DirectX::XMQuaternionRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

	//END SPINNING EFFECT

	m_local = XMMatrixMultiply(m_local, DirectX::XMMatrixTransformation(DirectX::g_XMZero, q_id, v_scale, DirectX::g_XMZero, v_rotate, v_translate));

	// Draw procedurally generated dynamic grid
	///const DirectX::XMVECTORF32 xaxis = { 20.f, 0.f, 0.f };
	///const DirectX::XMVECTORF32 yaxis = { 0.f, 0.f, 20.f };
	///DrawGrid(xaxis, yaxis, DirectX::g_XMZero, 20, 20, DirectX::Colors::Gray);

	//Draw Model
	m_Shape->Draw(m_local, m_view, m_projection, DirectX::Colors::White, m_pTexture);
}

//void ServoApp::DrawGrid(DirectX::FXMVECTOR xAxis, DirectX::FXMVECTOR yAxis, DirectX::FXMVECTOR origin, size_t xdivs, size_t ydivs, DirectX::GXMVECTOR color)
//{
//	m_batchEffect->Apply(m_pImmediateContext);
//
//	m_pImmediateContext->IASetInputLayout(m_batchInputLayout.Get());
//
//	m_batch->Begin();
//
//	//xdivs = std::max<size_t>(1, xdivs);
//	//ydivs = std::max<size_t>(1, ydivs);
//
//	for (size_t i = 0; i <= xdivs; ++i)
//	{
//		float fPercent = float(i) / float(xdivs);
//		fPercent = (fPercent * 2.0f) - 1.0f;
//		DirectX::XMVECTOR vScale = DirectX::XMVectorScale(xAxis, fPercent);
//		vScale = DirectX::XMVectorAdd(vScale, origin);
//
//		DirectX::VertexPositionColor v1(DirectX::XMVectorSubtract(vScale, yAxis), color);
//		DirectX::VertexPositionColor v2(DirectX::XMVectorAdd(vScale, yAxis), color);
//		m_batch->DrawLine(v1, v2);
//	}
//
//	for (size_t i = 0; i <= ydivs; i++)
//	{
//		FLOAT fPercent = float(i) / float(ydivs);
//		fPercent = (fPercent * 2.0f) - 1.0f;
//		DirectX::XMVECTOR vScale = DirectX::XMVectorScale(yAxis, fPercent);
//		vScale = DirectX::XMVectorAdd(vScale, origin);
//
//		DirectX::VertexPositionColor v1(DirectX::XMVectorSubtract(vScale, xAxis), color);
//		DirectX::VertexPositionColor v2(DirectX::XMVectorAdd(vScale, xAxis), color);
//		m_batch->DrawLine(v1, v2);
//	}
//
//	m_batch->End();
//}