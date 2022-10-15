#pragma once
#include "KinematicJoint.h"
#include "../Containers.h"
#include <vector>
#include <memory>

#define VARNAMES std::vector<Var>
struct Var
{
	bool Rotational;
	std::string Name;
	float Min;
	float Max;
	bool Current = false;
};

class IKSolver
{
public:
	IKSolver(Returns& e_returns, std::vector<std::shared_ptr<KinematicJoint>>& chain_joints): 
		eReturns(e_returns),
		chainJoints(chain_joints)
	{}

	~IKSolver(){}

	bool AddJoint(std::shared_ptr<KinematicJoint>& p_joint)
	{
		if (p_joint->jointType == KinematicJoint::JointType::Base)
		{
			//Add Base
			chainJointWorldMatrices.push_back(Matrix4x4::MatToEqn4x4(p_joint->dirCosMatLocalOriginal));
		}
		else
		{
			MAT4X4EQN last_world_mat = chainJointWorldMatrices[chainJointWorldMatrices.size() - 1];
			MAT4X4EQN curr_local_mat = CreateLocalEqnMat(p_joint);
			MAT4X4EQN curr_world_mat = Matrix4x4::MatrixMult4x4(last_world_mat, curr_local_mat);

			//MatrixMath::PrintMatrix(joint->dirCosMatLocalCurrent, "dcm local");
			//MatrixMath::PrintMatrix(joint->dirCosMatWorldCurrent, "dcm world");
			//PrintMatrix(curr_local_mat, "solved local");
			//PrintMatrix(curr_world_mat, "solved world");
			//PrintMatrix(last_world_mat, "solved world last");
			//DebugOut("\n\n");
			chainJointWorldMatrices.push_back(curr_world_mat);
			////Add other joints
			///MatrixMath::PrintMatrixEqn(curr_local_mat, joint->jointName);
			///MatrixMath::PrintMatrixEqn(last_world_mat, joint->jointName);
			///MatrixMath::PrintMatrixEqn(curr_world_mat, joint->jointName);


		}
		return true;
	}

	VARNAMES BuildVarVector()
	{
		VARNAMES var_names;
		for (auto& v_joint : chainJoints)
		{
			if (v_joint->jointName == "Base")
				continue; //skip base

			Var v_var;
			v_var.Name = "{" + v_joint->jointName + "}";
			if (v_joint->jointType == KinematicJoint::JointType::Rotation)
			{
				v_var.Min = v_joint->minValue / 57.2957795;
				v_var.Max = v_joint->maxValue / 57.2957795;
				v_var.Rotational = true;
			}
			else
			{
				v_var.Min = v_joint->minValue;
				v_var.Max = v_joint->maxValue;
				v_var.Rotational = false;
			}
			var_names.push_back(v_var);
		}
		return var_names;
	}

	VARVALS BuildCurrentVarValues()
	{
		VARVALS variable_values;
		for (auto& v_joint : chainJoints)
		{
			//if (chainJoints[i]->jointType == KinematicJoint::JointType::Rotation)
			//{
				variable_values.insert(std::map<std::string, float>::value_type("COS{" + v_joint->jointName + "}", cos(v_joint->jointActuation)));
				variable_values.insert(std::map<std::string, float>::value_type("SIN{" + v_joint->jointName + "}", sin(v_joint->jointActuation)));
			//}
			//else if (chainJoints[i]->jointType == KinematicJoint::JointType::Prismatic)
				variable_values.insert(std::map<std::string, float>::value_type("{" + v_joint->jointName + "}", v_joint->jointActuation));
		}
		return variable_values;
	}

	void PrintMatrix(int matrix_it, VARVALS& variable_values, std::string v_name)
	{
		if (matrix_it > chainJoints.size() - 1)
			throw std::runtime_error("Iterator larger than array");

		Matrix4x4 solved_matrix = Matrix4x4::MatFromEqn4x4(chainJointWorldMatrices[matrix_it], variable_values);
		Matrix4x4::PrintMatrix(solved_matrix, v_name);
	}

	VARVALS SolveMatrixItr(Matrix4x4 solution, VARVALS& var_vals, int matrix_it);
	VARVALS SolveMatrixAbs(Matrix4x4 solution, VARVALS& var_vals, int matrix_it);

private:
	Returns& eReturns;

	std::vector<std::shared_ptr<KinematicJoint>>& chainJoints;

	std::vector<MAT4X4EQN> chainJointWorldMatrices;

	MAT4X4EQN CreateLocalEqnMat(std::shared_ptr<KinematicJoint>& p_joint)
	{
		MAT4X4EQN eqn_local_orig_matrix = Matrix4x4::MatToEqn4x4(p_joint->dirCosMatLocalOriginal);

		std::string var_name = p_joint->jointName;
		std::string cos_angle = "COS{" + var_name + "}";
		std::string sin_angle = "SIN{" + var_name + "}";
		std::string v_displacement = "{" + var_name + "}";

		MAT4X4EQN eqn_motion_matrix = IDENTITYEQN;

		if (p_joint->jointType == KinematicJoint::JointType::Rotation)
		{
			eqn_motion_matrix[0][0] = Equation(cos_angle);
			eqn_motion_matrix[0][1] = Equation("-" + sin_angle);

			eqn_motion_matrix[1][0] = Equation(sin_angle);
			eqn_motion_matrix[1][1] = Equation(cos_angle);
		}
		else if (p_joint->jointType == KinematicJoint::JointType::Prismatic)
		{
			eqn_motion_matrix[2][3] = Equation(v_displacement);
		}

		MAT4X4EQN output_mat = Matrix4x4::MatrixMult4x4(eqn_local_orig_matrix, eqn_motion_matrix);
		
		return output_mat;
	}	
	bool CheckDOF(VARNAMES var_names);
	bool SolveRotationItr(Matrix4x4 m_solutions, VARVALS& var_vals, VARNAMES var_names, int matrix_it);
	bool SolveTranslationItr(Matrix4x4 m_solutions, VARVALS& var_vals, VARNAMES var_names, int matrix_it);
	bool SolveRotTransItr(Matrix4x4 m_solutions, VARVALS& var_vals, VARNAMES var_names, int matrix_it);
};

