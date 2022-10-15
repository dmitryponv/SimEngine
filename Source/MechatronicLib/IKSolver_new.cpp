#include "IKSolver.h"

bool IKSolver::SolveJointPosition(Matrix4x4 end_effector_pos, std::vector<double>& res_joint_motion)
{
	res_joint_motion.clear();
	int num_joints = chainJoints.size();

	std::vector<Vector4> jointPositionVector(num_joints);
	std::vector<Vector4> jointDirectionVector(num_joints);
	std::vector<Vector4> jointToEEVector(num_joints);
	std::vector<Vector4> EEMotionVector(num_joints);
	std::vector<Vector4> EERotationVector(num_joints);

	Matrix4x4 final_S = chainJoints[num_joints - 1]->dirCosMatWorldCurrent;

	Vector4 ee_relative_position(0,0,1);

	cv::Mat v_jacobian(6, num_joints, CV_64FC1);
	cv::Mat v_S(6, 1, CV_64FC1);
	cv::Mat v_T(6, 1, CV_64FC1);
	cv::Mat v_E(6, 1, CV_64FC1);

	for (int i = num_joints - 1; i >= 0; i--)
	{
		auto& v_joint = chainJoints[i];

		Matrix4x4 curr_local = v_joint->dirCosMatLocalOriginal;
		Matrix4x4 curr_world = v_joint->dirCosMatWorldCurrent;

		jointPositionVector[i] = curr_world.GetPosition();
		jointDirectionVector[i] = curr_world.GetZAxis();
		//jointToEEVector[i] = jointPositionVector[num_joints - 1] - jointPositionVector[i];
		jointToEEVector[i] = ee_relative_position - jointPositionVector[i];

		if (v_joint->jointType == KinematicJoint::JointType::Rotation)
		{
			EEMotionVector[i] = (jointDirectionVector[i].Cross(jointToEEVector[i])) / 10000.0F;
			EERotationVector[i] = jointDirectionVector[i] / 100.0F;
		}
		else if (v_joint->jointType == KinematicJoint::JointType::Prismatic)
		{
			EEMotionVector[i] = jointDirectionVector[i] / 10.0F;
			EERotationVector[i] = Vector4();
		}
		else
		{
			EEMotionVector[i] = Vector4();
			EERotationVector[i] = Vector4();
		}

		v_jacobian.at<double>(0, i) = EEMotionVector[i].x;
		v_jacobian.at<double>(1, i) = EEMotionVector[i].y;
		v_jacobian.at<double>(2, i) = EEMotionVector[i].z;
		v_jacobian.at<double>(3, i) = EERotationVector[i].x;
		v_jacobian.at<double>(4, i) = EERotationVector[i].y;
		v_jacobian.at<double>(5, i) = EERotationVector[i].z;
		
		std::cout << "v_jacobian = " << EEMotionVector[i].x << "\t\t" << EEMotionVector[i].y << "\t\t" << EEMotionVector[i].z << "\t\t\t dx/dq" << std::to_string(i) << "\tdy/dq" << std::to_string(i) << "\tdz/dq" << std::to_string(i) << std::endl;
	}

	//Vector4 v_t = end_effector_pos.GetPosition();
	Euler v_t = Matrix4x4::Decompose(end_effector_pos);
	v_T.at<double>(0, 0) = v_t.X;
	v_T.at<double>(1, 0) = v_t.Y;
	v_T.at<double>(2, 0) = v_t.Z;
	v_T.at<double>(3, 0) = v_t.aX;
	v_T.at<double>(4, 0) = v_t.aY;
	v_T.at<double>(5, 0) = v_t.aZ;
	
	//Vector4 v_s = final_S.GetPosition();
	Euler v_s = Matrix4x4::Decompose(final_S);
	v_S.at<double>(0, 0) = v_s.X;
	v_S.at<double>(1, 0) = v_s.Y;
	v_S.at<double>(2, 0) = v_s.Z;
	v_S.at<double>(3, 0) = v_s.aX;
	v_S.at<double>(4, 0) = v_s.aY;
	v_S.at<double>(5, 0) = v_s.aZ;

	v_E = v_T - v_S;
	std::cout << "v_T = " << std::endl << " " << v_T << std::endl << std::endl;
	std::cout << "v_S = " << std::endl << " " << v_S << std::endl << std::endl;
	std::cout << "v_E = " << std::endl << " " << v_E << std::endl << std::endl;

	//PrintMat("v_jacobian", v_jacobian);
	cv::Mat j_T;
	PrintMat("j_vacobian", v_jacobian);
	cv::transpose(v_jacobian, j_T);
	PrintMat("j_t", j_T);
	//cv::Mat j_T_j = j_T * v_jacobian;
	//PrintMat("j_T_j", j_T_j);
	//std::cout << "j_T_j determinant = " << std::endl << " " << cv::determinant(j_T_j) << std::endl << std::endl;
	//cv::Mat j_T_j_inv = j_T_j.inv();
	//PrintMat("j_T_j_inv", j_T_j_inv);
	//cv::Mat j_T_j_inv_j_T = j_T_j_inv * j_T;
	//PrintMat("j_T_j_inv_j_T", j_T_j_inv_j_T);
	//cv::Mat d_theta = j_T_j_inv_j_T * v_E;
	//std::cout << "d_theta = " << std::endl << " " << d_theta << std::endl << std::endl;

	cv::Mat d_theta = j_T * v_E;
	std::cout << "d_theta = " << std::endl << " " << d_theta << std::endl << std::endl;

	for (int i = 0; i < num_joints; i++)
	{
		//if(chainJoints[i]->jointType == KinematicJoint::JointType::Prismatic)
			res_joint_motion.push_back(d_theta.at<double>(i, 0));
		//else
		//	res_joint_motion.push_back(d_theta.at<double>(i, 0)/10.0F);
	}

	return true;
}




















bool IKSolver::SolveJointPosition2(Matrix4x4 end_effector_pos)
{

	int num_joints = chainJoints.size();

	//chainJoints.erase(chainJoints.begin());
	//MMULT test
	//Matrix4x4 A({ 5,2,6,1, 0,6,2,0, 3,8,1,4, 1,8,5,6 });
	//
	//Matrix4x4 B({ 7,5,8,0, 1,8,2,6, 9,4,3,8, 5,3,7,9 });
	//
	//Matrix4x4 C = A * B;
	
	double delta_prtb = 0.1F;
	
	std::vector<double> min_positions(num_joints);
	std::vector<double> max_positions(num_joints);
	std::vector<double> cur_positions(num_joints);

	std::vector<Matrix4x4> transformations_at_cur(num_joints);
	std::vector<Matrix4x4> transformations_prtb_up(num_joints);
	
	cv::Mat v_jacobian(3, num_joints, CV_64FC1);
	cv::Mat v_S(3, 1, CV_64FC1);
	cv::Mat v_T(3, 1, CV_64FC1);
	cv::Mat v_E(3, 1, CV_64FC1);

	//Calculate T matrix
	Euler v_t = Matrix4x4::Decompose(end_effector_pos);
	//v_T.at<double>(0, 0) = v_t.aX;
	//v_T.at<double>(1, 0) = v_t.aY;
	//v_T.at<double>(2, 0) = v_t.aZ;
	v_T.at<double>(0, 0) = v_t.X;
	v_T.at<double>(1, 0) = v_t.Y;
	v_T.at<double>(2, 0) = v_t.Z;

	Matrix4x4 final_S;

	for (int i  = 0; i < num_joints; i++)
	{
		auto& v_joint = chainJoints[i];

		// 1.  Save min and max (position or rotation) variable Θ for each of N joints, start with each joint in the middle position, choose Δ to be some small value
		min_positions[i] = v_joint->minValue;
		max_positions[i] = v_joint->maxValue;
		double joint_actuation = (v_joint->maxValue + v_joint->minValue) / 2;
		cur_positions[i] = joint_actuation;

		// 2.  Find transformation matrices for current value Θ for each joint (Call them #M matrix)
		Matrix4x4 curr_local = v_joint->dirCosMatLocalOriginal;
		Matrix4x4 motion_matrix_cur;
		Matrix4x4 motion_matrix_prtb_up;
		//Matrix4x4 motion_matrix_prtb_dn;
		
		if (v_joint->jointType == KinematicJoint::JointType::Rotation)
		{
			double cos_angle = cos(joint_actuation);
			double sin_angle = sin(joint_actuation);
			motion_matrix_cur.Get(0, 0) = cos_angle;
			motion_matrix_cur.Get(0, 1) = -sin_angle;
			motion_matrix_cur.Get(1, 0) = sin_angle;
			motion_matrix_cur.Get(1, 1) = cos_angle;

			cos_angle = cos(joint_actuation + delta_prtb);
			sin_angle = sin(joint_actuation + delta_prtb);
			motion_matrix_prtb_up.Get(0, 0) = cos_angle;
			motion_matrix_prtb_up.Get(0, 1) = -sin_angle;
			motion_matrix_prtb_up.Get(1, 0) = sin_angle;
			motion_matrix_prtb_up.Get(1, 1) = cos_angle;
		}
		else if (v_joint->jointType == KinematicJoint::JointType::Prismatic)
		{
			motion_matrix_cur.Get(2, 3) = joint_actuation;
			motion_matrix_prtb_up.Get(2, 3) = joint_actuation + delta_prtb;
		}

		// 3.  Find transformation matrices for Θ+Δ and Θ-Δ of each joint independently from the current position (Call them #M+ and #M- matrices)
		transformations_at_cur[i] = curr_local * motion_matrix_cur;
		transformations_prtb_up[i] = curr_local * motion_matrix_prtb_up;
		final_S = final_S * transformations_at_cur[i];
	}

	final_S.Print("Final S");

	for (int j = 0; j < num_joints; j++)
	{
		// 4.  Multiply (1M+)·(2M)·(3M)·(4M)... and (1M-)·(2M)·(3M)·(4M)...  to get F1M+ and F1M- matrices
		Matrix4x4 final_prtb_up; //F1M+

		for (int i = 0; i < num_joints; i++)
		{
			if (i == j)
			{
				final_prtb_up = final_prtb_up * transformations_prtb_up[i];
			}
			else
			{
				final_prtb_up = final_prtb_up * transformations_at_cur[i];
			}
		}

		final_prtb_up.Print("Joint ptb# " + std::to_string(j));
		
		// 5.  Convert F1M+ and F1M- to quaternion and translation vectors, combine into 1 vector [qw, qx, qy, qz, x, y, z] 
		// 6.  Subtract (F1M+)-(F1M-) vectors 
		// 7.  These 7 variables become first column of the Jacobian matrix
		// 8.  Repeat steps 4-7 for 2M+-, 3M+-, etc to complete jacobian matrix with N columns and 7 rows
		Euler _col = Matrix4x4::Decompose(final_prtb_up) - Matrix4x4::Decompose(final_S);
		//v_jacobian.at<double>(0, j) = _col.aX / (2 * delta_prtb);
		//v_jacobian.at<double>(1, j) = _col.aY / (2 * delta_prtb);
		//v_jacobian.at<double>(2, j) = _col.aZ / (2 * delta_prtb);
		v_jacobian.at<double>(0, j) = _col.X / (delta_prtb);
		v_jacobian.at<double>(1, j) = _col.Y / (delta_prtb);
		v_jacobian.at<double>(2, j) = _col.Z / (delta_prtb);

	}
	
	Euler v_s = Matrix4x4::Decompose(final_S);
	//v_S.at<double>(0, 0) = v_s.aX;
	//v_S.at<double>(1, 0) = v_s.aY;
	//v_S.at<double>(2, 0) = v_s.aZ;
	v_S.at<double>(0, 0) = v_s.X;
	v_S.at<double>(1, 0) = v_s.Y;
	v_S.at<double>(2, 0) = v_s.Z;


		//Compute Error
	// 9.  Compute (1M)·(2M)·(3M)·(4M)... convert to quaternion plus translation vector denote as 7 axis vector S
	// 10. Take end effector matrix, convert to quaterion plus translation vector denote as 7 axis vector T (only needs to be done once)
	// 11. Find E = T - S

	v_E = v_T - v_S;
	std::cout << "v_T = " << std::endl << " " << v_T << std::endl << std::endl;
	std::cout << "v_S = " << std::endl << " " << v_S << std::endl << std::endl;
	std::cout << "v_E = " << std::endl << " " << v_E << std::endl << std::endl;

	//PrintMat("v_jacobian", v_jacobian);
	cv::Mat j_T;
	std::cout << "v_jacobian = " << std::endl << " " << v_jacobian << std::endl << std::endl;
	cv::transpose(v_jacobian, j_T);
	std::cout << "j_T = " << std::endl << " " << j_T << std::endl << std::endl;
	cv::Mat j_T_j = j_T * v_jacobian;
	std::cout << "j_T_j = " << std::endl << " " << j_T_j << std::endl << std::endl;
	std::cout << "j_T_j determinant = " << std::endl << " " << cv::determinant(j_T_j) << std::endl << std::endl;
	cv::Mat j_T_j_inv = j_T_j.inv();
	std::cout << "j_T_j_inv = " << std::endl << " " << j_T_j_inv << std::endl << std::endl;
	cv::Mat j_T_j_inv_j_T = j_T_j_inv * j_T;
	std::cout << "j_T_j_inv_j_T = " << std::endl << " " << j_T_j_inv_j_T << std::endl << std::endl;
	cv::Mat d_theta = j_T_j_inv_j_T * v_E;
	std::cout << "d_theta = " << std::endl << " " << d_theta << std::endl << std::endl;
	
	//Matrix4x4 real_final_matrix;
	//kinematicChain->GetEndMatrix(real_final_matrix);
	
	//std::cout << "v_jacobian determinant = " << std::endl << " " << cv::determinant(v_jacobian) << std::endl << std::endl;
	//cv::Mat v_jacobian_inv = v_jacobian.inv();
	//std::cout << "v_jacobian_inv = " << std::endl << " " << v_jacobian_inv << std::endl << std::endl;
	//cv::Mat d_theta2 = v_jacobian_inv * v_E;
	//std::cout << "d_theta2 = " << std::endl << " " << d_theta2 << std::endl << std::endl;


	//Euler _euler(1, 2, 3, 4, 5, 6);
	//auto output = Matrix4x4::Compose(_euler);
	//_euler = Matrix4x4::Decompose(output);


		//Update Θ
	// 12. Solve equation E = J · ΔΘ for ΔΘ using Jᵀ·E = Jᵀ·J·ΔΘ  then (Jᵀ·J)¯¹·Jᵀ·E = ΔΘ
	// 13. Add ΔΘ to current Θ

	// 14. Repeat Steps 2-13 Until E is sufficiently small or T¯¹·S is close enough to identity matrix
	return true;
}

bool IKSolver::RunSolverLoop(Matrix4x4 end_effector_pos)
{
	int num_joints = chainJoints.size();


	std::vector<double> cur_positions(num_joints);
	std::vector<double> min_positions(num_joints);
	std::vector<double> max_positions(num_joints);

	std::vector<Matrix4x4> transformations_at_cur(num_joints);
	Matrix4x4 final_S;

	for (int i = 0; i < num_joints; i++)
	{
		auto& v_joint = chainJoints[i];
		// 1.  Save min and max (position or rotation) variable Θ for each of N joints, start with each joint in the middle position, choose Δ to be some small value
		min_positions[i] = v_joint->minValue;
		max_positions[i] = v_joint->maxValue;
	}

	bool going_up = true;
	int current_joint_actuated = 0; 
	double best_position = 0;
	double v_error = 100000;
	double max_error = 1000000000000;
	double v_increment = 1.0;
	double joint_actuation = 1000000000000;

	const double error_min = 1000; 


	while (v_error > error_min)
	{

		//Increment or decrement actuated joint
		if (joint_actuation >= max_positions[current_joint_actuated])
		{
			cur_positions[current_joint_actuated] = best_position;

			if (current_joint_actuated == num_joints - 1)
				going_up = false;
			else if (current_joint_actuated <= 1)
				going_up = true;
			
			if (going_up)
				current_joint_actuated++;
			else
				current_joint_actuated--;

			joint_actuation = min_positions[current_joint_actuated];
		}
		

		//Actuate a specific joint
		//double joint_actuation = (v_joint->maxValue + v_joint->minValue) / 2;
		//cur_positions[i] = joint_actuation;
		
		//int joint_actuated = 0;
		for (int i = 0; i < num_joints; i++)
		{
			auto& v_joint = chainJoints[i];

			// 1.  Save min and max (position or rotation) variable Θ for each of N joints, start with each joint in the middle position, choose Δ to be some small value


			// 2.  Find transformation matrices for current value Θ for each joint (Call them #M matrix)
			Matrix4x4 curr_local = v_joint->dirCosMatLocalOriginal;
			Matrix4x4 motion_matrix_cur;
			Matrix4x4 motion_matrix_prtb_up;
			//Matrix4x4 motion_matrix_prtb_dn;

			if (v_joint->jointType == KinematicJoint::JointType::Rotation)
			{
				double cos_angle = cos(joint_actuation);
				double sin_angle = sin(joint_actuation);
				motion_matrix_cur.Get(0, 0) = cos_angle;
				motion_matrix_cur.Get(0, 1) = -sin_angle;
				motion_matrix_cur.Get(1, 0) = sin_angle;
				motion_matrix_cur.Get(1, 1) = cos_angle;
			}
			else if (v_joint->jointType == KinematicJoint::JointType::Prismatic)
			{
				motion_matrix_cur.Get(2, 3) = joint_actuation;
			}

			// 3.  Find transformation matrices for Θ+Δ and Θ-Δ of each joint independently from the current position (Call them #M+ and #M- matrices)
			transformations_at_cur[i] = curr_local * motion_matrix_cur;
			//transformations_prtb_up[i] = curr_local * motion_matrix_prtb_up;
			final_S = final_S * transformations_at_cur[i];
			final_S.Print("Final S");
		}
		Euler v_s = Matrix4x4::Decompose(final_S);
		Euler v_t = Matrix4x4::Decompose(end_effector_pos);

		v_error = v_s.X * v_s.X + v_s.Y * v_s.Y + v_s.Z * v_s.Z;
	
		//Save position
		if (v_error < max_error)
		{
			max_error = v_error; 
			best_position = joint_actuation;
		}

		
		//Increment joint actuation
		joint_actuation += v_increment;

	}



	return true;
}
