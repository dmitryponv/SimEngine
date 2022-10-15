#include "ObjModel.h"
#include "DDSTextureLoader.h"
#include <wrl.h>
#include "DirectXTK-master\Src\BinaryReader.h"
#include "DirectXTK-master\Src\SDKMesh.h"

ObjModel::ObjModel() :
	mAlpha(0.0F)
{
}

ObjModel::~ObjModel()
{
	//Memory::SafeRelease(m_pResource);
}

void ObjModel::SetPosition(const Vector3& v_position)
{
}

void ObjModel::SetOrigin(const Vector3& v_origin)
{
}

void ObjModel::SetScale(const Vector3& v_scale)
{
}

void ObjModel::SetTint(const Color& v_color)
{
	mTint = v_color;
}

void ObjModel::SetAlpha(const float v_alpha)
{
	mAlpha = v_alpha;
}

void ObjModel::SetRotation(const Vector3 v_rotation)
{
}

bool ObjModel::SetWorldMatrix(const MAT4X4 world_matrix)
{
	DirectX::XMMATRIX m_world = DirectX::XMMATRIX( // 0-degree Z-rotation
		world_matrix[0][0], world_matrix[1][0], world_matrix[2][0], world_matrix[3][0],
		world_matrix[0][1], world_matrix[1][1], world_matrix[2][1], world_matrix[3][1],
		world_matrix[0][2], world_matrix[1][2], world_matrix[2][2], world_matrix[3][2],
		world_matrix[0][3], world_matrix[1][3], world_matrix[2][3], world_matrix[3][3]
	);

	//Adjust By default values
	//DirectX::XMVECTOR qid = DirectX::XMQuaternionIdentity();
	//const DirectX::XMVECTORF32 scale = { m_Scale.x, m_Scale.y, m_Scale.z };
	//const DirectX::XMVECTORF32 translate = { m_Position.x, m_Position.y, m_Position.z };
	////DirectX::XMMATRIX world = XMMatrixMultiply(m_world, DirectX::XMMatrixTranslation(m_Origin.x, m_Origin.y, m_Origin.z));
	//DirectX::XMVECTOR rotate = DirectX::XMQuaternionRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	worldMatrix = XMMatrixMultiply(originMatrix, m_world);

	return true;
}

bool ObjModel::Load(ID3D11Device* p_device, ID3D11DeviceContext * device_context, MAT4X4 origin_transform, std::string file_mesh)
{
	//file_mesh = "cup.cmo";
	originMatrix = DirectX::XMMATRIX( // 0-degree Z-rotation
		origin_transform[0][0], origin_transform[1][0], origin_transform[2][0], origin_transform[3][0],
		origin_transform[0][1], origin_transform[1][1], origin_transform[2][1], origin_transform[3][1],
		origin_transform[0][2], origin_transform[1][2], origin_transform[2][2], origin_transform[3][2],
		origin_transform[0][3], origin_transform[1][3], origin_transform[2][3], origin_transform[3][3]
	);
	if (file_mesh.find(".obj") != std::string::npos)
		LoadOBJ(p_device, device_context, file_mesh);
	else
		return false;

//#ifdef _DEBUG
//	mModel->UpdateEffects([](DirectX::IEffect* effect)
//	{
//		auto lights = dynamic_cast<DirectX::IEffectLights*>(effect);
//		if (lights)
//		{
//			lights->SetLightingEnabled(true);
//			lights->SetPerPixelLighting(true);
//			lights->SetLightEnabled(0, true);
//			lights->SetLightDiffuseColor(0, DirectX::Colors::Black);
//			lights->SetLightEnabled(1, true);
//			lights->SetLightEnabled(2, true);
//		}
//
//		auto fog = dynamic_cast<DirectX::IEffectFog*>(effect);
//		if (fog)
//		{
//			fog->SetFogEnabled(true);
//			fog->SetFogColor(DirectX::Colors::CornflowerBlue);
//			fog->SetFogStart(30.f);
//			fog->SetFogEnd(200.f);
//		}
//	});
//#endif

	return true;
}

bool ObjModel::LoadOBJ(ID3D11Device * p_device, ID3D11DeviceContext * device_context, std::string file_mesh)
{
	std::string err;

	//char mb_str[256];
	//std::wcstombs(mb_str, file_mesh, 256);

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file_mesh.c_str());

	if (!err.empty()) { // `err` may contain warning message.
						//std::cerr << err << std::endl;
		DebugOut(("Error: " + err + "\n").c_str());
	}

	if (!ret) {
		DebugOut("Bounding Region Load Error\n");
		return false;
	}


//TESTING
	//mAlpha = 1.0f;
	//mTint = DirectX::Colors::White.v;

	//mStates.reset(new DirectX::CommonStates(p_device));
	// Create DGSL Effect
	//ID3DBlob* PS_Buffer;
	//D3DCompileFromFile(L"PixelSHader.hlsl", 0, 0, "main", "ps_5_0", 0, 0, &PS_Buffer, 0);
	//auto blob = DX::ReadData(L"MyDGSLShader.cso");
	//HR(device->CreatePixelShader(&blob.front(), blob.size(),nullptr, m_pixelShader.ReleaseAndGetAddressOf()));

	///auto fx = new DirectX::DGSLEffectFactory(p_device);
	///fx->SetDirectory(L"media");
	///mFxFactory.reset(fx);
	///mBatch.reset(new DirectX::PrimitiveBatch<DirectX::VertexPositionColor>(device_context));
	///
	///mBatchEffect.reset(new DirectX::BasicEffect(p_device));
	///mBatchEffect->SetVertexColorEnabled(true);
	///{
	///	void const* shaderByteCode;
	///	size_t byteCodeLength;
	///
	///	mBatchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
	///
	///	HR(p_device->CreateInputLayout(DirectX::VertexPositionColor::InputElements,
	///		DirectX::VertexPositionColor::InputElementCount,
	///		shaderByteCode, byteCodeLength,
	///		mBatchInputLayout.ReleaseAndGetAddressOf()));
	///}

	const D3D11_INPUT_ELEMENT_DESC basicVertexLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	//
	//void const* shaderByteCode;
	//size_t byteCodeLength;
	//DirectX::IEffectFactory::EffectInfo info;
	//auto effect = mFxFactory->CreateEffect(info, device_context);
	//effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
	//
	//p_device->CreateInputLayout(
	//	basicVertexLayoutDesc,
	//	ARRAYSIZE(basicVertexLayoutDesc),
	//	shaderByteCode,
	//	byteCodeLength,
	//	&inputLayout);

	///CREATE SHADERS
	auto vertexShaderBytecode = DX::ReadData(L"media/SimpleVertexShader.cso");
	auto pixelShaderBytecode = DX::ReadData(L"media/SimplePixelShader.cso");


	//auto createVSTask = loadVSTask.then([this](const std::vector<byte>& vertexShaderBytecode) {
		Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;

		DirectX::ThrowIfFailed(
			p_device->CreateVertexShader(
				&vertexShaderBytecode[0],
				vertexShaderBytecode.size(),
				nullptr,
				&vertexShader
			)
		);

		// Create an input layout that matches the layout defined in the vertex shader code.
		// For this lesson, this is simply a DirectX::XMFLOAT3 vector defining the vertex position, and
		// a DirectX::XMFLOAT3 vector defining the vertex color.

		Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
		DirectX::ThrowIfFailed(
			p_device->CreateInputLayout(
				basicVertexLayoutDesc, 
				ARRAYSIZE(basicVertexLayoutDesc),
				&vertexShaderBytecode[0],
				vertexShaderBytecode.size(),
				&inputLayout
			)
		);
	//});


	// Load the raw pixel shader bytecode from disk and create a pixel shader with it.
	//auto createPSTask = loadPSTask.then([this](const std::vector<byte>& pixelShaderBytecode) {
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
		DirectX::ThrowIfFailed(
			p_device->CreatePixelShader(
				&pixelShaderBytecode[0],
				pixelShaderBytecode.size(),
				nullptr,
				&pixelShader
			)
		);

	///END SHADER


	D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
	vertexBufferDesc.ByteWidth = sizeof(cubeVertices) * ARRAYSIZE(cubeVertices);
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;
	vertexBufferData.pSysMem = cubeVertices;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	p_device->CreateBuffer(
		&vertexBufferDesc,
		&vertexBufferData,
		&vertexBuffer);

	D3D11_BUFFER_DESC indexBufferDesc = { 0 };
	indexBufferDesc.ByteWidth = sizeof(cubeIndices) * ARRAYSIZE(cubeIndices);
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;
	indexBufferData.pSysMem = cubeIndices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;

	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	p_device->CreateBuffer(
		&indexBufferDesc,
		&indexBufferData,
		&indexBuffer);


	struct ConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	ConstantBuffer m_constantBufferData;

	// ...

	// Create a constant buffer for passing model, view, and projection matrices
	// to the vertex shader.  This allows us to rotate the cube and apply
	// a perspective projection to it.

	D3D11_BUFFER_DESC constantBufferDesc = { 0 };
	constantBufferDesc.ByteWidth = sizeof(m_constantBufferData);
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.MiscFlags = 0;
	constantBufferDesc.StructureByteStride = 0;
	p_device->CreateBuffer(
		&constantBufferDesc,
		nullptr,
		&m_constantBuffer
	);

	m_constantBufferData.model = DirectX::XMFLOAT4X4( // Identity matrix, since you are not animating the object
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	// Specify the view (camera) transform corresponding to a camera position of
	// X = 0, Y = 1, Z = 2.  

	m_constantBufferData.view = DirectX::XMFLOAT4X4(
		-1.00000000f, 0.00000000f, 0.00000000f, 0.00000000f,
		0.00000000f, 0.89442718f, 0.44721359f, 0.00000000f,
		0.00000000f, 0.44721359f, -0.89442718f, -2.23606800f,
		0.00000000f, 0.00000000f, 0.00000000f, 1.00000000f);


	// Finally, update the constant buffer perspective projection parameters
	// to account for the size of the application window.  In this sample,
	// the parameters are fixed to a 70-degree field of view, with a depth
	// range of 0.01 to 100.0.  

	float xScale = 100.42814801f;
	float yScale = 100.42814801f;
	//if (backBufferDesc.Width > backBufferDesc.Height)
	//{
	//	xScale = yScale *
	//		static_cast<float>(backBufferDesc.Height) /
	//		static_cast<float>(backBufferDesc.Width);
	//}
	//else
	//{
	//	yScale = xScale *
	//		static_cast<float>(backBufferDesc.Width) /
	//		static_cast<float>(backBufferDesc.Height);
	//}
	m_constantBufferData.projection = DirectX::XMFLOAT4X4(
		xScale, 0.0f, 0.0f, 0.0f,
		0.0f, yScale, 0.0f, 0.0f,
		0.0f, 0.0f, -1.0f, -0.01f,
		0.0f, 0.0f, -1.0f, 0.0f
	);

	// Set the vertex and index buffers, and specify the way they define geometry.
	UINT stride = sizeof(cubeVertices);
	UINT offset = 0;
	device_context->IASetVertexBuffers(
		0,
		1,
		vertexBuffer.GetAddressOf(),
		&stride,
		&offset);

	device_context->IASetIndexBuffer(
		indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT,
		0);

	device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set the vertex and pixel shader stage state.
	device_context->VSSetShader(
		vertexShader.Get(),
		nullptr,
		0);

	device_context->VSSetConstantBuffers(
		0,
		1,
		m_constantBuffer.GetAddressOf());
//
	device_context->PSSetShader(pixelShader.Get(), nullptr, 0);
	
	// Present the rendered image to the window.  Because the maximum frame latency is set to 1,
	// the render loop is generally  throttled to the screen refresh rate, typically around
	// 60 Hz, by sleeping the app on Present until the screen is refreshed.

	///m_swapChain->Present(1, 0);

	return true;
}

void ObjModel::Draw(ID3D11DeviceContext* m_p_immediate_context, const DirectX::XMMATRIX& m_view, const DirectX::XMMATRIX& m_perspective)
{
	///mBatchEffect->SetView(m_view);
	///mBatchEffect->SetWorld(DirectX::XMMatrixIdentity());
	
	//Set View Orientation
	//DirectX::XMFLOAT4X4 m_orientation_transform_3D = DirectX::XMFLOAT4X4( // 0-degree Z-rotation
	//	100.0f, 0.0f, 0.0f, 0.0f,
	//	0.0f, 100.0f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 100.0f, 0.0f,
	//	0.0f, 0.0f, 0.0f, 100.0f
	//);
	//DirectX::XMMATRIX m_projection = DirectX::XMMatrixMultiply(m_perspective, DirectX::XMLoadFloat4x4(&m_orientation_transform_3D)	);

	// clear the back buffer to a deep blue
	//m_p_immediate_context->ClearRenderTargetView(backbuffer, D3DXCOLOR(0.0f, 0.2f, 0.4f, 1.0f));

	// select which vertex buffer to display
	UINT stride = sizeof(cubeVertices);
	UINT offset = 0;
	m_p_immediate_context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	// select which primtive type we are using
	m_p_immediate_context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// draw the vertex buffer to the back buffer
	m_p_immediate_context->Draw(3, 0);


	m_p_immediate_context->DrawIndexed(ARRAYSIZE(cubeIndices), 0, 0);
}
