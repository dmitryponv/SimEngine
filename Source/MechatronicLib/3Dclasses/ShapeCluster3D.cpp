#include "ShapeCluster3D.h"
#include "DDSTextureLoader.h"


ShapeCluster3D::ShapeCluster3D(std::shared_ptr<DXContainer> dx_ctr, std::string shape_type, std::vector<Vector3>& v_positions):
	dxCtr(dx_ctr),
	m_pResource(nullptr),
	m_Alpha(0.0F),
	m_Positions(v_positions)
{
	m_Scale = Vector3(1, 1, 1);
	m_Alpha = 1.0f;
	m_Tint = DirectX::Colors::White.v;

	m_States.reset(new DirectX::CommonStates(dxCtr->m_pDevice));
	auto m_fx = new DirectX::EffectFactory(dxCtr->m_pDevice);
	m_fx->SetDirectory(L"media");
	m_FxFactory.reset(m_fx);
	m_Batch.reset(new DirectX::PrimitiveBatch<DirectX::VertexPositionColor>(dxCtr->m_pImmediateContext));

	m_BatchEffect.reset(new DirectX::BasicEffect(dxCtr->m_pDevice));
	m_BatchEffect->SetVertexColorEnabled(true);

	{
		void const* shader_byte_code;
		size_t byte_code_length;

		m_BatchEffect->GetVertexShaderBytecode(&shader_byte_code, &byte_code_length);

		HR(dxCtr->m_pDevice->CreateInputLayout(DirectX::VertexPositionColor::InputElements,
			DirectX::VertexPositionColor::InputElementCount,
			shader_byte_code, byte_code_length,
			m_BatchInputLayout.ReleaseAndGetAddressOf()));
	}
	
	if(shape_type == "cylinder")
		m_Shape = DirectX::GeometricPrimitive::CreateCylinder(dxCtr->m_pImmediateContext, 4.f, 180);
	else if (shape_type == "cube")
		m_Shape = DirectX::GeometricPrimitive::CreateCube(dxCtr->m_pImmediateContext, 4.f, 180);
	else
		m_Shape = DirectX::GeometricPrimitive::CreateSphere(dxCtr->m_pImmediateContext, 0.5f);
}

ShapeCluster3D::~ShapeCluster3D()
{
	Memory::SafeRelease(m_pResource);
}

void ShapeCluster3D::Draw()
{
	m_BatchEffect->SetView(dxCtr->m_View);
	m_BatchEffect->SetWorld(DirectX::XMMatrixIdentity());	

	//Draw Model
	//Set View Orientation
	DirectX::XMFLOAT4X4 m_orientation_transform_3D = DirectX::XMFLOAT4X4( // 0-degree Z-rotation
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	DirectX::XMMATRIX m_projection = DirectX::XMMatrixMultiply(dxCtr->m_Perspective, DirectX::XMLoadFloat4x4(&m_orientation_transform_3D));

	for(int i = 0 ; i < m_Positions.size(); i++)// i+=100)
	{
		DirectX::XMMATRIX m_local = DirectX::XMMatrixTranslationFromVector(m_Positions[i]);
		m_Shape->Draw(m_local, dxCtr->m_View, m_projection, DirectX::Colors::Red);
	}
}

void ShapeCluster3D::SetOrigin(const Vector3& v_origin)
{
	m_Origin = v_origin;
}

void ShapeCluster3D::SetScale(const Vector3& v_scale)
{
	m_Scale = v_scale;
}

void ShapeCluster3D::SetTint(const Color& v_color)
{
	m_Tint = v_color;
}

void ShapeCluster3D::SetAlpha(const float v_alpha)
{
	m_Alpha = v_alpha;
}

void ShapeCluster3D::SetRotation(const Vector3 v_rotation)
{
	m_Rotation = v_rotation;
}

bool ShapeCluster3D::LoadOBJ(std::string file_mesh)
{
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, file_mesh.c_str());

	if (!err.empty()) { // `err` may contain warning message.
						//std::cerr << err << std::endl;
		DebugOut(("Error: " + err + "\n").c_str());
	}

	if (!ret) {
		DebugOut("Bounding Region Load Error\n");
		return false;
	}

	//find max distance between vertices for grid spacing
	int min_x = 1000, min_y = 1000, min_z = 1000, max_x = -1000, max_y = -1000, max_z = -1000;
	for (int v = 0; v < attrib.vertices.size(); v+=3)
	{
		min_x = min(attrib.vertices[v + 0], min_x);
		min_y = min(attrib.vertices[v + 1], min_y);
		min_z = min(attrib.vertices[v + 2], min_z);
		max_x = max(attrib.vertices[v + 0], max_x);
		max_y = max(attrib.vertices[v + 1], max_y);
		max_z = max(attrib.vertices[v + 2], max_z);
	}
	float max_all = max(max(max_x - min_x, max_y - min_y), max_z - min_z);

	//Recreate mShape
	m_Shape = DirectX::GeometricPrimitive::CreateSphere(dxCtr->m_pImmediateContext, max_all / 40.0F);
	
	//Create shapes the faces of obj file
	int grid_size;
	float grid_spacing = max_all / 10.0F;// 2.0F;
	
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		int num_faces = shapes[s].mesh.num_face_vertices.size();
		for (size_t f = 0; f < num_faces; f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];
	
			// Loop over vertices in the face.
			//Only supporting 3-vertex faces
			if (fv != 3)
			{
				index_offset += fv;
				continue;
			}

			tinyobj::index_t idx_0 = shapes[s].mesh.indices[index_offset];
			float vx_0 = attrib.vertices[3 * idx_0.vertex_index + 0];
			float vy_0 = attrib.vertices[3 * idx_0.vertex_index + 1];
			float vz_0 = attrib.vertices[3 * idx_0.vertex_index + 2];

			tinyobj::index_t idx_1 = shapes[s].mesh.indices[index_offset+1];
			float vx_1 = attrib.vertices[3 * idx_1.vertex_index + 0];
			float vy_1 = attrib.vertices[3 * idx_1.vertex_index + 1];
			float vz_1 = attrib.vertices[3 * idx_1.vertex_index + 2];

			tinyobj::index_t idx_2 = shapes[s].mesh.indices[index_offset+2];
			float vx_2 = attrib.vertices[3 * idx_2.vertex_index + 0];
			float vy_2 = attrib.vertices[3 * idx_2.vertex_index + 1];
			float vz_2 = attrib.vertices[3 * idx_2.vertex_index + 2];

			index_offset += fv;

			grid_size = sqrt((vx_1 - vx_0)*(vx_1 - vx_0) + (vy_1 - vy_0)*(vy_1 - vy_0) + (vz_1 - vz_0)*(vz_1 - vz_0)) / grid_spacing;
			//Draw a grid of points between 3 vertices
			for (float u = 0; u <= grid_size; u++)
			{
				float vx_middle = vx_0 + (vx_1 - vx_0) / 2.0F;
				float vy_middle = vy_0 + (vy_1 - vy_0) / 2.0F;
				float vz_middle = vz_0 + (vz_1 - vz_0) / 2.0F;

				float vx_vec = (vx_2 - vx_middle) / grid_size;
				float vy_vec = (vy_2 - vy_middle) / grid_size;
				float vz_vec = (vz_2 - vz_middle) / grid_size;
				
				for (float w = u/2; w <= grid_size - u/2; w++)
				{
					float vx_a = vx_0 + (vx_1 - vx_0) * w / grid_size + vx_vec * u;
					float vy_a = vy_0 + (vy_1 - vy_0) * w / grid_size + vy_vec * u;
					float vz_a = vz_0 + (vz_1 - vz_0) * w / grid_size + vz_vec * u;
					
					m_Positions.push_back(Vector3(vx_a, vy_a, vz_a));
				}
			}
		}
	}

	return true;
}