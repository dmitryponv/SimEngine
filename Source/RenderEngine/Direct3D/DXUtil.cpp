#include "DXUtil.h"

void Utility::GetTextureDim(ID3D11Resource* p_res, UINT* p_width, UINT* p_height)
{
	D3D11_RESOURCE_DIMENSION m_dim;
	p_res->GetType(&m_dim);
	switch (m_dim)
	{
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		{
			auto p_texture = reinterpret_cast<ID3D11Texture2D*>(p_res);
			D3D11_TEXTURE2D_DESC m_desc;
			p_texture->GetDesc(&m_desc);
			if (p_width) 
				*p_width = m_desc.Width; //Width of texture in pixels
			if (p_height) 
				*p_height = m_desc.Height; //Height of texture in pixels
			break;
		}
		default:
		{
			if (p_width) 
				*p_width = 0; //Width of texture in pixels
			if (p_height) 
				*p_height = 0; //Height of texture in pixels
			break;
		}
	}
}

void Utility::PrintMatrix(std::string name, const DirectX::XMMATRIX& input_m)
{	
	DirectX::XMFLOAT4X4 f_mat;
	DirectX::XMStoreFloat4x4(&f_mat, input_m);

	printf("Matrix: %s \r\n \t %f \t %f \t %f \t %f \r\n \t %f \t %f \t %f \t %f \r\n \t %f \t %f \t %f \t %f \r\n \t %f \t %f \t %f \t %f \r\n\r\n", name,
		f_mat._11, f_mat._21, f_mat._31, f_mat._41,
		f_mat._12, f_mat._22, f_mat._32, f_mat._42,
		f_mat._13, f_mat._23, f_mat._33, f_mat._43,
		f_mat._14, f_mat._24, f_mat._34, f_mat._44);
}
