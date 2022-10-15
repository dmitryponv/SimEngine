#include "Model3D.h"
#include "DDSTextureLoader.h"
#include <wrl.h>
#include "DirectXTK-master\Src\BinaryReader.h"
#include "DirectXTK-master\Src\SDKMesh.h"

void Model3D::SetPosition(const Vector3& v_position)
{
}

void Model3D::SetOrigin(const Vector3& v_origin)
{
}

void Model3D::SetScale(const Vector3& v_scale)
{
}

void Model3D::SetTint(const Color& v_color)
{
	mTint = v_color;
}

void Model3D::SetAlpha(const float v_alpha)
{
	mAlpha = v_alpha;
}

void Model3D::SetRotation(const Vector3 v_rotation)
{
}

bool Model3D::SetWorldMatrix(const MAT4X4 world_matrix)
{
	DirectX::XMMATRIX m_world = DirectX::XMMATRIX( // 0-degree Z-rotation
		world_matrix[0][0], world_matrix[1][0], world_matrix[2][0], world_matrix[3][0],
		world_matrix[0][1], world_matrix[1][1], world_matrix[2][1], world_matrix[3][1],
		world_matrix[0][2], world_matrix[1][2], world_matrix[2][2], world_matrix[3][2],
		world_matrix[0][3], world_matrix[1][3], world_matrix[2][3], world_matrix[3][3]
	);

	//Adjust By default values
	worldMatrix = XMMatrixMultiply(originMatrix, m_world);

	return true;
}

bool Model3D::Load(MAT4X4 origin_transform, std::string file_mesh)
{
	originMatrix = DirectX::XMMATRIX( // 0-degree Z-rotation
		origin_transform[0][0], origin_transform[1][0], origin_transform[2][0], origin_transform[3][0],
		origin_transform[0][1], origin_transform[1][1], origin_transform[2][1], origin_transform[3][1],
		origin_transform[0][2], origin_transform[1][2], origin_transform[2][2], origin_transform[3][2],
		origin_transform[0][3], origin_transform[1][3], origin_transform[2][3], origin_transform[3][3]
	);
	if (file_mesh.find(".vbo") != std::string::npos)
		LoadVBO(dxCtr->m_pDevice, dxCtr->m_pImmediateContext, std::wstring(file_mesh.begin(), file_mesh.end()).c_str());
	else if (file_mesh.find(".sdkmesh") != std::string::npos)
		LoadSDKMESH(dxCtr->m_pDevice, dxCtr->m_pImmediateContext, std::wstring(file_mesh.begin(), file_mesh.end()).c_str());
	else if (file_mesh.find(".cmo") != std::string::npos)
		LoadCMO(dxCtr->m_pDevice, dxCtr->m_pImmediateContext, std::wstring(file_mesh.begin(), file_mesh.end()).c_str());
	else
		return false;

	return true;
}

void Model3D::LoadSDKMESH(ID3D11Device* p_device, ID3D11DeviceContext* device_context, const wchar_t* file_mesh)
{
	mAlpha = 1.0f;
	mTint = DirectX::Colors::White.v;

	mStates.reset(new DirectX::CommonStates(p_device));

	auto fx = new DirectX::EffectFactory(p_device);
	fx->SetDirectory(L"media");
	mFxFactory.reset(fx);
	mBatch.reset(new DirectX::PrimitiveBatch<DirectX::VertexPositionColor>(device_context));

	mBatchEffect.reset(new DirectX::BasicEffect(p_device));
	mBatchEffect->SetVertexColorEnabled(true);
	{
		void const* shaderByteCode;
		size_t byteCodeLength;

		mBatchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

		HR(p_device->CreateInputLayout(DirectX::VertexPositionColor::InputElements,
			DirectX::VertexPositionColor::InputElementCount,
			shaderByteCode, byteCodeLength,
			mBatchInputLayout.ReleaseAndGetAddressOf()));
	}

	LoadVertexAndIndexBuffers(p_device, file_mesh);
}

void Model3D::LoadVertexAndIndexBuffers(ID3D11Device* p_device, const wchar_t* file_mesh)
{
	///Load Vertices and Face Indices - Expanded Method
	size_t data_size = 0;
	std::unique_ptr<uint8_t[]> v_data;
	HRESULT hr = DirectX::BinaryReader::ReadEntireFile(file_mesh, v_data, &data_size);
	if (FAILED(hr))
	{
		DirectX::DebugTrace("CreateFromSDKMESH failed (%08X) loading '%ls'\n", hr, file_mesh);
		throw std::exception("CreateFromSDKMESH");
	}
	uint8_t* mesh_data = v_data.get();
	mModel = DirectX::Model::CreateFromSDKMESH(p_device, v_data.get(), data_size, *mFxFactory, false, false);
	mModel->name = file_mesh;
	auto v_header = reinterpret_cast<const DXUT::SDKMESH_HEADER*>(mesh_data);
	auto vb_array = reinterpret_cast<const DXUT::SDKMESH_VERTEX_BUFFER_HEADER*>(mesh_data + v_header->VertexStreamHeadersOffset);
	auto ib_array = reinterpret_cast<const DXUT::SDKMESH_INDEX_BUFFER_HEADER*>(mesh_data + v_header->IndexStreamHeadersOffset);

	uint64_t buffer_data_offset = v_header->HeaderSize + v_header->NonBufferDataSize;
	uint8_t* buffer_data = mesh_data + buffer_data_offset;

	//Copy Vertex Buffer
	if(v_header->NumVertexBuffers < 1)
		throw std::exception("Vertex Buffers less than 1");
	auto& vertex_header = vb_array[0];
	vertexCnt = vertex_header.NumVertices;
	auto vertex_buf_temp = reinterpret_cast<std::pair<Vector3, Vector3>*>(buffer_data + (vertex_header.DataOffset - buffer_data_offset));
	vertexBuffer.resize(vertexCnt);
	for (int i = 0; i < vertexCnt; i++)
		vertexBuffer[i] = vertex_buf_temp[i].first;

	//Copy Index Buffer
	if (v_header->NumIndexBuffers < 1)
		throw std::exception("Index Buffers less than 1");
	auto& index_header = ib_array[0];
	indexCnt = index_header.NumIndices/3;
	int ByteWidth = static_cast<UINT>(index_header.SizeBytes);
	indexBuffer.resize(indexCnt);

	//For 16 bit index count
	if (ByteWidth / indexCnt == 6)
	{
		auto index_buf_temp = reinterpret_cast<Index3_16d*>(buffer_data + (index_header.DataOffset - buffer_data_offset));
		for (int i = 0; i < indexCnt; i++)
		{
			indexBuffer[i].a = index_buf_temp[i].a;
			indexBuffer[i].b = index_buf_temp[i].b;
			indexBuffer[i].c = index_buf_temp[i].c;
		}
	}
	//For 32 bit index count
	else
	{
		auto index_buf_temp = reinterpret_cast<Index3_32d*>(buffer_data + (index_header.DataOffset - buffer_data_offset));
		for (int i = 0; i < indexCnt; i++)
			indexBuffer[i] = index_buf_temp[i];
	}
}
	
void Model3D::LoadVBO(ID3D11Device* p_device, ID3D11DeviceContext* device_context, const wchar_t* file_mesh)
{
	mAlpha = 1.0f;
	mTint = DirectX::Colors::White.v;

	mStates.reset(new DirectX::CommonStates(p_device));
	auto fx = new DirectX::EffectFactory(p_device);
	fx->SetDirectory(L"media");
	mFxFactory.reset(fx);
	mBatch.reset(new DirectX::PrimitiveBatch<DirectX::VertexPositionColor>(device_context));
	
	mBatchEffect.reset(new DirectX::BasicEffect(p_device));
	mBatchEffect->SetVertexColorEnabled(true);
	
	{
		void const* shaderByteCode;
		size_t byteCodeLength;
	
		mBatchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
	
		HR(p_device->CreateInputLayout(DirectX::VertexPositionColor::InputElements,
			DirectX::VertexPositionColor::InputElementCount,
			shaderByteCode, byteCodeLength,
			mBatchInputLayout.ReleaseAndGetAddressOf()));
	}

	mModel = DirectX::Model::CreateFromVBO(p_device, file_mesh);
}

void Model3D::LoadCMO(ID3D11Device * p_device, ID3D11DeviceContext * device_context, const wchar_t * file_mesh)
{
	mAlpha = 1.0f;
	mTint = DirectX::Colors::White.v;

	mStates.reset(new DirectX::CommonStates(p_device));

	auto fx = new DirectX::DGSLEffectFactory(p_device);
	fx->SetDirectory(L"media");
	mFxFactory.reset(fx);
	mBatch.reset(new DirectX::PrimitiveBatch<DirectX::VertexPositionColor>(device_context));

	mBatchEffect.reset(new DirectX::BasicEffect(p_device));
	mBatchEffect->SetVertexColorEnabled(true);
	{
		void const* shaderByteCode;
		size_t byteCodeLength;

		mBatchEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

		HR(p_device->CreateInputLayout(DirectX::VertexPositionColor::InputElements,
			DirectX::VertexPositionColor::InputElementCount,
			shaderByteCode, byteCodeLength,
			mBatchInputLayout.ReleaseAndGetAddressOf()));
	}

	///Original Method
	mModel = DirectX::Model::CreateFromCMO(p_device, file_mesh, *mFxFactory, false);
}

void Model3D::UpdateEffects(float fog_dist)
{	
	mModel->UpdateEffects([&fog_dist](DirectX::IEffect* effect)
	{
		auto lights = dynamic_cast<DirectX::IEffectLights*>(effect);
		if (lights)
		{
			lights->SetLightingEnabled(true);
			lights->SetPerPixelLighting(true);
			lights->SetLightEnabled(0, true);
			lights->SetLightDiffuseColor(0, DirectX::Colors::Black);
			lights->SetLightEnabled(1, true);
			lights->SetLightEnabled(2, true);
		}

		auto fog = dynamic_cast<DirectX::IEffectFog*>(effect);
		if (fog)
		{
			fog->SetFogEnabled(true);
			fog->SetFogColor(DirectX::Colors::CornflowerBlue);
			fog->SetFogStart(30.f);
			fog->SetFogEnd(fog_dist);
		}
	});	
}

void Model3D::Draw()
{
	mBatchEffect->SetView(dxCtr->m_View);
	mBatchEffect->SetWorld(DirectX::XMMatrixIdentity());
	
	//Set View Orientation
	DirectX::XMFLOAT4X4 m_orientation_transform_3D = DirectX::XMFLOAT4X4( // 0-degree Z-rotation
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	DirectX::XMMATRIX m_projection = DirectX::XMMatrixMultiply(dxCtr->m_Perspective, DirectX::XMLoadFloat4x4(&m_orientation_transform_3D)	);
	
	if (mModel != nullptr)
	{
		//Draw Model
		mModel->Draw(dxCtr->m_pImmediateContext, *mStates, worldMatrix, dxCtr->m_View, m_projection);
	}
}
