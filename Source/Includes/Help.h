#pragma once
#ifndef __linux__
#include "Windows.h"
#endif

#include "math.h"
#include <sstream>
#include <iterator>
#include <vector>
#include <array>
#include <iomanip> // setprecision
#include <map>
#include <mutex>
#include <iostream>

#ifdef __linux__
#include <algorithm> //is_permutation
#include <stdexcept>
#endif

#ifdef _DEBUG
static void WINAPIV DebugOut(const char* p_format, ...) 
{	
	static char s_printf_buf[1024];
	va_list v_args;
	va_start(v_args, p_format);
	_vsnprintf_s(s_printf_buf, 1024, sizeof(s_printf_buf), p_format, v_args);
	va_end(v_args);
	OutputDebugStringA(s_printf_buf);
}
#else
#define DebugOut(f_, ...) printf((f_), __VA_ARGS__);std::cout.flush();
#endif

#define MAT4X4 std::array<std::array<float,4>,4>
#define MAT4X4EQN std::array<std::array<Equation,4>,4>
#define IDENTITY {{ {{1.0,0.0,0.0,0.0}},{{0.0,1.0,0.0,0.0}},{{0.0,0.0,1.0,0.0}},{{0.0,0.0,0.0,1.0}} }}
#define IDENTITYEQN {{{{Equation(1.0F),Equation(0.0F),Equation(0.0F),Equation(0.0F)}}, \
					  {{Equation(0.0F),Equation(1.0F),Equation(0.0F),Equation(0.0F)}}, \
					  {{Equation(0.0F),Equation(0.0F),Equation(1.0F),Equation(0.0F)}}, \
					  {{Equation(0.0F),Equation(0.0F),Equation(0.0F),Equation(1.0F)}}}}
#define VARVALS std::map<std::string, float>

struct Variables
{
	//Everything Inside variable is multiplied
	bool Zero = true;
	float Num = 0.0F;
	std::vector<std::string> Vars = {};

	Variables(float v_num = 0.0F, std::vector<std::string> v_vars = {})
	{
		Zero = abs(v_num) < 0.000001F;
		if (!Zero)
		{
			Num = v_num;
			Vars = v_vars;
		}
	}


	friend bool operator== (const Variables v_a, const Variables v_b)
	{
		//If Vars from A and B are the same
		if (v_a.Vars.size() == 0 && v_b.Vars.size() == 0)
			return true;
		else if (v_a.Vars.size() != v_b.Vars.size())
			return false;
		else
			return (std::is_permutation(v_a.Vars.begin(), v_a.Vars.end(), v_b.Vars.begin()));
	}

	friend std::vector<Variables> operator+ (const Variables v_a, const Variables v_b)
	{
		//If Vars from A and B are the same just add the numerical values
		if (v_a== v_b)
			return{ Variables(v_a.Num + v_b.Num, v_a.Vars) };
		else
			return{ v_a , v_b };
	}

	friend void operator+= (Variables& v_a, const Variables v_b)
	{
		//If Vars from A and B are the same just add the numerical values
		if (v_a == v_b)
			v_a.Num += v_b.Num;
		else
			throw std::runtime_error("Tried to += Unequal variables");
	}

	friend Variables operator* (const Variables v_a, const Variables v_b)
	{
		if (v_a.Zero || v_b.Zero)
			return Variables();
		//Add All variables from B to A
		std::vector<std::string> v_temp = v_a.Vars;
		v_temp.insert(std::end(v_temp), std::begin(v_b.Vars), std::end(v_b.Vars));
		return Variables(v_a.Num * v_b.Num, v_temp);
	}

	std::string Str()
	{
		std::string v_output;
		std::stringstream v_stream;
		v_stream << std::fixed << std::setprecision(2) << Num;
		v_output = v_stream.str();

		for (auto& v_var : Vars)
			v_output = v_output + "*" + v_var;
		return v_output;
	}
};

struct Equation
{
	//Everything inside  the equation is added
	std::vector<Variables> Vals = {};

	Equation()
	{
		//std::vector<Variables> vals = { Variables() };
	}

	Equation(std::vector<Variables> v_vals = {})
	{
		//Group equal variables
		for (int i = 0; i < v_vals.size(); i++)
		{
			if (!v_vals[i].Zero)
			{
				for (int j = i + 1; j < v_vals.size(); j++)
				{
					if (v_vals[i] == v_vals[j] && !v_vals[j].Zero)
					{
						v_vals[i] += v_vals[j];
						v_vals[j].Num = 0.0F;
						v_vals[j].Zero = true;
					}
				}
				if (!v_vals[i].Zero)
					Vals.push_back(v_vals[i]);
			}
		}
		//Add Numerical Zero if empty
		if (Vals.size() == 0)
			Vals.push_back(Variables());
	}

	Equation(float v_num)
	{
		Vals = { Variables(v_num, {}) };
	}

	Equation(std::string v_var)
	{
		if (v_var.size() == 0)
			throw std::runtime_error("Bad Equation input String");
		else if(v_var.at(0) == '-')
			Vals = { Variables(-1.0F,{ v_var.erase(0,1) }) };
		else
			Vals = { Variables(1.0F,{ v_var }) };
	}

	friend Equation operator+ (const Equation v_a, const Equation v_b)
	{
		std::vector<Variables> v_result;

		std::move(v_a.Vals.begin(), v_a.Vals.end(), std::back_inserter(v_result));
		std::move(v_b.Vals.begin(), v_b.Vals.end(), std::back_inserter(v_result));

		return Equation(v_result);
	}

	friend Equation operator* (const Equation v_a, const Equation v_b)
	{
		std::vector<Variables> v_result;
		for(auto& a_val : v_a.Vals)
			for (auto& b_val : v_b.Vals)
			{
				auto v_temp = a_val*b_val;
				v_result.push_back(v_temp);
			}
		return Equation(v_result);
	}

	std::string Str()
	{
		std::string v_temp = "";
		for (auto& v_val : Vals)
			v_temp += v_val.Str() + "+";
		if(v_temp !="")
			v_temp.pop_back();
		return v_temp;
	}
};


namespace MatrixMath
{
	static MAT4X4 MatrixMult4x4(MAT4X4 matrix_left, MAT4X4 matrix_right)
	{
		MAT4X4 matrix_out = IDENTITY;

		//Perform Matrix Multiplication
		for (int v_row = 0; v_row < 4; v_row++)
		{
			for (int v_col = 0; v_col < 4; v_col++)
			{
				matrix_out[v_row][v_col] =
					(matrix_left[v_row][0] * matrix_right[0][v_col]) +
					(matrix_left[v_row][1] * matrix_right[1][v_col]) +
					(matrix_left[v_row][2] * matrix_right[2][v_col]) +
					(matrix_left[v_row][3] * matrix_right[3][v_col]);
			}
		}

		return matrix_out;
	}

	static bool MatrixInv4x4(const MAT4X4 mat_in, MAT4X4& mat_inv_out)
	{
		MAT4X4 inv = IDENTITY;
		float det;

		inv[0][0] = mat_in[1][1] * mat_in[2][2] * mat_in[3][3] -
			mat_in[1][1] * mat_in[2][3] * mat_in[3][2] -
			mat_in[2][1] * mat_in[1][2] * mat_in[3][3] +
			mat_in[2][1] * mat_in[1][3] * mat_in[3][2] +
			mat_in[3][1] * mat_in[1][2] * mat_in[2][3] -
			mat_in[3][1] * mat_in[1][3] * mat_in[2][2];

		inv[1][0] = -mat_in[1][0] * mat_in[2][2] * mat_in[3][3] +
			mat_in[1][0] * mat_in[2][3] * mat_in[3][2] +
			mat_in[2][0] * mat_in[1][2] * mat_in[3][3] -
			mat_in[2][0] * mat_in[1][3] * mat_in[3][2] -
			mat_in[3][0] * mat_in[1][2] * mat_in[2][3] +
			mat_in[3][0] * mat_in[1][3] * mat_in[2][2];

		inv[2][0] = mat_in[1][0] * mat_in[2][1] * mat_in[3][3] -
			mat_in[1][0] * mat_in[2][3] * mat_in[3][1] -
			mat_in[2][0] * mat_in[1][1] * mat_in[3][3] +
			mat_in[2][0] * mat_in[1][3] * mat_in[3][1] +
			mat_in[3][0] * mat_in[1][1] * mat_in[2][3] -
			mat_in[3][0] * mat_in[1][3] * mat_in[2][1];

		inv[3][0] = -mat_in[1][0] * mat_in[2][1] * mat_in[3][2] +
			mat_in[1][0] * mat_in[2][2] * mat_in[3][1] +
			mat_in[2][0] * mat_in[1][1] * mat_in[3][2] -
			mat_in[2][0] * mat_in[1][2] * mat_in[3][1] -
			mat_in[3][0] * mat_in[1][1] * mat_in[2][2] +
			mat_in[3][0] * mat_in[1][2] * mat_in[2][1];

		inv[0][1] = -mat_in[0][1] * mat_in[2][2] * mat_in[3][3] +
			mat_in[0][1] * mat_in[2][3] * mat_in[3][2] +
			mat_in[2][1] * mat_in[0][2] * mat_in[3][3] -
			mat_in[2][1] * mat_in[0][3] * mat_in[3][2] -
			mat_in[3][1] * mat_in[0][2] * mat_in[2][3] +
			mat_in[3][1] * mat_in[0][3] * mat_in[2][2];

		inv[1][1] = mat_in[0][0] * mat_in[2][2] * mat_in[3][3] -
			mat_in[0][0] * mat_in[2][3] * mat_in[3][2] -
			mat_in[2][0] * mat_in[0][2] * mat_in[3][3] +
			mat_in[2][0] * mat_in[0][3] * mat_in[3][2] +
			mat_in[3][0] * mat_in[0][2] * mat_in[2][3] -
			mat_in[3][0] * mat_in[0][3] * mat_in[2][2];

		inv[2][1] = -mat_in[0][0] * mat_in[2][1] * mat_in[3][3] +
			mat_in[0][0] * mat_in[2][3] * mat_in[3][1] +
			mat_in[2][0] * mat_in[0][1] * mat_in[3][3] -
			mat_in[2][0] * mat_in[0][3] * mat_in[3][1] -
			mat_in[3][0] * mat_in[0][1] * mat_in[2][3] +
			mat_in[3][0] * mat_in[0][3] * mat_in[2][1];

		inv[3][1] = mat_in[0][0] * mat_in[2][1] * mat_in[3][2] -
			mat_in[0][0] * mat_in[2][2] * mat_in[3][1] -
			mat_in[2][0] * mat_in[0][1] * mat_in[3][2] +
			mat_in[2][0] * mat_in[0][2] * mat_in[3][1] +
			mat_in[3][0] * mat_in[0][1] * mat_in[2][2] -
			mat_in[3][0] * mat_in[0][2] * mat_in[2][1];

		inv[0][2] = mat_in[0][1] * mat_in[1][2] * mat_in[3][3] -
			mat_in[0][1] * mat_in[1][3] * mat_in[3][2] -
			mat_in[1][1] * mat_in[0][2] * mat_in[3][3] +
			mat_in[1][1] * mat_in[0][3] * mat_in[3][2] +
			mat_in[3][1] * mat_in[0][2] * mat_in[1][3] -
			mat_in[3][1] * mat_in[0][3] * mat_in[1][2];

		inv[1][2] = -mat_in[0][0] * mat_in[1][2] * mat_in[3][3] +
			mat_in[0][0] * mat_in[1][3] * mat_in[3][2] +
			mat_in[1][0] * mat_in[0][2] * mat_in[3][3] -
			mat_in[1][0] * mat_in[0][3] * mat_in[3][2] -
			mat_in[3][0] * mat_in[0][2] * mat_in[1][3] +
			mat_in[3][0] * mat_in[0][3] * mat_in[1][2];

		inv[2][2] = mat_in[0][0] * mat_in[1][1] * mat_in[3][3] -
			mat_in[0][0] * mat_in[1][3] * mat_in[3][1] -
			mat_in[1][0] * mat_in[0][1] * mat_in[3][3] +
			mat_in[1][0] * mat_in[0][3] * mat_in[3][1] +
			mat_in[3][0] * mat_in[0][1] * mat_in[1][3] -
			mat_in[3][0] * mat_in[0][3] * mat_in[1][1];

		inv[3][2] = -mat_in[0][0] * mat_in[1][1] * mat_in[3][2] +
			mat_in[0][0] * mat_in[1][2] * mat_in[3][1] +
			mat_in[1][0] * mat_in[0][1] * mat_in[3][2] -
			mat_in[1][0] * mat_in[0][2] * mat_in[3][1] -
			mat_in[3][0] * mat_in[0][1] * mat_in[1][2] +
			mat_in[3][0] * mat_in[0][2] * mat_in[1][1];

		inv[0][3] = -mat_in[0][1] * mat_in[1][2] * mat_in[2][3] +
			mat_in[0][1] * mat_in[1][3] * mat_in[2][2] +
			mat_in[1][1] * mat_in[0][2] * mat_in[2][3] -
			mat_in[1][1] * mat_in[0][3] * mat_in[2][2] -
			mat_in[2][1] * mat_in[0][2] * mat_in[1][3] +
			mat_in[2][1] * mat_in[0][3] * mat_in[1][2];

		inv[1][3] = mat_in[0][0] * mat_in[1][2] * mat_in[2][3] -
			mat_in[0][0] * mat_in[1][3] * mat_in[2][2] -
			mat_in[1][0] * mat_in[0][2] * mat_in[2][3] +
			mat_in[1][0] * mat_in[0][3] * mat_in[2][2] +
			mat_in[2][0] * mat_in[0][2] * mat_in[1][3] -
			mat_in[2][0] * mat_in[0][3] * mat_in[1][2];

		inv[2][3] = -mat_in[0][0] * mat_in[1][1] * mat_in[2][3] +
			mat_in[0][0] * mat_in[1][3] * mat_in[2][1] +
			mat_in[1][0] * mat_in[0][1] * mat_in[2][3] -
			mat_in[1][0] * mat_in[0][3] * mat_in[2][1] -
			mat_in[2][0] * mat_in[0][1] * mat_in[1][3] +
			mat_in[2][0] * mat_in[0][3] * mat_in[1][1];

		inv[3][3] = mat_in[0][0] * mat_in[1][1] * mat_in[2][2] -
			mat_in[0][0] * mat_in[1][2] * mat_in[2][1] -
			mat_in[1][0] * mat_in[0][1] * mat_in[2][2] +
			mat_in[1][0] * mat_in[0][2] * mat_in[2][1] +
			mat_in[2][0] * mat_in[0][1] * mat_in[1][2] -
			mat_in[2][0] * mat_in[0][2] * mat_in[1][1];

		det = mat_in[0][0] * inv[0][0] + mat_in[0][1] * inv[1][0] + mat_in[0][2] * inv[2][0] + mat_in[0][3] * inv[3][0];

		if (det == 0)
			return false;

		det = 1.0 / det;

		for (int v_row = 0; v_row < 4; v_row++)
			for (int v_col = 0; v_col < 4; v_col++)
				mat_inv_out[v_row][v_col] = inv[v_row][v_col] * det;

		return true;
	}

	static MAT4X4EQN MatToEqn4x4(MAT4X4 input_mat)
	{
		MAT4X4EQN eqn_mat = IDENTITYEQN;

		//Perform Matrix Multiplication
		for (int v_row = 0; v_row < 4; v_row++)
		{
			for (int v_col = 0; v_col < 4; v_col++)
			{
				eqn_mat[v_row][v_col] = Equation(input_mat[v_row][v_col]);
			}
		}
		return eqn_mat;
	}

	static MAT4X4EQN MatrixMult4x4(MAT4X4EQN& matrix_left, MAT4X4EQN& matrix_right)
	{
		MAT4X4EQN matrix_out = IDENTITYEQN;

		//Perform Matrix Multiplication
		for (int v_row = 0; v_row < 4; v_row++)
		{
			for (int v_col = 0; v_col < 4; v_col++)
			{				
				matrix_out[v_row][v_col] =
					(matrix_left[v_row][0] * matrix_right[0][v_col]) +
					(matrix_left[v_row][1] * matrix_right[1][v_col]) +
					(matrix_left[v_row][2] * matrix_right[2][v_col]) +
					(matrix_left[v_row][3] * matrix_right[3][v_col]);
			}
		}
		return matrix_out;
	}

	static float SolveEquation(Equation& v_eqn, VARVALS& variable_values)
	{
		//Add all values within Equation
		float value_eqn = 0.0F;
		for(auto& v_val : v_eqn.Vals)
		{
			if (!v_val.Zero)
			{
				//Multiply all values within Variables
				float temp_mult = 1.0F;
				for(auto& v_var : v_val.Vars)
				{
					//Get Variables Value at current angle
					temp_mult *= variable_values.at(v_var);
				}
				value_eqn += v_val.Num * temp_mult;
			}
		}
		return value_eqn;
	}

	static MAT4X4 MatFromEqn4x4(MAT4X4EQN& input_mat, VARVALS& variable_values)
	{
		MAT4X4 matrix_out = IDENTITY;
		//Perform Matrix Multiplication
		for (int v_row = 0; v_row < 4; v_row++)
		{
			for (int v_col = 0; v_col < 4; v_col++)
			{
				matrix_out[v_row][v_col] = SolveEquation(input_mat[v_row][v_col], variable_values);
			}
		}
		return matrix_out;
	}
	
	static void PrintMatrix(MAT4X4& m_matrix, std::string v_name)
	{
		DebugOut("[--%s--] Joint Matrix\n", v_name.c_str());
		for (int v_row = 0; v_row < 4; v_row++)
		{
			DebugOut("%.2f\t %.2f\t %.2f\t %.2f\n", m_matrix[v_row][0], m_matrix[v_row][1], m_matrix[v_row][2], m_matrix[v_row][3]);
		}
		DebugOut("\n");
	}

	static void PrintMatrixEqn(MAT4X4EQN& m_matrix, std::string v_name)
	{
		DebugOut("[--%s--]IK Equation Matrix\n", v_name.c_str());
		for (int v_row = 0; v_row < 4; v_row++)
		{
			DebugOut("%s\t\t\t\t %s\t\t\t\t %s\t\t\t\t %s\n", m_matrix[v_row][0].Str().c_str(), m_matrix[v_row][1].Str().c_str(), m_matrix[v_row][2].Str().c_str(), m_matrix[v_row][3].Str().c_str());
		}
		DebugOut("\n");
	}

	static MAT4X4 TaitBryanAnglesToMat(float first_about_x, float next_about_y, float last_about_z, float x, float y, float z)
	{
		float c1 = cos(last_about_z);
		float s1 = sin(last_about_z);
		float c2 = cos(next_about_y);
		float s2 = sin(next_about_y);
		float c3 = cos(first_about_x);
		float s3 = sin(first_about_x);

		//MAT4X4 move{{ {{1.0F,0.0F,0.0F,x}},{{0.0F,1.0F,0.0F,y}},{{0.0F,0.0F,1.0F,z}},{{0.0F,0.0F,0.0F,1.0F}} }};
		//MAT4X4 rotate{{ {{c1*c2, c1*s2*s3 - c3*s1, s1*s3 + c1*c3*s2, 0.0}},{{c2*s1, c1*c3 + s1*s2*s3,c3*s1*s2 - c1*s3, 0.0F}},{{-s2,c2*s3, c2*c3, 0.0F}},{{0.0F,0.0F,0.0F,1.0F}} }};
		//MAT4X4 ret = MatrixMult4x4(move, rotate);

		MAT4X4 m_ret{ { { { c1*c2, c1*s2*s3 - c3*s1, s1*s3 + c1*c3*s2, x } },{ { c2*s1, c1*c3 + s1*s2*s3,c3*s1*s2 - c1*s3, y } },{ { -s2,c2*s3, c2*c3, z } },{ { 0.0F,0.0F,0.0F,1.0F } } } };

		return m_ret;
	}

	static bool MatToTaitBryanAngles(MAT4X4& input_mat, float& first_about_x, float& next_about_y, float& last_about_z, float& v_x, float& v_y, float& v_z)
	{
		//float c1 = cos(last_about_z);
		//float s1 = sin(last_about_z);
		//float c2 = cos(next_about_y);
		//float s2 = sin(next_about_y);
		//float c3 = cos(first_about_x);
		//float s3 = sin(first_about_x);

		//MAT4X4 move{ { { { 1.0F,0.0F,0.0F,x } },{ { 0.0F,1.0F,0.0F,y } },{ { 0.0F,0.0F,1.0F,z } },{ { 0.0F,0.0F,0.0F,1.0F } } } };
		//MAT4X4 rotate{ { { { c1*c2, c1*s2*s3 - c3*s1, s1*s3 + c1*c3*s2, 0.0 } },{ { c2*s1, c1*c3 + s1*s2*s3,c3*s1*s2 - c1*s3, 0.0F } },{ { -s2,c2*s3, c2*c3, 0.0F } },{ { 0.0F,0.0F,0.0F,1.0F } } } };
		//MAT4X4 ret = MatrixMult4x4(move, rotate);

		next_about_y = asinf(-input_mat[2][0]);
		last_about_z = asinf(input_mat[1][0]/cos(next_about_y));
		first_about_x = asinf(input_mat[2][1]/cos(next_about_y));
		v_x = input_mat[0][3];
		v_y = input_mat[1][3];
		v_z = input_mat[2][3];

		return true;
	}
}

struct Returns
{
	std::map<std::string, std::vector<std::string>> errorCodes;
	std::mutex mMutex;

	void Throw(std::string v_name, std::string error_code)
	{
		std::unique_lock<std::mutex> lock(mMutex); //For thread safety
		DebugOut("%s:%s\n", v_name.c_str(), error_code.c_str());
		if (errorCodes.count(v_name) == 0)
			errorCodes.insert(std::map<std::string, std::vector<std::string>>::value_type(v_name, std::vector<std::string>({ error_code }))); //Create a ret code caller if doesn't exist
		else
			errorCodes.at(v_name).push_back(error_code); //Append ret code to existing caller
	}
};