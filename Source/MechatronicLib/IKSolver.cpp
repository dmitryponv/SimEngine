#include "IKSolver.h"

#define TRAN_INCR 0.1
#define ROT_INCR 0.0014

#define ALWAYSINDEP

VARVALS IKSolver::SolveMatrixItr(Matrix4x4 m_solutions, VARVALS& var_vals, int matrix_it)
{
	if (matrix_it > chainJoints.size() - 1)
		throw std::runtime_error("Iterator larger than array");

	//Create variable names
	VARNAMES var_names = BuildVarVector();

#ifdef ALWAYSINDEP
	SolveRotationItr(m_solutions, var_vals, var_names, matrix_it);
	SolveTranslationItr(m_solutions, var_vals, var_names, matrix_it);
#else
	if (CheckDOF(var_names)) //Solve Rot and Translation Independently if the arm has 6DOF
	{
		//Solve equations first for rotational transform
		SolveRotationItr(m_solutions, var_vals, var_names, matrix_it);

		//Last, solve translation equations
		SolveTranslationItr(m_solutions, var_vals, var_names, matrix_it);
	}
	else //Attempt to sovle together if the arm has less than 6DOF
	{
		//Solve All equations at once
		SolveRotTransItr(m_solutions, var_vals, var_names, matrix_it);
	}
#endif
	
	DebugOut("Solved IK Iteratively");
	return var_vals;
}

bool IKSolver::CheckDOF(VARNAMES var_names)
{
	int v_rot = 0, v_tran = 0;
	for (auto &v_var : var_names)
	{
		//Skip Non-rotational vars
		if (v_var.Rotational)
			v_rot++;
		else
			v_tran++;
	}
	if (v_rot >= 3 && v_tran >= 3)
		return true;
	else
		return false;
}

bool IKSolver::SolveRotationItr(Matrix4x4 m_solutions, VARVALS& var_vals, VARNAMES var_names, int matrix_it)
{
	
	//Take 6 equations from the matrix
	Equation r_eqn0 = chainJointWorldMatrices[matrix_it][0][0];
	Equation r_eqn1 = chainJointWorldMatrices[matrix_it][0][1];
	Equation r_eqn2 = chainJointWorldMatrices[matrix_it][0][2];
	Equation r_eqn3 = chainJointWorldMatrices[matrix_it][1][0];
	Equation r_eqn4 = chainJointWorldMatrices[matrix_it][1][1];
	Equation r_eqn5 = chainJointWorldMatrices[matrix_it][1][2];
	Equation r_eqn6 = chainJointWorldMatrices[matrix_it][2][0];
	Equation r_eqn7 = chainJointWorldMatrices[matrix_it][2][1];
	Equation r_eqn8 = chainJointWorldMatrices[matrix_it][2][2];

	//Take 6 solutions from the matrix
	float r_sln0 = m_solutions.Get(0,0);
	float r_sln1 = m_solutions.Get(0,1);
	float r_sln2 = m_solutions.Get(0,2);
	float r_sln3 = m_solutions.Get(1,0);
	float r_sln4 = m_solutions.Get(1,1);
	float r_sln5 = m_solutions.Get(1,2);
	float r_sln6 = m_solutions.Get(2,0);
	float r_sln7 = m_solutions.Get(2,1);
	float r_sln8 = m_solutions.Get(2,2);

	bool b_cont = true;
	for (int i = 0; i < 100, b_cont == true; i++)
	{
		b_cont = false;
		for (auto &v_var : var_names)
		{
			//Skip Non-rotational vars
			if (!v_var.Rotational)
				continue;

			//Test 3 vals, -step, 0, +step and see which brings test closer to 0
			float tmp_sln0;
			float tmp_sln1;
			float tmp_sln2;
			float tmp_sln3;
			float tmp_sln4;
			float tmp_sln5;
			float tmp_sln6;
			float tmp_sln7;
			float tmp_sln8;
			float curr_val = var_vals.at(v_var.Name);

			//Test -1 increment
			var_vals.at(v_var.Name) = curr_val - ROT_INCR;
			var_vals.at("COS" + v_var.Name) = cos(curr_val - ROT_INCR);
			var_vals.at("SIN" + v_var.Name) = sin(curr_val - ROT_INCR);
			tmp_sln0 = Matrix4x4::SolveEquation(r_eqn0, var_vals);
			tmp_sln1 = Matrix4x4::SolveEquation(r_eqn1, var_vals);
			tmp_sln2 = Matrix4x4::SolveEquation(r_eqn2, var_vals);
			tmp_sln3 = Matrix4x4::SolveEquation(r_eqn3, var_vals);
			tmp_sln4 = Matrix4x4::SolveEquation(r_eqn4, var_vals);
			tmp_sln5 = Matrix4x4::SolveEquation(r_eqn5, var_vals);
			tmp_sln6 = Matrix4x4::SolveEquation(r_eqn6, var_vals);
			tmp_sln7 = Matrix4x4::SolveEquation(r_eqn7, var_vals);
			tmp_sln8 = Matrix4x4::SolveEquation(r_eqn8, var_vals);
			float test_0 = pow((tmp_sln0 - r_sln0), 2) + pow((tmp_sln1 - r_sln1), 2) + pow((tmp_sln2 - r_sln2), 2) +
				pow((tmp_sln3 - r_sln3), 2) + pow((tmp_sln4 - r_sln4), 2) + pow((tmp_sln5 - r_sln5), 2) +
				pow((tmp_sln6 - r_sln6), 2) + pow((tmp_sln7 - r_sln7), 2) + pow((tmp_sln8 - r_sln8), 2);

			//Test current
			var_vals.at(v_var.Name) = curr_val;
			var_vals.at("COS" + v_var.Name) = cos(curr_val);
			var_vals.at("SIN" + v_var.Name) = sin(curr_val);
			tmp_sln0 = Matrix4x4::SolveEquation(r_eqn0, var_vals);
			tmp_sln1 = Matrix4x4::SolveEquation(r_eqn1, var_vals);
			tmp_sln2 = Matrix4x4::SolveEquation(r_eqn2, var_vals);
			tmp_sln3 = Matrix4x4::SolveEquation(r_eqn3, var_vals);
			tmp_sln4 = Matrix4x4::SolveEquation(r_eqn4, var_vals);
			tmp_sln5 = Matrix4x4::SolveEquation(r_eqn5, var_vals);
			tmp_sln6 = Matrix4x4::SolveEquation(r_eqn6, var_vals);
			tmp_sln7 = Matrix4x4::SolveEquation(r_eqn7, var_vals);
			tmp_sln8 = Matrix4x4::SolveEquation(r_eqn8, var_vals);
			float test_1 = pow((tmp_sln0 - r_sln0), 2) + pow((tmp_sln1 - r_sln1), 2) + pow((tmp_sln2 - r_sln2), 2) +
				pow((tmp_sln3 - r_sln3), 2) + pow((tmp_sln4 - r_sln4), 2) + pow((tmp_sln5 - r_sln5), 2) +
				pow((tmp_sln6 - r_sln6), 2) + pow((tmp_sln7 - r_sln7), 2) + pow((tmp_sln8 - r_sln8), 2);

			//Test +1 increment
			var_vals.at(v_var.Name) = curr_val + ROT_INCR; //Technically non-rotational variables will not make it into rot matrix
			var_vals.at("COS" + v_var.Name) = cos(curr_val + ROT_INCR);
			var_vals.at("SIN" + v_var.Name) = sin(curr_val + ROT_INCR);
			tmp_sln0 = Matrix4x4::SolveEquation(r_eqn0, var_vals);
			tmp_sln1 = Matrix4x4::SolveEquation(r_eqn1, var_vals);
			tmp_sln2 = Matrix4x4::SolveEquation(r_eqn2, var_vals);
			tmp_sln3 = Matrix4x4::SolveEquation(r_eqn3, var_vals);
			tmp_sln4 = Matrix4x4::SolveEquation(r_eqn4, var_vals);
			tmp_sln5 = Matrix4x4::SolveEquation(r_eqn5, var_vals);
			tmp_sln6 = Matrix4x4::SolveEquation(r_eqn6, var_vals);
			tmp_sln7 = Matrix4x4::SolveEquation(r_eqn7, var_vals);
			tmp_sln8 = Matrix4x4::SolveEquation(r_eqn8, var_vals);
			float test_2 = pow((tmp_sln0 - r_sln0), 2) + pow((tmp_sln1 - r_sln1), 2) + pow((tmp_sln2 - r_sln2), 2) +
				pow((tmp_sln3 - r_sln3), 2) + pow((tmp_sln4 - r_sln4), 2) + pow((tmp_sln5 - r_sln5), 2) +
				pow((tmp_sln6 - r_sln6), 2) + pow((tmp_sln7 - r_sln7), 2) + pow((tmp_sln8 - r_sln8), 2);

			//Save each test condition
			if (test_0 < test_1 && test_0 < test_2)
			{
				var_vals.at(v_var.Name) = curr_val - ROT_INCR;
				var_vals.at("COS" + v_var.Name) = cos(curr_val - ROT_INCR);
				var_vals.at("SIN" + v_var.Name) = sin(curr_val - ROT_INCR);
				b_cont = true;
			}
			else if (test_1 < test_0 && test_1 < test_2)
			{
				var_vals.at(v_var.Name) = curr_val;
				var_vals.at("COS" + v_var.Name) = cos(curr_val);
				var_vals.at("SIN" + v_var.Name) = sin(curr_val);
			}
			else if (test_2 < test_0 && test_2 < test_1)
			{
				//var_vals.at(v_var.Name) = curr_val + rot_increment; //Should already be at these values anyway
				//var_vals.at("COS" + v_var.Name) = cos(curr_val + rot_increment);
				//var_vals.at("SIN" + v_var.Name) = sin(curr_val + rot_increment);
				b_cont = true;
			}
		}
	} //while (b_cont); //Set Some limiting condition

	return true;
}

bool IKSolver::SolveTranslationItr(Matrix4x4 solutions, VARVALS& var_vals, VARNAMES var_names, int matrix_it)
{
	//Take 6 equations from the matrix
	Equation t_eqn0 = chainJointWorldMatrices[matrix_it][0][3];
	Equation t_eqn1 = chainJointWorldMatrices[matrix_it][1][3];
	Equation t_eqn2 = chainJointWorldMatrices[matrix_it][2][3];

	//Take 6 solutions from the matrix
	float t_sln0 = solutions.Get(0,3);
	float t_sln1 = solutions.Get(1,3);
	float t_sln2 = solutions.Get(2,3);

	bool b_cont = true;
	for(int i = 0; i < 100, b_cont == true; i++)
	{
		b_cont = false;
		for (auto &v_var : var_names)
		{
			//Skip Non-translational vars
			if (v_var.Rotational)
				continue;

			//Test 3 vals, -step, 0, +step and see which brings test closer to 0
			float tmp_sln0;
			float tmp_sln1;
			float tmp_sln2;
			float curr_val = var_vals.at(v_var.Name);

			//Test -1 increment
			var_vals.at(v_var.Name) = curr_val - TRAN_INCR;
			var_vals.at("COS" + v_var.Name) = cos(curr_val - TRAN_INCR);
			var_vals.at("SIN" + v_var.Name) = sin(curr_val - TRAN_INCR);
			tmp_sln0 = Matrix4x4::SolveEquation(t_eqn0, var_vals);
			tmp_sln1 = Matrix4x4::SolveEquation(t_eqn1, var_vals);
			tmp_sln2 = Matrix4x4::SolveEquation(t_eqn2, var_vals);
			float test_0 = pow((tmp_sln0 - t_sln0), 2) + pow((tmp_sln1 - t_sln1), 2) + pow((tmp_sln2 - t_sln2), 2);

			//Test current
			var_vals.at(v_var.Name) = curr_val;
			var_vals.at("COS" + v_var.Name) = cos(curr_val);
			var_vals.at("SIN" + v_var.Name) = sin(curr_val);
			tmp_sln0 = Matrix4x4::SolveEquation(t_eqn0, var_vals);
			tmp_sln1 = Matrix4x4::SolveEquation(t_eqn1, var_vals);
			tmp_sln2 = Matrix4x4::SolveEquation(t_eqn2, var_vals);
			float test_1 = pow((tmp_sln0 - t_sln0), 2) + pow((tmp_sln1 - t_sln1), 2) + pow((tmp_sln2 - t_sln2), 2);

			//Test +1 increment
			var_vals.at(v_var.Name) = curr_val + TRAN_INCR;
			var_vals.at("COS" + v_var.Name) = cos(curr_val + TRAN_INCR);
			var_vals.at("SIN" + v_var.Name) = sin(curr_val + TRAN_INCR);
			tmp_sln0 = Matrix4x4::SolveEquation(t_eqn0, var_vals);
			tmp_sln1 = Matrix4x4::SolveEquation(t_eqn1, var_vals);
			tmp_sln2 = Matrix4x4::SolveEquation(t_eqn2, var_vals);
			float test_2 = pow((tmp_sln0 - t_sln0), 2) + pow((tmp_sln1 - t_sln1), 2) + pow((tmp_sln2 - t_sln2), 2);

			//Save each test condition
			if (test_0 < test_1 && test_0 < test_2)
			{
				var_vals.at(v_var.Name) = curr_val - TRAN_INCR;
				var_vals.at("COS" + v_var.Name) = cos(curr_val - TRAN_INCR);
				var_vals.at("SIN" + v_var.Name) = sin(curr_val - TRAN_INCR);
				b_cont = true;
			}
			else if (test_1 < test_0 && test_1 < test_2)
			{
				var_vals.at(v_var.Name) = curr_val;
				var_vals.at("COS" + v_var.Name) = cos(curr_val);
				var_vals.at("SIN" + v_var.Name) = sin(curr_val);
			}
			else if (test_2 < test_0 && test_2 < test_1)
			{
				//var_vals.at(v_var.Name) = curr_val + TRAN_INCR; //Should already be at these values anyway
				//var_vals.at("COS" + v_var.Name) = cos(curr_val + TRAN_INCR);
				//var_vals.at("SIN" + v_var.Name) = sin(curr_val + TRAN_INCR);
				b_cont = true;
			}
		}


	}// while (b_cont); //Set Some limiting condition

	return true;
}

bool IKSolver::SolveRotTransItr(Matrix4x4 m_solutions, VARVALS& var_vals, VARNAMES var_names, int matrix_it)
{
	//Create inrement for computation (the smaller the higher accuracy)
	float v_increment;

	//Take 3 equations from the matrix
	//Equation r_eqn0 = chainJointWorldMatrices[matrix_it][0][0];
	//Equation r_eqn1 = chainJointWorldMatrices[matrix_it][0][1];
	//Equation r_eqn2 = chainJointWorldMatrices[matrix_it][0][2];
	Equation r_eqn3 = chainJointWorldMatrices[matrix_it][1][0];
	//Equation r_eqn4 = chainJointWorldMatrices[matrix_it][1][1];
	//Equation r_eqn5 = chainJointWorldMatrices[matrix_it][1][2];
	Equation r_eqn6 = chainJointWorldMatrices[matrix_it][2][0];
	Equation r_eqn7 = chainJointWorldMatrices[matrix_it][2][1];
	//Equation r_eqn8 = chainJointWorldMatrices[matrix_it][2][2];
	Equation t_eqn0 = chainJointWorldMatrices[matrix_it][0][3];
	Equation t_eqn1 = chainJointWorldMatrices[matrix_it][1][3];
	Equation t_eqn2 = chainJointWorldMatrices[matrix_it][2][3];

	//Take 3 solutions from the matrix
	//float r_sln0 = m_solutions[0][0];
	//float r_sln1 = m_solutions[0][1];
	//float r_sln2 = m_solutions[0][2];
	float r_sln3 = m_solutions.Get(1,0);
	//float r_sln4 = m_solutions[1][1];
	//float r_sln5 = m_solutions[1][2];
	float r_sln6 = m_solutions.Get(2,0);
	float r_sln7 = m_solutions.Get(2,1);
	//float r_sln8 = m_solutions[2][2];
	float t_sln0 = m_solutions.Get(0,3);
	float t_sln1 = m_solutions.Get(1,3);
	float t_sln2 = m_solutions.Get(2,3);

	bool b_cont = true;
	for (int i = 0; i < 100, b_cont == true; i++)
	{
		b_cont = false;
		for (auto &v_var : var_names)
		{
			//Skip Non-rotational vars
			if (v_var.Rotational)
				v_increment = ROT_INCR;
			else
				v_increment = TRAN_INCR;

			//Test 3 vals, -step, 0, +step and see which brings test closer to 0
			//float tmp_sln0;
			//float tmp_sln1;
			//float tmp_sln2;
			float tmp_sln3;
			//float tmp_sln4;
			//float tmp_sln5;
			float tmp_sln6;
			float tmp_sln7;
			//float tmp_sln8;
			float tmp_sln9;
			float tmp_sln10;
			float tmp_sln11;
			float curr_val = var_vals.at(v_var.Name);

			//Test -1 increment
			var_vals.at(v_var.Name) = curr_val - v_increment;
			var_vals.at("COS" + v_var.Name) = cos(curr_val - v_increment);
			var_vals.at("SIN" + v_var.Name) = sin(curr_val - v_increment);
			//tmp_sln0 = MatrixMath::SolveEquation(r_eqn0, var_vals);
			//tmp_sln1 = MatrixMath::SolveEquation(r_eqn1, var_vals);
			//tmp_sln2 = MatrixMath::SolveEquation(r_eqn2, var_vals);
			tmp_sln3 = Matrix4x4::SolveEquation(r_eqn3, var_vals);
			//tmp_sln4 = MatrixMath::SolveEquation(r_eqn4, var_vals);
			//tmp_sln5 = MatrixMath::SolveEquation(r_eqn5, var_vals);
			tmp_sln6 = Matrix4x4::SolveEquation(r_eqn6, var_vals);
			tmp_sln7 = Matrix4x4::SolveEquation(r_eqn7, var_vals);
			//tmp_sln8 = MatrixMath::SolveEquation(r_eqn8, var_vals);
			tmp_sln9 = Matrix4x4::SolveEquation(t_eqn0, var_vals);
			tmp_sln10 = Matrix4x4::SolveEquation(t_eqn1, var_vals);
			tmp_sln11 = Matrix4x4::SolveEquation(t_eqn2, var_vals);
			//float test_0 = pow((tmp_sln0 - r_sln0), 2) + pow((tmp_sln1 - r_sln1), 2) + pow((tmp_sln2 - r_sln2), 2) +
			//	pow((tmp_sln3 - r_sln3), 2) + pow((tmp_sln4 - r_sln4), 2) + pow((tmp_sln5 - r_sln5), 2) +
			//	pow((tmp_sln6 - r_sln6), 2) + pow((tmp_sln7 - r_sln7), 2) + pow((tmp_sln8 - r_sln8), 2) +
			//	pow((tmp_sln9 - t_sln0), 2) + pow((tmp_sln10 - t_sln1), 2) + pow((tmp_sln11 - t_sln2), 2);
			float test_0 = pow((tmp_sln3 - r_sln3), 2) + pow((tmp_sln6 - r_sln6), 2) + pow((tmp_sln7 - r_sln7), 2) +
				pow((tmp_sln9 - t_sln0), 2) + pow((tmp_sln10 - t_sln1), 2) + pow((tmp_sln11 - t_sln2), 2);

			//Test current
			var_vals.at(v_var.Name) = curr_val;
			var_vals.at("COS" + v_var.Name) = cos(curr_val);
			var_vals.at("SIN" + v_var.Name) = sin(curr_val);
			//tmp_sln0 = MatrixMath::SolveEquation(r_eqn0, var_vals);
			//tmp_sln1 = MatrixMath::SolveEquation(r_eqn1, var_vals);
			//tmp_sln2 = MatrixMath::SolveEquation(r_eqn2, var_vals);
			tmp_sln3 = Matrix4x4::SolveEquation(r_eqn3, var_vals);
			//tmp_sln4 = MatrixMath::SolveEquation(r_eqn4, var_vals);
			//tmp_sln5 = MatrixMath::SolveEquation(r_eqn5, var_vals);
			tmp_sln6 = Matrix4x4::SolveEquation(r_eqn6, var_vals);
			tmp_sln7 = Matrix4x4::SolveEquation(r_eqn7, var_vals);
			//tmp_sln8 = MatrixMath::SolveEquation(r_eqn8, var_vals);
			tmp_sln9 = Matrix4x4::SolveEquation(t_eqn0, var_vals);
			tmp_sln10 = Matrix4x4::SolveEquation(t_eqn1, var_vals);
			tmp_sln11 = Matrix4x4::SolveEquation(t_eqn2, var_vals);
			float test_1 = pow((tmp_sln3 - r_sln3), 2) + pow((tmp_sln6 - r_sln6), 2) + pow((tmp_sln7 - r_sln7), 2) +
				pow((tmp_sln9 - t_sln0), 2) + pow((tmp_sln10 - t_sln1), 2) + pow((tmp_sln11 - t_sln2), 2);

			//Test +1 increment
			var_vals.at(v_var.Name) = curr_val + v_increment;
			var_vals.at("COS" + v_var.Name) = cos(curr_val + v_increment);
			var_vals.at("SIN" + v_var.Name) = sin(curr_val + v_increment);
			//tmp_sln0 = MatrixMath::SolveEquation(r_eqn0, var_vals);
			//tmp_sln1 = MatrixMath::SolveEquation(r_eqn1, var_vals);
			//tmp_sln2 = MatrixMath::SolveEquation(r_eqn2, var_vals);
			tmp_sln3 = Matrix4x4::SolveEquation(r_eqn3, var_vals);
			//tmp_sln4 = MatrixMath::SolveEquation(r_eqn4, var_vals);
			//tmp_sln5 = MatrixMath::SolveEquation(r_eqn5, var_vals);
			tmp_sln6 = Matrix4x4::SolveEquation(r_eqn6, var_vals);
			tmp_sln7 = Matrix4x4::SolveEquation(r_eqn7, var_vals);
			//tmp_sln8 = MatrixMath::SolveEquation(r_eqn8, var_vals);
			tmp_sln9 = Matrix4x4::SolveEquation(t_eqn0, var_vals);
			tmp_sln10 = Matrix4x4::SolveEquation(t_eqn1, var_vals);
			tmp_sln11 = Matrix4x4::SolveEquation(t_eqn2, var_vals);
			float test_2 = pow((tmp_sln3 - r_sln3), 2) + pow((tmp_sln6 - r_sln6), 2) + pow((tmp_sln7 - r_sln7), 2) +
				pow((tmp_sln9 - t_sln0), 2) + pow((tmp_sln10 - t_sln1), 2) + pow((tmp_sln11 - t_sln2), 2);

			//Save each test condition
			if (test_0 < test_1 && test_0 < test_2)
			{
				var_vals.at(v_var.Name) = curr_val - v_increment;
				var_vals.at("COS" + v_var.Name) = cos(curr_val - v_increment);
				var_vals.at("SIN" + v_var.Name) = sin(curr_val - v_increment);
				b_cont = true;
			}
			else if (test_1 < test_0 && test_1 < test_2)
			{
				var_vals.at(v_var.Name) = curr_val;
				var_vals.at("COS" + v_var.Name) = cos(curr_val);
				var_vals.at("SIN" + v_var.Name) = sin(curr_val);
			}
			else if (test_2 < test_0 && test_2 < test_1)
			{
				//var_vals.at(v_var.Name) = curr_val + v_increment; //Should already be at these values anyway
				//var_vals.at("COS" + v_var.Name) = cos(curr_val + v_increment);
				//var_vals.at("SIN" + v_var.Name) = sin(curr_val + v_increment);
				b_cont = true;
			}
		}
	}// while (b_cont); //Set Some limiting condition

	return true;
}


VARVALS IKSolver::SolveMatrixAbs(Matrix4x4 m_solutions, VARVALS& var_vals, int matrix_it)
{
	if (matrix_it > chainJoints.size() - 1)
		throw std::runtime_error("Iterator larger than array");

	float rot_increment = 0.01;

	//Create variable names
	VARNAMES var_names = BuildVarVector();

	//Take 6 equations from the matrix
	Equation eqn_0 = chainJointWorldMatrices[matrix_it][0][3];
	Equation eqn_1 = chainJointWorldMatrices[matrix_it][1][3];
	Equation eqn_2 = chainJointWorldMatrices[matrix_it][2][3];
	//Equation eqn_3 = chainJointWorldMatrices[matrix_it][1][0];
	//Equation eqn_4 = chainJointWorldMatrices[matrix_it][2][0];
	//Equation eqn_5 = chainJointWorldMatrices[matrix_it][2][1];

	//Take 6 solutions from the matrix
	float sln_0 = m_solutions.Get(0,3);
	float sln_1 = m_solutions.Get(1,3);
	float sln_2 = m_solutions.Get(2,3);
	//float sln_3 = m_solutions[1][0];
	//float sln_4 = m_solutions[2][0];
	//float sln_5 = m_solutions[2][1];


	//Set all var values to center (this should be changed later to use last known values)
	for (auto &v_var : var_names)
	{
		float var_avr = (v_var.Min + v_var.Max) / 2;
		var_vals.at(v_var.Name) = var_avr;
		var_vals.at("COS" + v_var.Name) = cos(var_avr);
		var_vals.at("SIN" + v_var.Name) = sin(var_avr);
	}

	//First Do an Estimate solution by breaking up into segments
	const int segment_cnt = 100;
	for (auto &v_var : var_names)
	{
		float var_avr = (v_var.Min + v_var.Max) / 2;
		float segment_delta = (v_var.Max - v_var.Min) / segment_cnt;
		float smallest_test = 100000000000;
		float best_val = var_avr;
		for (int i = -segment_cnt/2; i <= segment_cnt/2; i++)
		{
			float var_val = var_avr + i * segment_delta;
			var_vals.at(v_var.Name) = var_val;
			var_vals.at("COS" + v_var.Name) = cos(var_val);
			var_vals.at("SIN" + v_var.Name) = sin(var_val);
			//Solve 6 equations
			float tmp_sln0 = Matrix4x4::SolveEquation(eqn_0, var_vals);
			float tmp_sln1 = Matrix4x4::SolveEquation(eqn_1, var_vals);
			float tmp_sln2 = Matrix4x4::SolveEquation(eqn_2, var_vals);
			//float tmp_sln3 = MatrixMath::SolveEquation(eqn_3, var_vals);
			//float tmp_sln4 = MatrixMath::SolveEquation(eqn_4, var_vals);
			//float tmp_sln5 = MatrixMath::SolveEquation(eqn_5, var_vals);
			float test_val = pow((tmp_sln0 - sln_0), 2) + pow((tmp_sln1 - sln_1), 2) + pow((tmp_sln2 - sln_2), 2);
				//+pow((tmp_sln3 - sln_3), 2) + pow((tmp_sln4 - sln_4), 2) + pow((tmp_sln5 - sln_5), 2);

			if (test_val < smallest_test)
			{
				smallest_test = test_val;
				best_val = var_val;
			}
		}
		//Set the value as the best found

		var_vals.at(v_var.Name) = best_val;
		var_vals.at("COS" + v_var.Name) = cos(best_val);
		var_vals.at("SIN" + v_var.Name) = sin(best_val);
	}


	//Solve for translation
	//float v_test = 1.0F;
	//do
	//{
	//	for (auto &v_var : var_names)
	//	{
	//		float var_val = var_vals.at(v_var.Name);
	//
	//		var_vals.at(v_var.Name) = var_val;
	//		var_vals.at("COS" + v_var.Name) = cos(var_val);
	//		var_vals.at("SIN" + v_var.Name) = sin(var_val);
	//		//Solve 6 equations
	//		float tmp_sln0 = MatrixMath::SolveEquation(eqn_0, var_vals);
	//		float tmp_sln1 = MatrixMath::SolveEquation(eqn_1, var_vals);
	//		float tmp_sln2 = MatrixMath::SolveEquation(eqn_2, var_vals);
	//		float tmp_sln3 = MatrixMath::SolveEquation(eqn_3, var_vals);
	//		float tmp_sln4 = MatrixMath::SolveEquation(eqn_4, var_vals);
	//		float tmp_sln5 = MatrixMath::SolveEquation(eqn_5, var_vals);
	//		float test_val = pow((tmp_sln0 - sln_0), 2) + pow((tmp_sln1 - sln_1), 2) + pow((tmp_sln2 - sln_2), 2);
	//
	//		if (test_val < smallest_test)
	//		{
	//			smallest_test = test_val;
	//			best_val = var_val;
	//		}
	//		
	//		//Set the value as the best found
	//
	//		var_vals.at(v_var.Name) = best_val;
	//		var_vals.at("COS" + v_var.Name) = cos(best_val);
	//		var_vals.at("SIN" + v_var.Name) = sin(best_val);
	//	}	
	//} while (v_test > 0.01F); //Set Some limiting condition

	return var_vals;
}