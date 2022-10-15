#pragma once
#include <DirectXMath.h>

using namespace DirectX;
#define RADIANS(x) (x*0.0174533F)

class Direct3DCamera
{
public:
	Direct3DCamera();
	Direct3DCamera(const Direct3DCamera&);
	~Direct3DCamera();

	void SetIntrinsics(float field_of_view, float screen_aspect, float roll_angle = 0.0F);
	void SetDefaultMatUsingLookAt(float p_x, float p_y, float p_z, float tgt_x, float tgt_y, float tgt_z, float up_x, float up_y, float up_z, const XMMATRIX&  camera_transform);
	void SetDefaultMatUsingQuaternion(float p_x, float p_y, float p_z, FXMVECTOR v_quaternion); //Set Rotation using a quaternion

	bool MoveCamera(const XMMATRIX&  new_move_matrix);

	void GetViewMatrix(XMMATRIX&);
	void GetMoveToPointMatrix(XMMATRIX&);
	void GetCamRollMatrix(XMMATRIX&);
	void GetProjectionMatrix(XMMATRIX&);

	//void RenderReflection(float);
	XMMATRIX GetReflectionViewMatrix();

private:
	XMMATRIX viewMatrixDefault;
	XMMATRIX viewMatrixMoving;
	XMMATRIX camRotationMatrix;

	XMMATRIX reflectionViewMatrix;
	
	XMMATRIX projectionMatrix;
	
	XMFLOAT3 mUp;
	XMFLOAT3 mPosition;
	XMFLOAT3 mLookAt;
};