#include "Mechatronic.h"

bool Mechatronic::Connect(const char* com_port, uint16_t& serial_number, bool& needs_calibration, bool& needs_serial_number)
{
	if (servoControl != nullptr)
		if (!servoControl->Connect(com_port, serial_number, needs_calibration, needs_serial_number))
			return false;
		else
			servosConnected = true;
	
	return mechanismCtrl.Init();
}

bool Mechatronic::Disconnect()
{
	servosConnected = false;
	if (servoControl != nullptr)
		return servoControl->Disconnect();
	else
		return false;
}

bool Mechatronic::CutTorque()
{
	return true;
}

bool Mechatronic::LoadMechanisms(LdrMechanisms mechanisms_ldr)
{
	return mechanismCtrl.LoadMechanisms(mechanisms_ldr, servoControl);
}


//bool Mechatronic::AddSeparateActuator(std::string chain_name, int servo_id, std::string joint_name, float steps_per_unit, float min_value, float max_value)
//{
//	//TODO: Needs work
//	return servoControl.CreateServo(joint_name, servo_id, steps_per_unit);
//}

bool Mechatronic::GetEndMatrix(std::string mechanism_name, std::string chain_name,  Matrix4x4& dir_cos_mat_joint)
{
	return mechanismCtrl.GetEndMatrix(mechanism_name, chain_name, dir_cos_mat_joint);
}


std::vector<std::shared_ptr<IModel>> Mechatronic::GetModels()
{
	std::vector<std::shared_ptr<IModel>> return_models = mechanismCtrl.GetModels();

	return return_models;
}

bool Mechatronic::CalibrateAll()
{
	return mechanismCtrl.CalibrateAll();
}

bool Mechatronic::ZeroAll()
{
	return mechanismCtrl.ZeroAll();
}

bool Mechatronic::ActuateJoint(std::string mechanism_name, std::string chain_name, std::string joint_name, float v_value, bool & inside_bounding_region)
{
	return mechanismCtrl.ActuateJoint(mechanism_name, chain_name, joint_name, v_value, inside_bounding_region);
}

bool Mechatronic::GetJointPosition(std::string mechanism_name, std::string chain_name, std::string joint_name, float& v_value)
{
	return mechanismCtrl.GetJointPosition(mechanism_name, chain_name, joint_name, v_value);
}

bool Mechatronic::ActuateEE(std::string mechanism_name, std::string chain_name, Matrix4x4 ee_world_mat, bool slv_iterative, bool solve)
{
	return mechanismCtrl.ActuateEE(mechanism_name, chain_name, ee_world_mat, slv_iterative, solve);
}

//Simplified functions, no kinematics involved, do not use in combination with Kinematic Functions.
bool Mechatronic::AddServo(std::string chain_name, LdrJoint joint_ldr)
{
	std::shared_ptr<IServoDriver> attached_servo = nullptr;
	if (joint_ldr.servoId > 0 && joint_ldr.servoId < 251 && servoControl != nullptr)
	{
		std::string servo_name = chain_name + joint_ldr.jointName;
		if (!servoControl->CreateServo(servo_name, joint_ldr))
			return false;
	}
	else
		return false;
	return true;
}

bool Mechatronic::CalibrateServo(std::string chain_name, std::string joint_name)
{
	std::shared_ptr<IServoDriver> attached_servo = nullptr;
	if (servoControl != nullptr)
	{
		if (!servoControl->GetServo(chain_name + joint_name, attached_servo))
		{
			eReturns.Throw(chain_name + joint_name, "Failed to Create Servo, servo name already exists");
			return false; //Does not create a servo if the joint_name given already exists
		}

		if (attached_servo != nullptr)
			return attached_servo->Calibrate();
	}
	return false;
}

bool Mechatronic::ActuateServo(std::string chain_name, std::string joint_name, float v_value)
{
	std::shared_ptr<IServoDriver> attached_servo = nullptr;
	if (servoControl != nullptr)
	{
		if (!servoControl->GetServo(chain_name + joint_name, attached_servo))
		{
			eReturns.Throw(chain_name + joint_name, "Failed to Create Servo, servo name already exists");
			return false; //Does not create a servo if the joint_name given already exists
		}

		if (attached_servo != nullptr)
			return attached_servo->MoveServoUnitsAbs(v_value);
	}
	return false;
}

