#pragma once
#include <string>
#include <iostream>
//#include <sstream>      // std::ostringstream
#include <d3d11.h>
#include <vector>
#include <DirectXMath.h>
#include <memory>
#include <map>
#include <functional>
#include <mutex>
#include <array>
#include <iostream>
#include <iomanip>
#include <sstream>
//#include "RenderEngine/Direct3D/Direct3DRenderer.h"
//#include "OsEngine/Windows/CustomControl.h"
//#include "MechatronicLib/Loader.h"

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

#define CALLBACKS std::map<std::string, std::function<void(std::shared_ptr<CallbackData::BlankData>)>>


#define TO_XMMATRIX(_val) XMMatrixTranspose(XMMATRIX(_val.mElements))
#define PI 3.14159265359F

#define TOSTRING(x) (std::string((x).begin(), (x).end()))
#define TOWSTRING(x) (std::wstring((x).begin(), (x).end()))
#define TOLPCWSTR(x) (std::wstring((x).begin(), (x).end()).c_str())
//#define WCHARTOSTRING(x) (std::string((std::wstring(x)).begin(), (std::wstring(x)).end()))


//#define DIK_UP				0xC8 
//#define DIK_DOWN            0xD0
//#define DIK_LEFT            0xCB
//#define DIK_RIGHT           0xCD


//Thread Locking Mutex
#ifndef MUTEX_LOCK
#define MUTEX_LOCK
static std::mutex mMutex;
#endif


#define IDENTITY {{ {{1.0,0.0,0.0,0.0}},{{0.0,1.0,0.0,0.0}},{{0.0,0.0,1.0,0.0}},{{0.0,0.0,0.0,1.0}} }}
#define IDENTITYEQN {{{{Equation(1.0F),Equation(0.0F),Equation(0.0F),Equation(0.0F)}}, \
					  {{Equation(0.0F),Equation(1.0F),Equation(0.0F),Equation(0.0F)}}, \
					  {{Equation(0.0F),Equation(0.0F),Equation(1.0F),Equation(0.0F)}}, \
					  {{Equation(0.0F),Equation(0.0F),Equation(0.0F),Equation(1.0F)}}}}
#define VARVALS std::map<std::string, float>
#define MAT4X4EQN std::array<std::array<Equation,4>,4>

//Forward Declarations for callbacks only
class LdrMechanisms;
class Direct3DRenderer;
class CustomControl;
class IModel;

//std::string ftos(float f, int nd) {
//	std::ostringstream ostr;
//	int tens = std::stoi("1" + std::string(nd, '0'));
//	ostr << round(f*tens) / tens;
//	return ostr.str().c_str();
//}


struct Vector4
{
	Vector4()
	{
		x = 0.0F; y = 0.0F; z = 0.0F;
	}

	Vector4(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	Vector4 Normalize()
	{
		float scale = sqrt(x*x + y*y + z*z);
		return Vector4(x/scale, y/scale, z/scale);
	}

	Vector4 ToSpehrical()
	{
		float r = sqrt(x*x + y*y + z*z);
		float theta_deg = acos(z / r) * 57.2958;
		float phi_deg = atan2(y, x) * 57.2958;
		return Vector4(r, theta_deg, phi_deg);
	}

	Vector4 FromSpehrical()
	{
		float r = x;
		float theta_deg = y; 
		float phi_deg = z;
		x = r*sin(theta_deg * 0.0174533F)*cos(phi_deg* 0.0174533F);
		y = r*sin(theta_deg * 0.0174533F)*sin(phi_deg* 0.0174533F);
		z = r*cos(theta_deg * 0.0174533F);
		return Vector4(x, y, z);
	}

	Vector4 Cross(Vector4 vector_2)
	{
		Vector4 c_product;
		c_product.x = this->y * vector_2.z - this->z * vector_2.y;
		c_product.y = this->z * vector_2.x - this->x * vector_2.z;
		c_product.z = this->x * vector_2.y - this->y * vector_2.x;
		return c_product;
	}


	Vector4 NormCross(Vector4 vector_2)
	{
		Vector4 this_temp = this->Normalize();
		vector_2 = vector_2.Normalize();
		Vector4 c_product;
		c_product.x = this_temp.y * vector_2.z - this_temp.z * vector_2.y;
		c_product.y = this_temp.z * vector_2.x - this_temp.x * vector_2.z;
		c_product.z = this_temp.x * vector_2.y - this_temp.y * vector_2.x;
		return c_product.Normalize();
	}
	
	float Dot(Vector4 vector_2)
	{
		float d_product = this->x * vector_2.x + this->y * vector_2.y + this->z * vector_2.z;
		return d_product;
	}

	float GetAngle(Vector4 vector_2)
	{
		Vector4 this_temp = this->Normalize();
		vector_2 = vector_2.Normalize();
		float d_product = this_temp.x * vector_2.x + this_temp.y * vector_2.y + this_temp.z * vector_2.z;
		return acos(d_product);
	}

	inline Vector4 operator+(const Vector4& rhs)
	{
		Vector4 out;
		out.x = this->x + rhs.x;
		out.y = this->y + rhs.y;
		out.z = this->z + rhs.z;
		return out;
	}

	inline Vector4 operator-(const Vector4& rhs)
	{
		Vector4 out;
		out.x = this->x - rhs.x;
		out.y = this->y - rhs.y;
		out.z = this->z - rhs.z;
		return out;
	}

	inline Vector4 operator/(const float& rhs)
	{
		Vector4 out;
		out.x = this->x / rhs;
		out.y = this->y / rhs;
		out.z = this->z / rhs;
		return out;
	}

	inline Vector4 operator*(const float& rhs)
	{
		Vector4 out;
		out.x = this->x * rhs;
		out.y = this->y * rhs;
		out.z = this->z * rhs;
		return out;
	}

	float x; //r
	float y; //theta_deg
	float z; //phi_deg
};


struct Quaternion
{
	Quaternion()
	{
		x = 0.0F; y = 0.0F; z = 0.0F; w = 1.0F;
	}

	Quaternion(float _x, float _y, float _z, float _w)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}

	bool Undefined()
	{
		return(x != x || y != y || z != z || w != w);
	}

	Quaternion(std::pair<Vector4, float> axis_angle_radians)
	{
		auto axis = axis_angle_radians.first.Normalize();
		float s = sinf(axis_angle_radians.second / 2.0F);
		x = axis.x * s;
		y = axis.y * s;
		z = axis.z * s;
		w = cosf(axis_angle_radians.second / 2.0F);
	}

	inline Quaternion& operator*(const Quaternion& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;
		return *this;
	}

	Quaternion Normalize()
	{
		float scale = sqrt(x*x + y*y + z*z + w*w);
		return Quaternion(x / scale, y / scale, z / scale, w / scale);
	}

	std::pair<Vector4, float> ToAxisAngle()
	{
		Vector4 axis;
		if (this->w > 1.0F)
			this->Normalize(); // if w>1 acos and sqrt will produce errors, this cant happen if quaternion is normalised
		float angle = 2.0F * acosf(this->w);
		float s = sqrtf(1.0F - this->w*this->w); // assuming quaternion normalised then w is less than 1, so term always positive.
		if (s < 0.001) { // test to avoid divide by zero, s is always positive due to sqrt
						 // if s close to zero then direction of axis not important
			axis.x = this->x; // if it is important that axis is normalised then replace with x=1; y=z=0;
			axis.y = this->y;
			axis.z = this->z;
		}
		else {
			axis.x = this->x / s; // normalise axis
			axis.y = this->y / s;
			axis.z = this->z / s;
		}
		return std::make_pair(axis, angle);
	}


	inline Quaternion operator+(const Quaternion& rhs)
	{
		Quaternion out;
		out.x = this->x + rhs.x;
		out.y = this->y + rhs.y;
		out.z = this->z + rhs.z;
		out.w = this->w + rhs.w;
		return out;
	}

	inline Quaternion operator-(const Quaternion& rhs)
	{
		Quaternion out;
		out.x = this->x - rhs.x;
		out.y = this->y - rhs.y;
		out.z = this->z - rhs.z;
		out.w = this->w - rhs.w;
		return out;
	}

	float x;
	float y;
	float z;
	float w;
};

struct Euler
{
	Euler()
	{
		aX =0.0F;
		aY =0.0F;
		aZ =0.0F;
		X = 0.0F;
		Y = 0.0F;
		Z = 0.0F;
	}

	Euler(float _x, float _y, float _z, float a_x, float a_y, float a_z)
	{
		aX = a_x;
		aY = a_y;
		aZ = a_z;
		X = _x;
		Y = _y;
		Z = _z;
	}

	inline Euler operator-(const Euler& rhs)
	{
		Euler out;
		out.X = this->X - rhs.X;
		out.Y = this->Y - rhs.Y;
		out.Z = this->Z - rhs.Z;
		out.aX = this->aX - rhs.aX;
		out.aY = this->aY - rhs.aY;
		out.aZ = this->aZ - rhs.aZ;
		return out;
	}

	inline Euler operator+(const Euler& rhs)
	{
		Euler out;
		out.X = this->X + rhs.X;
		out.Y = this->Y + rhs.Y;
		out.Z = this->Z + rhs.Z;
		out.aX = this->aX + rhs.aX;
		out.aY = this->aY + rhs.aY;
		out.aZ = this->aZ + rhs.aZ;
		return out;
	}

	static float Compare(const Euler& rhs, const Euler& lhs)
	{
		return sqrtf(powf(rhs.X - lhs.X, 2) + powf(rhs.Y - lhs.Y, 2) + powf(rhs.Z - lhs.Z, 2)
			+ powf(rhs.aX - lhs.aX, 2) + powf(rhs.aY - lhs.aY, 2) + powf(rhs.aZ - lhs.aZ, 2));
	}

	float aX, aY, aZ, X, Y, Z;
};



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
		if (v_a == v_b)
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
		else if (v_var.at(0) == '-')
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
		for (auto& a_val : v_a.Vals)
			for (auto& b_val : v_b.Vals)
			{
				auto v_temp = a_val * b_val;
				v_result.push_back(v_temp);
			}
		return Equation(v_result);
	}

	std::string Str()
	{
		std::string v_temp = "";
		for (auto& v_val : Vals)
			v_temp += v_val.Str() + "+";
		if (v_temp != "")
			v_temp.pop_back();
		return v_temp;
	}
};

//TODO: Reverse Matrix Multiplication
class Matrix4x4
{
public:
	float mElements[16];

	Matrix4x4()
	{
		memset(&this->mElements, 0, sizeof(this->mElements));
		this->mElements[0] = 1;
		this->mElements[5] = 1;
		this->mElements[10] = 1;
		this->mElements[15] = 1;
	}

	Matrix4x4(const float (&mElements)[16])
	{
		for (int i = 0; i < 16; i++)
			this->mElements[i] = mElements[i];
	}
	
	//float& operator [](const int& i)
	//{
	//	return mElements[i];
	//}

	float& Get(int a, int b)
	{
		return mElements[a*4 + b];
	}

	float DetRotation()
	{
		return mElements[0] * (mElements[5] * mElements[10] - mElements[6] * mElements[9]) 
			- mElements[1] * (mElements[4] * mElements[10] - mElements[6] * mElements[8]) 
			+ mElements[2] * (mElements[4] * mElements[9] - mElements[5] * mElements[8]);
	}
	
	static Matrix4x4 CreateFromAxisAngle(std::pair<Vector4, float> axis_angle_radians)
	{
		auto axis = axis_angle_radians.first.Normalize();
		Matrix4x4 newMatrix = Matrix4x4();
		float cos_a = cos(axis_angle_radians.second);
		float sin_a = sin(axis_angle_radians.second);

		newMatrix.mElements[0] = cos_a + axis.x* axis.x*(1 - cos_a);
		newMatrix.mElements[1] = axis.x * axis.y * (1 - cos_a) - axis.z*sin_a;
		newMatrix.mElements[2] = axis.x*axis.z*(1-cos_a) + axis.y * sin_a;

		newMatrix.mElements[4] = axis.y * axis.x * (1 - cos_a) + axis.z*sin_a;
		newMatrix.mElements[5] = cos_a + axis.y* axis.y*(1 - cos_a);
		newMatrix.mElements[6] = axis.y*axis.z*(1 - cos_a) - axis.x * sin_a;

		newMatrix.mElements[8] = axis.z*axis.x*(1 - cos_a) - axis.y * sin_a;
		newMatrix.mElements[9] = axis.z*axis.y*(1 - cos_a) + axis.x * sin_a;
		newMatrix.mElements[10] = cos_a + axis.z* axis.z*(1 - cos_a);

		DebugOut("Axis length = %f, Rot Determinant = %f, ", axis.x*axis.x + axis.y*axis.y + axis.z*axis.z, newMatrix.DetRotation());

		return newMatrix;
	}

	static Matrix4x4 CreateFromQuaterion(Quaternion input_quat)
	{
		return Matrix4x4::CreateFromAxisAngle(input_quat.ToAxisAngle());
	}

	Quaternion ConvertToQuaterion()
	{
		return Quaternion(this->ConvertToAxisAngle());
	}

	std::pair<Vector4, float> ConvertToAxisAngle()
	{
		float angle, x, y, z; // variables for result
		float epsilon = 0.01; // margin to allow for rounding errors
		float epsilon2 = 0.1; // margin to distinguish between 0 and 180 degrees
		//assert isRotationMatrix(m) : "not valid rotation matrix";// for debugging
		if ((abs(mElements[1] - mElements[4])< epsilon)
			&& (abs(mElements[2] - mElements[8])< epsilon)
			&& (abs(mElements[6] - mElements[9])< epsilon)) {
			// singularity found
			// first check for identity matrix which must have +1 for all terms
			//  in leading diagonaland zero in other terms
			if ((abs(mElements[1] + mElements[4]) < epsilon2)
				&& (abs(mElements[2] + mElements[8]) < epsilon2)
				&& (abs(mElements[6] + mElements[9]) < epsilon2)
				&& (abs(mElements[0] + mElements[5] + mElements[10] - 3) < epsilon2)) {
				// this singularity is identity matrix so angle = 0
				//return new axisAngle(0, 1, 0, 0); // zero angle, arbitrary axis
				return std::make_pair(Vector4(1, 0, 0), 0.0F);
			}
			// otherwise this singularity is angle = 180
			angle = PI;
			float xx = (mElements[0] + 1) / 2;
			float yy = (mElements[5] + 1) / 2;
			float zz = (mElements[10] + 1) / 2;
			float xy = (mElements[1] + mElements[4]) / 4;
			float xz = (mElements[2] + mElements[8]) / 4;
			float yz = (mElements[6] + mElements[9]) / 4;
			if ((xx > yy) && (xx > zz)) { // mElements[0] is the largest diagonal term
				if (xx< epsilon) {
					x = 0;
					y = 0.7071;
					z = 0.7071;
				}
				else {
					x = sqrt(xx);
					y = xy / x;
					z = xz / x;
				}
			}
			else if (yy > zz) { // mElements[5] is the largest diagonal term
				if (yy< epsilon) {
					x = 0.7071;
					y = 0;
					z = 0.7071;
				}
				else {
					y = sqrt(yy);
					x = xy / y;
					z = yz / y;
				}
			}
			else { // mElements[10] is the largest diagonal term so base result on this
				if (zz< epsilon) {
					x = 0.7071;
					y = 0.7071;
					z = 0;
				}
				else {
					z = sqrt(zz);
					x = xz / z;
					y = yz / z;
				}
			}
			return std::make_pair(Vector4(x, y, z), angle);// return 180 deg rotation
		}
		// as we have reached here there are no singularities so we can handle normally
		double s = sqrt((mElements[9] - mElements[6])*(mElements[9] - mElements[6])
			+ (mElements[2] - mElements[8])*(mElements[2] - mElements[8])
			+ (mElements[4] - mElements[1])*(mElements[4] - mElements[1])); // used to normalise
		if (abs(s) < 0.001) s = 1;
		// prevent divide by zero, should not happen if matrix is orthogonal and should be
		// caught by singularity test above, but I've left it in just in case
		angle = acos((mElements[0] + mElements[5] + mElements[10] - 1) / 2);
		x = (mElements[9] - mElements[6]) / s;
		y = (mElements[2] - mElements[8]) / s;
		z = (mElements[4] - mElements[1]) / s;
		return std::make_pair(Vector4(x, y, z), angle);
	}
	
	static Matrix4x4 CreateView(Vector4 position, Vector4 target, Vector4 up)
	{
		Vector4 forward = (target - position).Normalize();
		Vector4 right = (forward.Cross(up)).Normalize();
		up = (right.Cross(forward)).Normalize();
		Matrix4x4 newMatrixOne = Matrix4x4();
		newMatrixOne.mElements[0] = right.x;
		newMatrixOne.mElements[1] = right.y;
		newMatrixOne.mElements[2] = right.z;
		newMatrixOne.mElements[4] = up.x;
		newMatrixOne.mElements[5] = up.y;
		newMatrixOne.mElements[6] = up.z;
		newMatrixOne.mElements[8] = forward.x;
		newMatrixOne.mElements[9] = forward.y;
		newMatrixOne.mElements[10] = forward.z;

		Matrix4x4 newMatrixTwo = Matrix4x4();
		newMatrixTwo.mElements[3] = position.x;
		newMatrixTwo.mElements[7] = position.y;
		newMatrixTwo.mElements[11] = position.z;

		Matrix4x4 newMatrix = newMatrixOne * newMatrixTwo;
		return newMatrix;
	}

	static Matrix4x4 CreateView(Vector4 forward, Vector4 up, Vector4 right, Vector4 position)
	{
		Matrix4x4 newMatrixOne = Matrix4x4();
		newMatrixOne.mElements[0] = right.x;
		newMatrixOne.mElements[1] = right.y;
		newMatrixOne.mElements[2] = right.z;
		newMatrixOne.mElements[4] = up.x;
		newMatrixOne.mElements[5] = up.y;
		newMatrixOne.mElements[6] = up.z;
		newMatrixOne.mElements[8] = forward.x;
		newMatrixOne.mElements[9] = forward.y;
		newMatrixOne.mElements[10] = forward.z;

		Matrix4x4 newMatrixTwo = Matrix4x4();
		newMatrixTwo.mElements[3] = -position.x;
		newMatrixTwo.mElements[7] = -position.y;
		newMatrixTwo.mElements[11] = -position.z;

		Matrix4x4 newMatrix = newMatrixOne * newMatrixTwo;
		return newMatrix;
	}

	void DecomposeView(Vector4& forward, Vector4& up, Vector4& right, Vector4& position)
	{
		right.x=mElements[0]; 
		right.y=mElements[1]; 
		right.z=mElements[2]; 
		up.x=mElements[4]; 
		up.y=mElements[5]; 
		up.z=mElements[6]; 
		forward.x=mElements[8]; 
		forward.y=mElements[9]; 
		forward.z=mElements[10];
		position.x = -mElements[3];
		position.y = -mElements[7];
		position.z = -mElements[11];
	}
	
	static Matrix4x4 CreateRotX(float about_x_degrees)
	{
		Matrix4x4 newMatrix = Matrix4x4(); 
		float cos_x = cos(about_x_degrees * 0.0174533F);
		float sin_x = sin(about_x_degrees * 0.0174533F);
		newMatrix.mElements[5] = cos_x;
		newMatrix.mElements[6] = -sin_x;
		newMatrix.mElements[9] = sin_x;
		newMatrix.mElements[10] = cos_x;
		DebugOut("Rot Determinant = %f, ", newMatrix.DetRotation());
		return newMatrix;
	}

	static Matrix4x4 CreateRotY(float about_y_degrees)
	{
		Matrix4x4 newMatrix = Matrix4x4();
		float cos_x = cos(about_y_degrees * 0.0174533F);
		float sin_x = sin(about_y_degrees * 0.0174533F);
		newMatrix.mElements[0] = cos_x;
		newMatrix.mElements[2] = sin_x;
		newMatrix.mElements[8] = -sin_x;
		newMatrix.mElements[10] = cos_x;
		return newMatrix;
	}

	static Matrix4x4 CreateRotZ(float about_z_degrees)
	{
		Matrix4x4 newMatrix = Matrix4x4();
		float cos_x = cos(about_z_degrees * 0.0174533F);
		float sin_x = sin(about_z_degrees * 0.0174533F);
		newMatrix.mElements[0] = cos_x;
		newMatrix.mElements[1] = -sin_x;
		newMatrix.mElements[4] = sin_x;
		newMatrix.mElements[5] = cos_x;
		return newMatrix;
	}

	static Matrix4x4 CreateTranslation(Vector4 v_translation)
	{
		Matrix4x4 newMatrix = Matrix4x4();
		newMatrix.mElements[3] = v_translation.x;
		newMatrix.mElements[7] = v_translation.y;
		newMatrix.mElements[11] = v_translation.z;
		DebugOut("Tran Determinant = %f, ", newMatrix.DetRotation());
		return newMatrix;
	}

	static Matrix4x4 CreateTranslation(float x_m, float y_m, float z_m)
	{
		Matrix4x4 newMatrix = Matrix4x4();
		newMatrix.mElements[3] = x_m;
		newMatrix.mElements[7] = y_m;
		newMatrix.mElements[11] = z_m;
		DebugOut("Tran Determinant = %f, ", newMatrix.DetRotation());
		return newMatrix;
	}

	static Matrix4x4 CreateXYZUVW(float _x, float _y, float _z, float _u, float _v, float _w)
	{
		Matrix4x4 rot_X = CreateRotX(_v);
		Matrix4x4 rot_Y = CreateRotY(_w);
		Matrix4x4 rot_Z = CreateRotZ(_u);
		Matrix4x4 trans_mat = CreateTranslation(_x, _y, _z);

		return (rot_Z * rot_Y * rot_X * trans_mat);
	}

	inline Matrix4x4& operator+=(const Matrix4x4& rhs)
	{
		for (int i = 0; i < 16; i++)
		{
			this->mElements[i] += rhs.mElements[i];
		}
		return *this;
	}

	inline Matrix4x4& operator-=(const Matrix4x4& rhs)
	{
		for (int i = 0; i < 16; i++)
		{
			this->mElements[i] -= rhs.mElements[i];
		}
		return *this;
	}

	inline Matrix4x4& operator*=(const Matrix4x4& rhs)
	{
		Matrix4x4 newMatrix;
		float temp_0= this->mElements[0] * rhs.mElements[0] + this->mElements[1] * rhs.mElements[4] + this->mElements[2] * rhs.mElements[8] + this->mElements[3] * rhs.mElements[12];
		float temp_1= this->mElements[0] * rhs.mElements[1] + this->mElements[1] * rhs.mElements[5] + this->mElements[2] * rhs.mElements[9] + this->mElements[3] * rhs.mElements[13];
		float temp_2= this->mElements[0] * rhs.mElements[2] + this->mElements[1] * rhs.mElements[6] + this->mElements[2] * rhs.mElements[10] + this->mElements[3] * rhs.mElements[14];
		float temp_3= this->mElements[0] * rhs.mElements[3] + this->mElements[1] * rhs.mElements[7] + this->mElements[2] * rhs.mElements[11] + this->mElements[3] * rhs.mElements[15];
		
		float temp_4= this->mElements[4] * rhs.mElements[0] + this->mElements[5] * rhs.mElements[4] + this->mElements[6] * rhs.mElements[8] + this->mElements[7] * rhs.mElements[12];
		float temp_5= this->mElements[4] * rhs.mElements[1] + this->mElements[5] * rhs.mElements[5] + this->mElements[6] * rhs.mElements[9] + this->mElements[7] * rhs.mElements[13];
		float temp_6= this->mElements[4] * rhs.mElements[2] + this->mElements[5] * rhs.mElements[6] + this->mElements[6] * rhs.mElements[10] + this->mElements[7] * rhs.mElements[14];
		float temp_7= this->mElements[4] * rhs.mElements[3] + this->mElements[5] * rhs.mElements[7] + this->mElements[6] * rhs.mElements[11] + this->mElements[7] * rhs.mElements[15];
		
		float temp_8= this->mElements[8] * rhs.mElements[0] + this->mElements[9] * rhs.mElements[4] + this->mElements[10] * rhs.mElements[8] + this->mElements[11] * rhs.mElements[12];
		float temp_9= this->mElements[8] * rhs.mElements[1] + this->mElements[9] * rhs.mElements[5] + this->mElements[10] * rhs.mElements[9] + this->mElements[11] * rhs.mElements[13];
		float temp_10 = this->mElements[8] * rhs.mElements[2] + this->mElements[9] * rhs.mElements[6] + this->mElements[10] * rhs.mElements[10] + this->mElements[11] * rhs.mElements[14];
		float temp_11 = this->mElements[8] * rhs.mElements[3] + this->mElements[9] * rhs.mElements[7] + this->mElements[10] * rhs.mElements[11] + this->mElements[11] * rhs.mElements[15];
		
		float temp_12 = this->mElements[12] * rhs.mElements[0] + this->mElements[13] * rhs.mElements[4] + this->mElements[14] * rhs.mElements[8] + this->mElements[15] * rhs.mElements[12];
		float temp_13 = this->mElements[12] * rhs.mElements[1] + this->mElements[13] * rhs.mElements[5] + this->mElements[14] * rhs.mElements[9] + this->mElements[15] * rhs.mElements[13];
		float temp_14 = this->mElements[12] * rhs.mElements[2] + this->mElements[13] * rhs.mElements[6] + this->mElements[14] * rhs.mElements[10] + this->mElements[15] * rhs.mElements[14];
		float temp_15 = this->mElements[12] * rhs.mElements[3] + this->mElements[13] * rhs.mElements[7] + this->mElements[14] * rhs.mElements[11] + this->mElements[15] * rhs.mElements[15];
		
		this->mElements[0] =  temp_0;		this->mElements[1] =  temp_1;		this->mElements[2] =  temp_2;		this->mElements[3] =  temp_3;							  
		this->mElements[4] =  temp_4;		this->mElements[5] =  temp_5;		this->mElements[6] =  temp_6;		this->mElements[7] =  temp_7;							  
		this->mElements[8] =  temp_8;		this->mElements[9] =  temp_9;		this->mElements[10] = temp_10;		this->mElements[11] = temp_11;							  
		this->mElements[12] = temp_12;		this->mElements[13] = temp_13;		this->mElements[14] = temp_14;		this->mElements[15] = temp_15;

		return  *this;
	}

	static bool MatrixInv4x4(Matrix4x4& mat_in, Matrix4x4& mat_inv_out)
	{
		Matrix4x4 inv;
		float det;

		inv.Get(0,0) = mat_in.Get(1,1) * mat_in.Get(2,2) * mat_in.Get(3,3) -
			mat_in.Get(1,1) * mat_in.Get(2,3) * mat_in.Get(3,2) -
			mat_in.Get(2,1) * mat_in.Get(1,2) * mat_in.Get(3,3) +
			mat_in.Get(2,1) * mat_in.Get(1,3) * mat_in.Get(3,2) +
			mat_in.Get(3,1) * mat_in.Get(1,2) * mat_in.Get(2,3) -
			mat_in.Get(3,1) * mat_in.Get(1,3) * mat_in.Get(2,2);

		inv.Get(1,0) = -mat_in.Get(1,0) * mat_in.Get(2,2) * mat_in.Get(3,3) +
			mat_in.Get(1,0) * mat_in.Get(2,3) * mat_in.Get(3,2) +
			mat_in.Get(2,0) * mat_in.Get(1,2) * mat_in.Get(3,3) -
			mat_in.Get(2,0) * mat_in.Get(1,3) * mat_in.Get(3,2) -
			mat_in.Get(3,0) * mat_in.Get(1,2) * mat_in.Get(2,3) +
			mat_in.Get(3,0) * mat_in.Get(1,3) * mat_in.Get(2,2);

		inv.Get(2,0) = mat_in.Get(1,0) * mat_in.Get(2,1) * mat_in.Get(3,3) -
			mat_in.Get(1,0) * mat_in.Get(2,3) * mat_in.Get(3,1) -
			mat_in.Get(2,0) * mat_in.Get(1,1) * mat_in.Get(3,3) +
			mat_in.Get(2,0) * mat_in.Get(1,3) * mat_in.Get(3,1) +
			mat_in.Get(3,0) * mat_in.Get(1,1) * mat_in.Get(2,3) -
			mat_in.Get(3,0) * mat_in.Get(1,3) * mat_in.Get(2,1);

		inv.Get(3,0) = -mat_in.Get(1,0) * mat_in.Get(2,1) * mat_in.Get(3,2) +
			mat_in.Get(1,0) * mat_in.Get(2,2) * mat_in.Get(3,1) +
			mat_in.Get(2,0) * mat_in.Get(1,1) * mat_in.Get(3,2) -
			mat_in.Get(2,0) * mat_in.Get(1,2) * mat_in.Get(3,1) -
			mat_in.Get(3,0) * mat_in.Get(1,1) * mat_in.Get(2,2) +
			mat_in.Get(3,0) * mat_in.Get(1,2) * mat_in.Get(2,1);

		inv.Get(0,1) = -mat_in.Get(0,1) * mat_in.Get(2,2) * mat_in.Get(3,3) +
			mat_in.Get(0,1) * mat_in.Get(2,3) * mat_in.Get(3,2) +
			mat_in.Get(2,1) * mat_in.Get(0,2) * mat_in.Get(3,3) -
			mat_in.Get(2,1) * mat_in.Get(0,3) * mat_in.Get(3,2) -
			mat_in.Get(3,1) * mat_in.Get(0,2) * mat_in.Get(2,3) +
			mat_in.Get(3,1) * mat_in.Get(0,3) * mat_in.Get(2,2);

		inv.Get(1,1) = mat_in.Get(0,0) * mat_in.Get(2,2) * mat_in.Get(3,3) -
			mat_in.Get(0,0) * mat_in.Get(2,3) * mat_in.Get(3,2) -
			mat_in.Get(2,0) * mat_in.Get(0,2) * mat_in.Get(3,3) +
			mat_in.Get(2,0) * mat_in.Get(0,3) * mat_in.Get(3,2) +
			mat_in.Get(3,0) * mat_in.Get(0,2) * mat_in.Get(2,3) -
			mat_in.Get(3,0) * mat_in.Get(0,3) * mat_in.Get(2,2);

		inv.Get(2,1) = -mat_in.Get(0,0) * mat_in.Get(2,1) * mat_in.Get(3,3) +
			mat_in.Get(0,0) * mat_in.Get(2,3) * mat_in.Get(3,1) +
			mat_in.Get(2,0) * mat_in.Get(0,1) * mat_in.Get(3,3) -
			mat_in.Get(2,0) * mat_in.Get(0,3) * mat_in.Get(3,1) -
			mat_in.Get(3,0) * mat_in.Get(0,1) * mat_in.Get(2,3) +
			mat_in.Get(3,0) * mat_in.Get(0,3) * mat_in.Get(2,1);

		inv.Get(3,1) = mat_in.Get(0,0) * mat_in.Get(2,1) * mat_in.Get(3,2) -
			mat_in.Get(0,0) * mat_in.Get(2,2) * mat_in.Get(3,1) -
			mat_in.Get(2,0) * mat_in.Get(0,1) * mat_in.Get(3,2) +
			mat_in.Get(2,0) * mat_in.Get(0,2) * mat_in.Get(3,1) +
			mat_in.Get(3,0) * mat_in.Get(0,1) * mat_in.Get(2,2) -
			mat_in.Get(3,0) * mat_in.Get(0,2) * mat_in.Get(2,1);

		inv.Get(0,2) = mat_in.Get(0,1) * mat_in.Get(1,2) * mat_in.Get(3,3) -
			mat_in.Get(0,1) * mat_in.Get(1,3) * mat_in.Get(3,2) -
			mat_in.Get(1,1) * mat_in.Get(0,2) * mat_in.Get(3,3) +
			mat_in.Get(1,1) * mat_in.Get(0,3) * mat_in.Get(3,2) +
			mat_in.Get(3,1) * mat_in.Get(0,2) * mat_in.Get(1,3) -
			mat_in.Get(3,1) * mat_in.Get(0,3) * mat_in.Get(1,2);

		inv.Get(1,2) = -mat_in.Get(0,0) * mat_in.Get(1,2) * mat_in.Get(3,3) +
			mat_in.Get(0,0) * mat_in.Get(1,3) * mat_in.Get(3,2) +
			mat_in.Get(1,0) * mat_in.Get(0,2) * mat_in.Get(3,3) -
			mat_in.Get(1,0) * mat_in.Get(0,3) * mat_in.Get(3,2) -
			mat_in.Get(3,0) * mat_in.Get(0,2) * mat_in.Get(1,3) +
			mat_in.Get(3,0) * mat_in.Get(0,3) * mat_in.Get(1,2);

		inv.Get(2,2) = mat_in.Get(0,0) * mat_in.Get(1,1) * mat_in.Get(3,3) -
			mat_in.Get(0,0) * mat_in.Get(1,3) * mat_in.Get(3,1) -
			mat_in.Get(1,0) * mat_in.Get(0,1) * mat_in.Get(3,3) +
			mat_in.Get(1,0) * mat_in.Get(0,3) * mat_in.Get(3,1) +
			mat_in.Get(3,0) * mat_in.Get(0,1) * mat_in.Get(1,3) -
			mat_in.Get(3,0) * mat_in.Get(0,3) * mat_in.Get(1,1);

		inv.Get(3,2) = -mat_in.Get(0,0) * mat_in.Get(1,1) * mat_in.Get(3,2) +
			mat_in.Get(0,0) * mat_in.Get(1,2) * mat_in.Get(3,1) +
			mat_in.Get(1,0) * mat_in.Get(0,1) * mat_in.Get(3,2) -
			mat_in.Get(1,0) * mat_in.Get(0,2) * mat_in.Get(3,1) -
			mat_in.Get(3,0) * mat_in.Get(0,1) * mat_in.Get(1,2) +
			mat_in.Get(3,0) * mat_in.Get(0,2) * mat_in.Get(1,1);

		inv.Get(0,3) = -mat_in.Get(0,1) * mat_in.Get(1,2) * mat_in.Get(2,3) +
			mat_in.Get(0,1) * mat_in.Get(1,3) * mat_in.Get(2,2) +
			mat_in.Get(1,1) * mat_in.Get(0,2) * mat_in.Get(2,3) -
			mat_in.Get(1,1) * mat_in.Get(0,3) * mat_in.Get(2,2) -
			mat_in.Get(2,1) * mat_in.Get(0,2) * mat_in.Get(1,3) +
			mat_in.Get(2,1) * mat_in.Get(0,3) * mat_in.Get(1,2);

		inv.Get(1,3) = mat_in.Get(0,0) * mat_in.Get(1,2) * mat_in.Get(2,3) -
			mat_in.Get(0,0) * mat_in.Get(1,3) * mat_in.Get(2,2) -
			mat_in.Get(1,0) * mat_in.Get(0,2) * mat_in.Get(2,3) +
			mat_in.Get(1,0) * mat_in.Get(0,3) * mat_in.Get(2,2) +
			mat_in.Get(2,0) * mat_in.Get(0,2) * mat_in.Get(1,3) -
			mat_in.Get(2,0) * mat_in.Get(0,3) * mat_in.Get(1,2);

		inv.Get(2,3) = -mat_in.Get(0,0) * mat_in.Get(1,1) * mat_in.Get(2,3) +
			mat_in.Get(0,0) * mat_in.Get(1,3) * mat_in.Get(2,1) +
			mat_in.Get(1,0) * mat_in.Get(0,1) * mat_in.Get(2,3) -
			mat_in.Get(1,0) * mat_in.Get(0,3) * mat_in.Get(2,1) -
			mat_in.Get(2,0) * mat_in.Get(0,1) * mat_in.Get(1,3) +
			mat_in.Get(2,0) * mat_in.Get(0,3) * mat_in.Get(1,1);

		inv.Get(3,3) = mat_in.Get(0,0) * mat_in.Get(1,1) * mat_in.Get(2,2) -
			mat_in.Get(0,0) * mat_in.Get(1,2) * mat_in.Get(2,1) -
			mat_in.Get(1,0) * mat_in.Get(0,1) * mat_in.Get(2,2) +
			mat_in.Get(1,0) * mat_in.Get(0,2) * mat_in.Get(2,1) +
			mat_in.Get(2,0) * mat_in.Get(0,1) * mat_in.Get(1,2) -
			mat_in.Get(2,0) * mat_in.Get(0,2) * mat_in.Get(1,1);

		det = mat_in.Get(0,0) * inv.Get(0,0) + mat_in.Get(0,1) * inv.Get(1,0) + mat_in.Get(0,2) * inv.Get(2,0) + mat_in.Get(0,3) * inv.Get(3,0);

		if (det == 0)
			return false;

		det = 1.0 / det;

		for (int v_row = 0; v_row < 4; v_row++)
			for (int v_col = 0; v_col < 4; v_col++)
				mat_inv_out.Get(v_row,v_col) = inv.Get(v_row,v_col) * det;

		return true;
	}
	
	inline Vector4 operator* (const Vector4& rhs)
	{
		Matrix4x4 newMatrix;
		float temp_0 = this->mElements[0] * rhs.x + this->mElements[1] * rhs.y + this->mElements[2] * rhs.z + this->mElements[3];
		float temp_1 = this->mElements[4] * rhs.x + this->mElements[5] * rhs.y + this->mElements[6] * rhs.z + this->mElements[7];
		float temp_2 = this->mElements[8] * rhs.x + this->mElements[9] * rhs.y + this->mElements[10] * rhs.z + this->mElements[11];

		return Vector4(temp_0, temp_1, temp_2);
	}

	inline Matrix4x4& operator*=(const float& rhs)
	{
		for (int i = 0; i < 16; i++)
		{
			this->mElements[i] *= rhs;
		}
		return *this;
	}

	inline Matrix4x4 operator+(const Matrix4x4& rhs)
	{
		Matrix4x4 newMatrix = Matrix4x4(this->mElements);
		newMatrix += rhs;
		return newMatrix;
	}

	inline Matrix4x4 operator-(const Matrix4x4& rhs)
	{
		Matrix4x4 newMatrix = Matrix4x4(this->mElements);
		newMatrix -= rhs;
		return newMatrix;
	}

	inline Matrix4x4 operator*(const Matrix4x4& rhs)
	{
		Matrix4x4 newMatrix = Matrix4x4(this->mElements);
		newMatrix *= rhs;
		return newMatrix;
	}

	inline Matrix4x4 operator*(const float& rhs)
	{
		Matrix4x4 newMatrix = Matrix4x4(this->mElements);
		newMatrix *= rhs;
		return newMatrix;
	}

	static Matrix4x4 Compose(float first_about_x, float next_about_y, float last_about_z, float x, float y, float z)
	{

		float cosY = cosf(next_about_y);     // Yaw
		float sinY = sinf(next_about_y);

		float cosP = cosf(first_about_x);     // Pitch
		float sinP = sinf(first_about_x);

		float cosR = cosf(last_about_z);     // Roll
		float sinR = sinf(last_about_z);

		Matrix4x4 mat_;
		mat_.Get(0, 0) = cosY * cosR + sinY * sinP * sinR;
		mat_.Get(1, 0) = cosR * sinY * sinP - sinR * cosY;
		mat_.Get(2, 0) = cosP * sinY;

		mat_.Get(0, 1)  = cosP * sinR;
		mat_.Get(1, 1) = cosR * cosP;
		mat_.Get(2, 1) = -sinP;

		mat_.Get(0, 2) = sinR * cosY * sinP - sinY * cosR;
		mat_.Get(1, 2) = sinY * sinR + cosR * cosY * sinP;
		mat_.Get(2, 2) = cosP * cosY;

		mat_.Get(0, 3) = x;
		mat_.Get(1, 3) = y;
		mat_.Get(2, 3) = z;

		return mat_;
	}

	static Matrix4x4 Compose(Euler input_euler)
	{

		float cosY = cosf(input_euler.aY);     // Yaw
		float sinY = sinf(input_euler.aY);

		float cosP = cosf(input_euler.aX);     // Pitch
		float sinP = sinf(input_euler.aX);

		float cosR = cosf(input_euler.aZ);     // Roll
		float sinR = sinf(input_euler.aZ);

		Matrix4x4 mat_;
		mat_.Get(0, 0) = cosY * cosR + sinY * sinP * sinR;
		mat_.Get(1, 0) = cosR * sinY * sinP - sinR * cosY;
		mat_.Get(2, 0) = cosP * sinY;

		mat_.Get(0, 1)  = cosP * sinR;
		mat_.Get(1, 1) = cosR * cosP;
		mat_.Get(2, 1) = -sinP;

		mat_.Get(0, 2) = sinR * cosY * sinP - sinY * cosR;
		mat_.Get(1, 2) = sinY * sinR + cosR * cosY * sinP;
		mat_.Get(2, 2) = cosP * cosY;

		mat_.Get(0, 3) = input_euler.X;
		mat_.Get(1, 3) = input_euler.Y;
		mat_.Get(2, 3) = input_euler.Z;

		return mat_;
	}

	Vector4 GetPosition()
	{
		Vector4 v_position(mElements[3], mElements[7], mElements[11]);
		return v_position;
	}

	Vector4 GetZAxis()
	{
		Vector4 v_direction(mElements[2], mElements[6], mElements[10]);
		return v_direction;
	}

	static Euler Decompose(Matrix4x4 input_matrix)
	{
		Euler out_euler;
		out_euler.aX = asinf(-(input_matrix.Get(2, 1)));                  // Pitch
		if (cosf(out_euler.aX) > 0.0001)                 // Not at poles
		{
			out_euler.aY = atan2f(input_matrix.Get(2, 0), input_matrix.Get(2, 2));     // Yaw
			out_euler.aZ = atan2f(input_matrix.Get(0, 1), input_matrix.Get(1, 1));     // Roll
		}
		else
		{
			out_euler.aY = 0.0f;                         // Yaw
			out_euler.aZ = atan2f(-input_matrix.Get(1, 0), input_matrix.Get(0, 0));    // Roll
		}

		out_euler.X = input_matrix.Get(0, 3);
		out_euler.Y = input_matrix.Get(1, 3);
		out_euler.Z = input_matrix.Get(2, 3);

		return out_euler;
	}

	//static Matrix4x4 AboutNormalVector(float x, float y, float z)
	//{
	//
	//
	//	float length = sqrt(x*x + y*y + z*z);
	//	x /= length;
	//	y /= length;
	//	z /= length;
	//
	//	Matrix4x4 m_ret({
	//		y/sqrt(x*x+y*y), -x/sqrt(x*x+y*y), 0, 0 ,
	//		x*z/sqrt(x*x+y*y),y*z/sqrt(x*x + y*y),-sqrt(x*x + y*y),0,
	//		x,y, z, 0 ,
	//		0.0F,0.0F,0.0F,1.0F });
	//
	//	return m_ret;
	//}

	static Matrix4x4 RotateZaxisToVector(Vector4 v_vector)
	{
		Vector4 z_axis(0.0, 0.0, 1.0);
		float v_angle = z_axis.GetAngle(v_vector);
		if (v_angle < 0.0001F)
			return Matrix4x4();
		else if (v_angle > 3.1415F)
			return Matrix4x4::CreateRotZ(180)*Matrix4x4();

		Vector4 rotation_vector = v_vector.Normalize().NormCross(z_axis);
		Matrix4x4 _W({ 0, -rotation_vector.z, rotation_vector.y, 0,
			rotation_vector.z, 0, -rotation_vector.x, 0,
			-rotation_vector.y, rotation_vector.x, 0, 0,
			0, 0, 0, 1.0 });

		Matrix4x4 rot_mat = Matrix4x4() + _W * sin(v_angle) + (_W*_W)*(2 * sin(v_angle / 2)*sin(v_angle / 2));
		rot_mat.mElements[15] = 1.0F;


		return rot_mat;
	}
	
	static float Compare(const Matrix4x4& rhs, const Matrix4x4& lhs)
	{
		return Euler::Compare(Matrix4x4::Decompose(rhs), Matrix4x4::Decompose(lhs));
	}

	void Print(std::string v_name = "Matrix:")
	{
		std::cout << v_name << std::endl << std::endl;
		std::cout << std::to_string(mElements[0]).c_str() << "   " << std::to_string(mElements[1]).c_str() << "   " << std::to_string(mElements[2]).c_str() << "   " << std::to_string(mElements[3]).c_str() << "   " << std::endl;
		std::cout << std::to_string(mElements[4]).c_str() << "   " << std::to_string(mElements[5]).c_str() << "   " << std::to_string(mElements[6]).c_str() << "   " << std::to_string(mElements[7]).c_str() << "   " << std::endl;
		std::cout << std::to_string(mElements[8]).c_str() << "   " << std::to_string(mElements[9]).c_str() << "   " << std::to_string(mElements[10]).c_str() << "   " << std::to_string(mElements[11]).c_str() << "   " << std::endl;
		std::cout << std::to_string(mElements[12]).c_str() << "   " << std::to_string(mElements[13]).c_str() << "   " << std::to_string(mElements[14]).c_str() << "   " << std::to_string(mElements[15]).c_str() << "   " << std::endl << std::endl;
	}
	
	static MAT4X4EQN MatToEqn4x4(Matrix4x4 input_mat)
	{
		MAT4X4EQN eqn_mat = IDENTITYEQN;

		//Perform Matrix Multiplication
		for (int v_row = 0; v_row < 4; v_row++)
		{
			for (int v_col = 0; v_col < 4; v_col++)
			{
				eqn_mat[v_row][v_col] = Equation(input_mat.Get(v_row,v_col));
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
		for (auto& v_val : v_eqn.Vals)
		{
			if (!v_val.Zero)
			{
				//Multiply all values within Variables
				float temp_mult = 1.0F;
				for (auto& v_var : v_val.Vars)
				{
					//Get Variables Value at current angle
					temp_mult *= variable_values.at(v_var);
				}
				value_eqn += v_val.Num * temp_mult;
			}
		}
		return value_eqn;
	}

	static Matrix4x4 MatFromEqn4x4(MAT4X4EQN& input_mat, VARVALS& variable_values)
	{
		Matrix4x4 matrix_out;
		//Perform Matrix Multiplication
		for (int v_row = 0; v_row < 4; v_row++)
		{
			for (int v_col = 0; v_col < 4; v_col++)
			{
				matrix_out.Get(v_row,v_col) = SolveEquation(input_mat[v_row][v_col], variable_values);
			}
		}
		return matrix_out;
	}

	static void PrintMatrix(Matrix4x4& m_matrix, std::string v_name)
	{
		DebugOut("[--%s--] Joint Matrix\n", v_name.c_str());
		for (int v_row = 0; v_row < 4; v_row++)
		{
			DebugOut("%.2f\t %.2f\t %.2f\t %.2f\n", m_matrix.Get(v_row,0), m_matrix.Get(v_row,1), m_matrix.Get(v_row,2), m_matrix.Get(v_row,3));
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

};


struct BlankContainer 
{
	virtual ~BlankContainer() {}
	int clientWidth;
	int clientHeight;
	
	//std::map<std::string, std::shared_ptr<Matrix4x4>> camMoveMatrix;
	Matrix4x4 camMoveMatrix;
	
	template<typename K, typename V>
	static bool FindByValue(std::vector<K> & vec, std::map<K, V> mapOfElemen, V value)
	{
		bool bResult = false;
		auto it = mapOfElemen.begin();
		// Iterate through the map
		while (it != mapOfElemen.end())
		{
			// Check if value of this entry matches with given value
			if (it->second == value)
			{
				// Yes found
				bResult = true;
				// Push the key in given map
				vec.push_back(it->first);
			}
			// Go to next entry in map
			it++;
		}
		return bResult;
	}
};

struct WindowContainer
{
	HWND viewportWindow;
	int viewportWidth;
	int viewportHeight;
};

struct WindowsContainer : public BlankContainer
{
	HWND mainWindow;
	HINSTANCE hInstance;
	HINSTANCE hPrevInstance;
	std::map<std::string, std::shared_ptr<WindowContainer>> viewportWindows;
	std::map<std::string, std::shared_ptr<Direct3DRenderer>> viewportRenderer;
	std::map<std::string, std::pair<std::string,std::shared_ptr<CustomControl>>> menuItems;
	
	std::shared_ptr<WindowContainer> GetWindow(std::string window_name)
	{
		if (viewportWindows.count(window_name) == 0)
		{
			return nullptr;
		}
		return viewportWindows.at(window_name);
	}

	std::shared_ptr<Direct3DRenderer> GetRenderer(std::string window_name)
	{
		if (viewportRenderer.count(window_name) == 0)
		{
			return nullptr;
		}
		return viewportRenderer.at(window_name);
	}

	void AddWindow(std::string viewport_name, std::shared_ptr<WindowContainer> child_window)
	{
		viewportWindows.insert(std::map<std::string, std::shared_ptr<WindowContainer>>::value_type(viewport_name, child_window));
	}

	void AddRenderer(std::string viewport_name, std::shared_ptr<Direct3DRenderer> child_renderer)
	{
		viewportRenderer.insert(std::map<std::string, std::shared_ptr<Direct3DRenderer>>::value_type(viewport_name, child_renderer));
	}

};

struct EpsonPoint
{
	std::string label = "";
	float rX = 0, rY = 0, rZ = 0, rU = 0, rV = 0, rW = 0;
	int local;
	int number;
	Matrix4x4 point_world_matrix;
};

struct EpsonCall
{
	std::string function_call = "";
	std::string point_call = "";
	std::string tool_call = "";
	std::string point_array_call = "";
	float speed = 0;
	float accel = 0;
	float wait = 0;
};

struct EpsonPath
{
	std::string function_name = "";
	std::vector<EpsonCall> call_stack;

	std::map<std::string, std::vector<std::string>> point_arrays;

	void Append(EpsonPath new_path)
	{
		call_stack.insert(call_stack.end(), new_path.call_stack.begin(), new_path.call_stack.end());
	}
};

typedef enum Keyboard_Device
{
	DIK_F1              =0x3B,
	DIK_F2              =0x3C,
	DIK_F3              =0x3D,
	DIK_F4              =0x3E,
	DIK_F5              =0x3F,
	DIK_F6              =0x40,
	DIK_F7              =0x41,
	DIK_F8              =0x42,
	DIK_F9              =0x43,
	DIK_F10             =0x44,
	DIK_F11             =0x57,
	DIK_F12             =0x58,
	DIK_F13             =0x64,    /*                     (NEC PC98) */
	DIK_F14             =0x65,    /*                     (NEC PC98) */
	DIK_F15             =0x66,    /*                     (NEC PC98) */
	DIK_0               =0x0B,
	DIK_1               =0x02,
	DIK_2               =0x03,
	DIK_3               =0x04,
	DIK_4               =0x05,
	DIK_5               =0x06,
	DIK_6               =0x07,
	DIK_7               =0x08,
	DIK_8               =0x09,
	DIK_9               =0x0A,
	DIK_NUMPAD0         =0x52,
	DIK_NUMPAD1         =0x4F,
	DIK_NUMPAD2         =0x50,
	DIK_NUMPAD3         =0x51,
	DIK_NUMPAD4         =0x4B,
	DIK_NUMPAD5         =0x4C,
	DIK_NUMPAD6         =0x4D,
	DIK_NUMPAD7         =0x47,
	DIK_NUMPAD8         =0x48,
	DIK_NUMPAD9         =0x49,
	DIK_A               =0x1E,
	DIK_B               =0x30,
	DIK_C               =0x2E,
	DIK_D               =0x20,
	DIK_E               =0x12,
	DIK_F               =0x21,
	DIK_G               =0x22,
	DIK_H               =0x23,
	DIK_I               =0x17,
	DIK_J               =0x24,
	DIK_K               =0x25,
	DIK_L               =0x26,
	DIK_M               =0x32,
	DIK_N               =0x31,
	DIK_O               =0x18,
	DIK_P               =0x19,
	DIK_Q               =0x10,
	DIK_R               =0x13,
	DIK_S               =0x1F,
	DIK_T               =0x14,
	DIK_U               =0x16,
	DIK_V               =0x2F,
	DIK_W               =0x11,
	DIK_X               =0x2D,
	DIK_Y               =0x15,
	DIK_Z               =0x2C,
	DIK_ESCAPE          =0x01,
	DIK_TAB             =0x0F,
	DIK_LSHIFT          =0x2A,
	DIK_RSHIFT          =0x36,
	DIK_LCONTROL        =0x1D,
	DIK_RCONTROL        =0x9D,
	DIK_BACK            =0x0E,    /* backspace */
	DIK_RETURN          =0x1C,    /* Enter on main keyboard */
	DIK_NUMPADENTER     =0x9C,    /* Enter on numeric keypad */
	DIK_LMENU           =0x38,    /* left Alt */
	DIK_SPACE           =0x39,
	DIK_CAPITAL         =0x3A,    /* capslock */
	DIK_NUMLOCK         =0x45,
	DIK_SCROLL          =0x46,    /* Scroll Lock */
	DIK_RMENU           =0xB8,    /* right Alt */
	DIK_AT              =0x91,     /*                     (NEC PC98) */
	DIK_COLON           =0x92,     /*                     (NEC PC98) */
	DIK_UNDERLINE       =0x93,     /*                     (NEC PC98) */
	DIK_MINUS           =0x0C,     /* - on main keyboard */
	DIK_EQUALS          =0x0D, 
	DIK_LBRACKET        =0x1A, 
	DIK_RBRACKET        =0x1B, 
	DIK_SEMICOLON       =0x27, 
	DIK_APOSTROPHE      =0x28, 
	DIK_GRAVE           =0x29,     /* accent grave */
	DIK_BACKSLASH       =0x2B, 
	DIK_COMMA           =0x33, 
	DIK_PERIOD          =0x34,     /* . on main keyboard */
	DIK_SLASH           =0x35,     /* / on main keyboard */
	DIK_MULTIPLY        =0x37,     /* * on numeric keypad */
	DIK_SUBTRACT        =0x4A,    /* - on numeric keypad */
	DIK_ADD             =0x4E,    /* + on numeric keypad */
	DIK_DECIMAL         =0x53,    /* . on numeric keypad */
	DIK_NUMPADEQUALS    =0x8D,    /* = on numeric keypad (NEC PC98) */
	DIK_NUMPADCOMMA     =0xB3,    /* , on numeric keypad (NEC PC98) */
	DIK_NEXTTRACK       =0x99,    /* Next Track */
	DIK_MUTE            =0xA0,    /* Mute */
	DIK_CALCULATOR      =0xA1,    /* Calculator */
	DIK_PLAYPAUSE       =0xA2,    /* Play / Pause */
	DIK_MEDIASTOP       =0xA4,    /* Media Stop */
	DIK_VOLUMEDOWN      =0xAE,    /* Volume - */
	DIK_VOLUMEUP        =0xB0,    /* Volume + */
	DIK_WEBHOME         =0xB2,    /* Web home */
	DIK_SYSRQ           =0xB7,
	DIK_PAUSE           =0xC5,    /* Pause */
	DIK_APPS            =0xDD,    /* AppMenu key */
	DIK_POWER           =0xDE,    /* System Power */
	DIK_SLEEP           =0xDF,    /* System Sleep */
	DIK_WAKE            =0xE3,    /* System Wake */
	DIK_WEBSEARCH       =0xE5,    /* Web Search */
	DIK_WEBFAVORITES    =0xE6,    /* Web Favorites */
	DIK_WEBREFRESH      =0xE7,    /* Web Refresh */
	DIK_WEBSTOP         =0xE8,    /* Web Stop */
	DIK_WEBFORWARD      =0xE9,    /* Web Forward */
	DIK_WEBBACK         =0xEA,    /* Web Back */
	DIK_MYCOMPUTER      =0xEB,    /* My Computer */
	DIK_MAIL            =0xEC,    /* Mail */
	DIK_MEDIASELECT     =0xED,    /* Media Select */
	DIK_HOME            =0xC7,    /* Home on arrow keypad */
	DIK_UP              =0xC8,    /* UpArrow on arrow keypad */
	DIK_PRIOR           =0xC9,    /* PgUp on arrow keypad */
	DIK_LEFT            =0xCB,    /* LeftArrow on arrow keypad */
	DIK_RIGHT           =0xCD,    /* RightArrow on arrow keypad */
	DIK_END             =0xCF,    /* End on arrow keypad */
	DIK_DOWN            =0xD0,    /* DownArrow on arrow keypad */
	DIK_NEXT            =0xD1,    /* PgDn on arrow keypad */
	DIK_INSERT          =0xD2,    /* Insert on arrow keypad */
	DIK_DELETE          =0xD3,    /* Delete on arrow keypad */
	DIK_LWIN            =0xDB,    /* Left Windows key */
	DIK_RWIN            =0xDC    /* Right Windows key */
};


namespace CallbackData
{
	struct BlankData
	{
		virtual ~BlankData() {};
	}; 
	
	struct Float : public BlankData
	{
		Float(float v_number)
		{
			mNumber = v_number;
		}
		float mNumber;
	};

	struct Int : public BlankData
	{
		Int(int v_number)
		{
			mNumber = v_number;
		}
		int mNumber;
	};


	struct Text : public BlankData
	{
		Text(std::string v_name, std::string v_text)
		{
			itemName = v_name;
			itemText = v_text;
		}
		std::string itemName;
		std::string itemText;
	};

	struct Matrix : public BlankData
	{
		Matrix(std::string v_name, std::string renderer_name, std::string viewport_name, Matrix4x4 v_matrix)
		{
			itemName = v_name;
			rendererName = renderer_name;
			viewportName = viewport_name;
			mMatrix = v_matrix;
		}
		std::string itemName;
		std::string rendererName;
		std::string viewportName;
		Matrix4x4 mMatrix;
	};

	struct Mouse : public BlankData
	{
		Mouse(int pos_x, int pos_y, int wheel_z, int first_click)
		{
			posX = pos_x;
			posY = pos_y;
			wheelZ = wheel_z;
			firstClick = first_click;
		}
		int posX;
		int posY;
		int wheelZ;
		int firstClick;
	};

	struct ComboBoxEntry : public BlankData
	{
		ComboBoxEntry(std::string v_name, std::string v_text)
		{
			comboName = v_name;
			comboText = v_text;
		}
		std::string comboName;
		std::string comboText;
	};

	struct ComboBoxList : public BlankData
	{
		ComboBoxList(std::string v_name, std::vector<std::wstring> v_items)
		{
			comboName = v_name;
			comboItems = v_items;
		}
		ComboBoxList(std::string v_name)
		{
			comboName = v_name;
		}
		std::string comboName;
		std::vector<std::wstring> comboItems;
	};

	struct Button : public BlankData
	{
		Button(std::string v_name)
		{
			buttonName = v_name;
		}
		std::string buttonName;
	};

	struct CheckBox : public BlankData
	{
		CheckBox(std::string v_name, bool v_checked)
		{
			cboxName = v_name;
			mChecked = v_checked;
		}
		std::string cboxName;
		bool mChecked;
	};

	struct Slider : public BlankData
	{
		Slider(std::string v_name, float v_position)
		{
			sliderName = v_name;
			sliderPosition = v_position;
		}
		std::string sliderName;
		float sliderPosition;
	};

	struct Keyboard : public BlankData
	{
		Keyboard(uint8_t* kb_data)
		{
			kbData = kb_data;
		}
		uint8_t* kbData;
	};

	struct Frame : public BlankData
	{
		Frame(Matrix4x4& mat_data) : 
			matData(mat_data)
		{
		}
		Matrix4x4& matData;
		int frameCounter;
		long long elapsedTime;
		bool playButtonClicked;
	};


	struct Target : public BlankData
	{
		Target(std::string m_name)
		{
			modelName = m_name;
			textureName = m_name;
		}
		std::string modelName;
		std::string textureName;
	};

	struct ModelCtr : public BlankData
	{
		ModelCtr(std::string m_name, std::string r_name)
		{
			modelName = m_name;
			rendererName = r_name;
		}
		std::string modelName;
		std::string rendererName;
		std::shared_ptr<IModel> iModel;
	};

	struct ModelVecCtr : public BlankData
	{
		ModelVecCtr(std::string r_name)
		{
			rendererName = r_name;
		}
		std::string rendererName;
		std::vector<std::shared_ptr<IModel>> iModels;
	};

	struct MechanismData : public BlankData
	{
		MechanismData(std::shared_ptr<LdrMechanisms> v_mechanisms)
		{
			mMechanisms = v_mechanisms;
		}
		std::shared_ptr<LdrMechanisms> mMechanisms;
	};

	struct JointMotion : public BlankData
	{
		JointMotion(std::string mechanism_name, std::string chain_name, std::string joint_name, float v_number, bool prismatic_joint)
		{
			mechanismName = mechanism_name;
			chainName = chain_name;
			jointName = joint_name;
			mNumber = v_number;
			prismaticJoint = prismatic_joint;
		}
		std::string mechanismName;
		std::string chainName;
		std::string jointName;
		float mNumber;
		bool prismaticJoint;
	};
	
	struct EEMotion : public BlankData
	{
		EEMotion(std::string mechanism_name, std::string chain_name, float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot)
		{
			mechanismName = mechanism_name;
			chainName = chain_name;
			xPos = x_pos;
			yPos = y_pos;
			zPos = z_pos;
			xRot = x_rot;
			yRot = y_rot;
			zRot = z_rot;
		}
		std::string mechanismName;
		std::string chainName; 
		float xPos, yPos, zPos, xRot, yRot, zRot;
	};
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

class Connection
{
public:
	virtual ~Connection() {};

	virtual bool ConnectPort(const char* com_port, int baud_rate) { return true; }
	virtual bool DisconnectPort() { return true; }
	virtual bool WriteByte(int device_id, std::string servo_name, int v_addr, uint8_t v_value) { return true; }
	virtual bool WriteBytes(int device_id, std::string servo_name, int v_addr, uint32_t v_value, bool only_2_bytes = false) { return true; }
	virtual bool ReadByte(int device_id, std::string servo_name, int v_addr, uint8_t& v_value) { return true; }
	virtual bool ReadBytes(int device_id, std::string servo_name, int v_addr, uint32_t& v_value, bool only_2_byte = false) { return true; }
	virtual void CommandDelay(uint16_t ms_delay) { ; }

	int iD;
};

