#pragma once

#include <math.h>
#include "../Containers.h"
#include <string>
#include "IServoDriver.h"
#include "3Dclasses\IModel.h"
#include "Loader.h"

class KinematicJoint
{
public:

	enum JointType
	{
		Rotation,
		Prismatic,
		Base
	};

	KinematicJoint(Returns& e_returns, Matrix4x4 joint_orientation_world, Matrix4x4 prev_joint_orientation_world, 
		std::string chain_name, LdrJoint joint_ldr,	std::shared_ptr<IModel> joint_model = nullptr, std::shared_ptr<IServoDriver> attached_servo = nullptr) :
		eReturns(e_returns),
		jointModel(joint_model),
		attachedServo(attached_servo)
	{
		jointName = joint_ldr.jointName;
		chainName = chain_name;
		dirCosMatWorldCurrent = joint_orientation_world;

		if (jointName == "Base")
		{
			jointType = JointType::Base;
			dirCosMatLocalCurrent = dirCosMatLocalOriginal = dirCosMatWorldCurrent;
		}
		else
		{
			jointType = joint_ldr.prismaticJoint ? JointType::Prismatic : JointType::Rotation;
			CreateLocal(prev_joint_orientation_world);
		}

		minValue = joint_ldr.minRange;
		maxValue = joint_ldr.maxRange;

		if (joint_model != nullptr)
			joint_model->SetWorldMatrix(dirCosMatWorldCurrent);
	}
	~KinematicJoint() {}

	bool Init()
	{
		if (attachedServo != nullptr)
		{
			if (attachedServo->portConnected)
			{
				float servo_pos = attachedServo->GetServoPosUnits();
				if (!ActuateABS(servo_pos))
					return false;
			}
		}
		return true;
	}


	bool ActuateABS(float v_value)
	{
		if (v_value < minValue)
		{
			v_value = minValue;
		}
		else if (v_value > maxValue)
		{
			v_value = maxValue;
		}

		//Move the Joint
		if (jointType == JointType::Rotation || jointType == JointType::Prismatic)
		{
			jointActuation = v_value;
			UpdateLocal();
		}
		else
		{
			eReturns.Throw(chainName + jointName, "Attempted to actuate fixed joint");
			return false;
		}
		return true;
	}

	bool ActuateREL(float v_value)
	{
		if (v_value < minValue)
		{
			v_value = minValue;
		}
		else if (v_value > maxValue)
		{
			v_value = maxValue;
		}

		//Move the Joint
		if (jointType == JointType::Rotation || jointType == JointType::Prismatic)
		{
			jointActuation += v_value;
			UpdateLocal();
		}
		else
		{
			eReturns.Throw(chainName + jointName, "Attempted to actuate fixed joint");
			return false;
		}
		return true;
	}

	bool UpdateServo(float v_value)
	{
		//Move the Servo
		if (attachedServo != nullptr)
		{
			if (attachedServo->portConnected)
			{
				if (!attachedServo->MoveServoUnitsAbs(v_value))
					return false;
			}
		}
		return true;
	}

	bool CalibrateJoint()
	{
		if (attachedServo != nullptr)
			return attachedServo->Calibrate();
		return true;
	}

	bool CutTorque()
	{
		if (attachedServo != nullptr)
			return attachedServo->CutTorque();
		return true;
	}


	Matrix4x4 dirCosMatLocalOriginal; //The orientation of joint in default position without actuation relative to last joint
	Matrix4x4 dirCosMatLocalCurrent; //The orientation of joint after actuation relative to last joint
	Matrix4x4 dirCosMatWorldCurrent; //The orientation of joint relative to world
	std::string jointName;
	JointType jointType;
	float jointActuation = 0.0F;
	float minValue;
	float maxValue;
	std::shared_ptr<IModel> jointModel;

private:
	Returns& eReturns;
	std::shared_ptr<IServoDriver> attachedServo;
	std::string chainName;

	bool CreateLocal(Matrix4x4 prev_joint_world_mat)
	{
		//Find Local Dir Cos Matrix using inverse dirCosMatWorld from previous joint
		Matrix4x4 inv_mat;
		if (!Matrix4x4::MatrixInv4x4(prev_joint_world_mat, inv_mat))
		{
			eReturns.Throw(chainName + jointName, "Could Not Create Local Matrix due to a non-invertible previous world matrix");
			return false;
		}

		dirCosMatLocalCurrent = dirCosMatLocalOriginal = inv_mat * dirCosMatWorldCurrent ;
		
		return true;
	}

	bool UpdateLocal()
	{
		//Create a ned Local Transform matrix using original transform and Z-axis transform
		Matrix4x4 motion_matrix;
		
		if (jointType == JointType::Rotation)
		{
			float cos_angle = cos(jointActuation);
			float sin_angle = sin(jointActuation);
			motion_matrix.Get(0,0) = cos_angle;
			motion_matrix.Get(0,1) = -sin_angle;
			motion_matrix.Get(1,0) = sin_angle;
			motion_matrix.Get(1,1) = cos_angle;
		}
		else if (jointType == JointType::Prismatic)
			motion_matrix.Get(2,3) = jointActuation;
		
		dirCosMatLocalCurrent = dirCosMatLocalOriginal * motion_matrix;

		return true;
	}
};