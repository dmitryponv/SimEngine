#include "Direct3DModel.h"

//#define MIRRORX
#define SOLIDWORKS

Direct3DModel::Direct3DModel(ID3D11Device* p_device, ID3D11DeviceContext* p_device_context, std::string model_name)
{
	pDevice = p_device;
	pDeviceContext = p_device_context;
	modelName = model_name;
	//textureShader = std::make_shared<Direct3DTextureShader>();
	lightShader = std::make_shared<Direct3DLightShader>();
}


Direct3DModel::~Direct3DModel()
{
	Shutdown();
}




void Direct3DModel::Shutdown()
{
	// Release the model texture.
	ReleaseTexture();

	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	// Release the model data.
	ReleaseModel();

	return;
}


void Direct3DModel::Render()
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers();

	return;
}


int Direct3DModel::GetIndexCount()
{
	return mIndexCount;
}


ID3D11ShaderResourceView* Direct3DModel::GetTexture()
{
	return mTexture->GetTexture();
}

void Direct3DModel::GetWorldMatrix(XMMATRIX& world_matrix)
{
	world_matrix = worldMatrix;
	return;
}



void Direct3DModel::ShutdownBuffers()
{
	// Release the index buffer.
	if(mIndexBuffer)
	{
		mIndexBuffer->Release();
		mIndexBuffer = 0;
	}

	// Release the vertex buffer.
	if(mVertexBuffer)
	{
		mVertexBuffer->Release();
		mVertexBuffer = 0;
	}

	return;
}


void Direct3DModel::RenderBuffers()
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	pDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	pDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


bool Direct3DModel::LoadTexture(const WCHAR* file_name, bool has_texture)
{
	bool result;


	// Create the texture object.
	mTexture = new Direct3DTexture;
	if(!mTexture)
	{
		return false;
	}

	// Initialize the texture object.
	if (has_texture)
		result = mTexture->Initialize(pDevice, file_name);
	
	if(!has_texture || !result)
		result = mTexture->CreateBlankTexture(pDevice);
	
	return true;
}



void Direct3DModel::ReleaseTexture()
{
	// Release the texture object.
	if(mTexture)
	{
		mTexture->Shutdown();
		delete mTexture;
		mTexture = 0;
	}

	return;
}

bool Direct3DModel::Load(Matrix4x4 matrix_transformation, std::string file_mesh)
{
	//file_mesh = "Inputs/data/cube.mdl";
	bool has_texture = false;
	if (file_mesh.find(".obj") != std::string::npos) //if it has obj extention
	{

		if (!LoadModelOBJ(file_mesh, has_texture))
			return false;

	}
	else if (file_mesh.find(".mdl") != std::string::npos) //if it has obj extention
	{
		if (!LoadModelPlain(file_mesh))
			return false;
	}

	if (!InitializeBuffers())
		return false;

	//Texture should be the same name as model with .dds extension
	std::string file_mesh_dds = file_mesh.substr(0, file_mesh.find_last_of('.')) + ".dds";
	std::wstring file_mesh_ddsw(file_mesh_dds.begin(), file_mesh_dds.end());
	// Load the texture for this model
	if (!LoadTexture(file_mesh_ddsw.c_str(), has_texture))
		return false;

	//if (!textureShader->Initialize(pDevice))
	//	return false;
	if (!lightShader->Initialize(pDevice))
		return false;


	//Set Matrices
	originMatrix = DirectX::XMMATRIX( // 0-degree Z-rotation
		matrix_transformation.Get(0,0), matrix_transformation.Get(1,0), matrix_transformation.Get(2,0), matrix_transformation.Get(3,0),
		matrix_transformation.Get(0,1), matrix_transformation.Get(1,1), matrix_transformation.Get(2,1), matrix_transformation.Get(3,1),
		matrix_transformation.Get(0,2), matrix_transformation.Get(1,2), matrix_transformation.Get(2,2), matrix_transformation.Get(3,2),
		matrix_transformation.Get(0,3), matrix_transformation.Get(1,3), matrix_transformation.Get(2,3), matrix_transformation.Get(3,3)
	);

	worldMatrix = originMatrix;

	return true;
}


bool Direct3DModel::LoadModelPlain(std::string filename)
{
	ifstream fin;
	char input;
	int i;

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	ModelType* mModel;

	// Open the model file.  If it could not open the file then exit.
	fin.open(filename);
	if(fin.fail())
	{
		return false;
	}

	// Read up to the value of vertex count.
	fin.get(input);
	while(input != ':')
	{
		fin.get(input);
	}

	// Read in the vertex count.
	fin >> mVertexCount;

	// Set the number of indices to be the same as the vertex count.
	mIndexCount = mVertexCount;

	// Create the model using the vertex count that was read in.
	mModel = new ModelType[mVertexCount];
	if(!mModel)
	{
		return false;
	}

	// Read up to the beginning of the data.
	fin.get(input);
	while(input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// Read in the vertex data.
	for(i=0; i<mVertexCount; i++)
	{
		fin >> mModel[i].x >> mModel[i].y >> mModel[i].z;
		fin >> mModel[i].tu >> mModel[i].tv;
		fin >> mModel[i].nx >> mModel[i].ny >> mModel[i].nz;
	}

	// Close the model file.
	fin.close();
	
	pVertices.resize(mVertexCount);
	pIndices.resize(mIndexCount);

	for (i = 0; i<mVertexCount; i++)
	{
		pVertices[i].position = XMFLOAT3(mModel[i].x, mModel[i].y, mModel[i].z);
		pVertices[i].texture = XMFLOAT2(mModel[i].tu, mModel[i].tv);
		pVertices[i].normal = XMFLOAT3(mModel[i].nx, mModel[i].ny, mModel[i].nz);

		pIndices[i] = i;
	}


	return true;
}

bool Direct3DModel::LoadModelOBJ(std::string object_file, bool& has_texture)
{
	IModel::LoadOBJ(object_file);
	
	int i = 0;
	for (const auto& shape : mShapes) {
		for (const auto& index : shape.mesh.indices) {
			VertexType v_vertex;

			if (mAttrib.vertices.size() > (3 * index.vertex_index + 2))
			{
				has_texture = true;
				v_vertex.position = {
#ifdef SOLIDWORKS
					mAttrib.vertices[3 * index.vertex_index + 0], //negative corection for solidworks mirroring 
					mAttrib.vertices[3 * index.vertex_index + 2],
					-mAttrib.vertices[3 * index.vertex_index + 1]
#else
					mAttrib.vertices[3 * index.vertex_index + 0], //negative corection for solidworks mirroring 
					mAttrib.vertices[3 * index.vertex_index + 1],
					mAttrib.vertices[3 * index.vertex_index + 2]
#endif
				};

				if (mAttrib.texcoords.size() > (2 * index.texcoord_index + 1))
				{
					v_vertex.texture = {
						mAttrib.texcoords[2 * index.texcoord_index + 0],
						mAttrib.texcoords[2 * index.texcoord_index + 1]
					};
				}
				else
				{
					has_texture = false;
					v_vertex.texture = { 0,0 };
				}

				if (mAttrib.normals.size() > (3 * index.normal_index + 2))
				{
					v_vertex.normal = {
#ifdef SOLIDWORKS
						mAttrib.normals[3 * index.normal_index + 0],
						mAttrib.normals[3 * index.normal_index + 1],
						mAttrib.normals[3 * index.normal_index + 2]
#else
						mAttrib.normals[3 * index.normal_index + 0],
						mAttrib.normals[3 * index.normal_index + 1],
						mAttrib.normals[3 * index.normal_index + 2]
#endif
					};
				}

				pVertices.push_back(v_vertex);
				pIndices.push_back(i++); //Auto increment indices for simplicity
			}
		}
	}


#ifdef MIRRORX
	unsigned long temp_indx;
	for (int i = 0; i < pIndices.size(); i++)
	{
		if (i % 3 == 1)
			temp_indx = pIndices[i];
		else if (i % 3 == 2)
		{
			pIndices[i - 1] = pIndices[i];
			pIndices[i] = temp_indx;
		}
	}
#endif

	mIndexCount = pIndices.size();
	mVertexCount = pVertices.size();

	return true;
}


bool Direct3DModel::InitializeBuffers()
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// Load the vertex array and index array with data.		

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = mVertexCount * sizeof(VertexType);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = pVertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &mVertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = mIndexCount * sizeof(unsigned long);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = pIndices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = pDevice->CreateBuffer(&indexBufferDesc, &indexData, &mIndexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	pVertices.clear();
	pIndices.clear();

	return true;
}


void Direct3DModel::ReleaseModel()
{
	// Release the light shader object.
	if (lightShader)
	{
		lightShader->Shutdown();
		//delete lightShader;
		lightShader = 0;
	}
	return;
}