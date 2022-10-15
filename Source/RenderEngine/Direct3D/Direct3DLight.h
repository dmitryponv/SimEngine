#pragma once
#include <DirectXMath.h>

class Direct3DLight
{
public:
	Direct3DLight() {};
	~Direct3DLight() {};

	void SetDiffuseColor(float red, float green, float blue, float alpha)
	{
		m_diffuseColor = DirectX::XMFLOAT4(red, green, blue, alpha);
		return;
	}


	void SetDirection(float x, float y, float z)
	{
		m_direction = DirectX::XMFLOAT3(x, y, z);
		return;
	}


	DirectX::XMFLOAT4 GetDiffuseColor()
	{
		return m_diffuseColor;
	}


	DirectX::XMFLOAT3 GetDirection()
	{
		return m_direction;
	}

private:
	DirectX::XMFLOAT4 m_diffuseColor;
	DirectX::XMFLOAT3 m_direction;
};

