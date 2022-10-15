#pragma once
#include "KinematicJoint.h"
#include "../Containers.h"
#include <vector>
#include <memory>

#include "opencv2/imgproc.hpp"

// 1.  Save min and max (position or rotation) variable Θ for each of N joints, start with each joint in the middle position, choose Δ to be some small value
	//Compute Jacobian
// 2.  Find transformation matrices for current value Θ for each joint (Call them #M matrix)
// 3.  Find transformation matrices for Θ+Δ and Θ-Δ of each joint independently from the current position (Call them #M+ and #M- matrices)
// 4.  Multiply (1M+)·(2M)·(3M)·(4M)... and (1M-)·(2M)·(3M)·(4M)...  to get F1M+ and F1M- matrices
// 5.  Convert F1M+ and F1M- to quaternion and translation vectors, combine into 1 vector [qw, qx, qy, qz, x, y, z] 
// 6.  Subtract (F1M+)-(F1M-) vectors 
// 7.  These 7 variables become first column of the Jacobian matrix
// 8.  Repeat steps 4-7 for 2M+-, 3M+-, etc to complete jacobian matrix with N columns and 7 rows
	//Compute Error
// 9.  Compute (1M)·(2M)·(3M)·(4M)... convert to quaternion plus translation vector denote as 7 axis vector S
// 10. Take end effector matrix, convert to quaterion plus translation vector denote as 7 axis vector T (only needs to be done once)
// 11. Find E = T - S
	//Update Θ
// 12. Solve equation E = J · ΔΘ for ΔΘ using Jᵀ·E = Jᵀ·J·ΔΘ  then (Jᵀ·J)¯¹·Jᵀ·E = ΔΘ
// 13. Add ΔΘ to current Θ
	
// 14. Repeat Steps 2-13 Until E is sufficiently small or T¯¹·S is close enough to identity matrix



class IKSolver
{
public:
	IKSolver(Returns& e_returns, std::vector<std::shared_ptr<KinematicJoint>>& chain_joints):
		eReturns(e_returns),
		chainJoints(chain_joints)
	{
	}

	~IKSolver(){}

	bool SolveJointPosition(Matrix4x4 end_effector_pos, std::vector<double>& res_joint_motion);
	bool SolveJointPosition2(Matrix4x4 end_effector_pos);

	bool RunSolverLoop(Matrix4x4 end_effector_pos);

	bool ActuateJoints();

	void SetFinalMatrix(Matrix4x4 end_matrix)
	{
		endMatrix = end_matrix;
	}

	void PrintMat(std::string in_name, cv::Mat& in_mat)
	{
		std::cout << in_name << std::endl;
		for (int i = 0; i < in_mat.cols; i++)
		{
			for (int j = 0; j < in_mat.rows; j++)
			{
				std::cout << std::to_string(in_mat.at<double>(j, i)) << "\t";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	//bool AddJoint(std::shared_ptr<KinematicJoint>& p_joint)
	//{
	//	if (p_joint->jointType == KinematicJoint::JointType::Base)
	//	{
	//		//Add Base
	//		//chainJointWorldMatrices.push_back(MatrixMath::MatToEqn4x4(p_joint->dirCosMatLocalOriginal));
	//	}
	//	else
	//	{
	//		//Matrix4x4EQN last_world_mat = chainJointWorldMatrices[chainJointWorldMatrices.size() - 1];
	//		//Matrix4x4EQN curr_local_mat = CreateLocalEqnMat(p_joint);
	//		//Matrix4x4EQN curr_world_mat = MatrixMath::MatrixMult4x4(last_world_mat, curr_local_mat);
	//		//
	//		//chainJointWorldMatrices.push_back(curr_world_mat);
	//
	//	}
	//	return true;
	//}


private:
	Returns& eReturns;

	std::vector<std::shared_ptr<KinematicJoint>>& chainJoints;

	Matrix4x4 endMatrix;

	//std::vector<Matrix4x4EQN> chainJointWorldMatrices;

};
