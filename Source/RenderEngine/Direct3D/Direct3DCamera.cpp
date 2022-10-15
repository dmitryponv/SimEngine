#include "Direct3DCamera.h"
#include "DXUtil.h"

Direct3DCamera::Direct3DCamera()
{
	viewMatrixMoving = XMMatrixIdentity();
}


Direct3DCamera::Direct3DCamera(const Direct3DCamera& other)
{
}


Direct3DCamera::~Direct3DCamera()
{
}


void Direct3DCamera::SetIntrinsics(float field_of_view, float screen_aspect, float roll_angle)
{
	projectionMatrix  = XMMatrixPerspectiveFovLH(field_of_view, screen_aspect, 0.1F, 1000.0F);
	//rotatingViewMatrix = XMMatrixRotationZ(RADIANS(roll_angle));
	//projectionMatrix = XMMatrixRotationZ(RADIANS(roll_angle)) * XMMatrixPerspectiveFovLH(field_of_view, screen_aspect, 0.1F, 1000.0F);

	camRotationMatrix = XMMatrixRotationX(RADIANS(roll_angle));
	//XMFLOAT4X4 v_projection;
	//XMStoreFloat4x4(&v_projection, projectionMatrix);
	////v_projection._14 = 0.0f;
	////v_projection._24 = 0.0f;
	////v_projection._34 = 1.0f;
	//projectionMatrix = XMLoadFloat4x4(&v_projection);
}

void Direct3DCamera::SetDefaultMatUsingLookAt(float p_x, float p_y, float p_z, float tgt_x, float tgt_y, float tgt_z, float up_x, float up_y, float up_z, const XMMATRIX&  camera_transform)
{
	// Calculate the rotation in radians.
	//float radians = v_y * 0.0174532925f;
	// Setup the position of the camera in the world.
	mPosition.x = p_x;
	mPosition.y = p_y;
	mPosition.z = p_z;
	// Setup where the camera is looking.
	mLookAt.x = tgt_x;//sinf(radians) + mPosition.x;
	mLookAt.y = tgt_y;//mPosition.y;
	mLookAt.z = tgt_z;//cosf(radians) + mPosition.z;
	
	// Setup the vector that points upwards.
	mUp.x = -(up_x);//-p_x);
	mUp.y = -(up_y);//-p_y);
	mUp.z = -(up_z);//-p_z);

	DirectX::XMMATRIX orig_view = XMMatrixLookAtLH(XMLoadFloat3(&mPosition), XMLoadFloat3(&mLookAt), XMLoadFloat3(&mUp));
	Utility::PrintMatrix("Camera Look At Matrix", orig_view);
	viewMatrixDefault = camera_transform * orig_view;

	return;
}


void Direct3DCamera::SetDefaultMatUsingQuaternion(float p_x, float p_y, float p_z, FXMVECTOR v_quaternion)
{
	viewMatrixDefault = XMMatrixRotationQuaternion(v_quaternion);
	XMFLOAT4X4 f_view;
	XMStoreFloat4x4(&f_view, viewMatrixDefault);
	f_view._41 = p_x;//mPosition.x * f_view._11 + mPosition.y * f_view._21 + mPosition.x * f_view._31;
	f_view._42 = p_y;//mPosition.x * f_view._12 + mPosition.y * f_view._22 + mPosition.x * f_view._32;
	f_view._43 = p_z;//mPosition.x * f_view._13 + mPosition.y * f_view._23 + mPosition.x * f_view._33;
	viewMatrixDefault = XMLoadFloat4x4(&f_view);
	return;
}

bool Direct3DCamera::MoveCamera(const XMMATRIX & new_move_matrix)
{
	viewMatrixMoving = new_move_matrix;
	return true;
}

void Direct3DCamera::GetViewMatrix(XMMATRIX& view_matrix)
{
	//view_matrix = viewMatrixMoving * rotatingViewMatrix * viewMatrixDefault;
	//view_matrix = viewMatrixMoving * viewMatrixDefault;
	view_matrix = viewMatrixDefault;
	return;
}


void Direct3DCamera::GetMoveToPointMatrix(XMMATRIX& view_matrix)
{
	view_matrix = viewMatrixMoving;
	return;
}

void Direct3DCamera::GetCamRollMatrix(XMMATRIX& view_matrix)
{
	view_matrix = camRotationMatrix;
	return;
}

void Direct3DCamera::GetProjectionMatrix(XMMATRIX& projection_matrix)
{
	projection_matrix = projectionMatrix;
	return;
}


/*
void Direct3DCamera::RenderReflection(float height)
{
	XMFLOAT3 up, position, lookAt;
	float radians;
	
	
	// Setup the vector that points upwards.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	 Setup the position of the camera in the world.
	 For planar reflection invert the Y position of the camera.
	position.x = m_positionX;
	position.y = -m_positionY + (height * 2.0f);
	position.z = m_positionZ;
	
	// Calculate the rotation in radians.
	radians = m_rotationY * 0.0174532925f;
	
	// Setup where the camera is looking.
	lookAt.x = sinf(radians) + m_positionX;
	lookAt.y = position.y;
	lookAt.z = cosf(radians) + m_positionZ;

	// Create the view matrix from the three vectors.
	reflectionViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&mPosition), XMLoadFloat3(&mLookAt), XMLoadFloat3(&mUp));
	return;
}	*/


XMMATRIX Direct3DCamera::GetReflectionViewMatrix()
{
	return reflectionViewMatrix;
}