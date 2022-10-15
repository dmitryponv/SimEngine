#pragma once
#include "../Containers.h"

#define TOOL_ROT (-55.0F)
#define ROT_OFFSET 0

class PathBuilder
{
public:
	PathBuilder();
	~PathBuilder();

	std::vector<EpsonPoint> CreateDiscretePath(EpsonPath current_path, std::map<std::string, EpsonPoint> input_points, float time_step);

	bool GetPointWorldMatrix(EpsonPoint& current_point,	Matrix4x4 curr_tool_transform);

private:

	Matrix4x4 TOOL_HT_FRONT = Matrix4x4();

	Matrix4x4 TOOL_HT_LEFT = Matrix4x4::CreateRotZ(-TOOL_ROT);

	Matrix4x4 TOOL_HT_RIGHT = Matrix4x4::CreateRotZ(TOOL_ROT);

	Matrix4x4 LOCALS[9] =
	{
		Matrix4x4(), //0
		Matrix4x4::CreateXYZUVW(-716,-108,156, -135,0,-180), //1
		Matrix4x4::CreateXYZUVW(0,0,0,0,0,0), //2
		Matrix4x4::CreateXYZUVW(0,0,0,0,0,0), //3
		Matrix4x4::CreateXYZUVW(0,0,0,0,0,0), //4
		Matrix4x4::CreateXYZUVW(0,0,0,0,0,0), //5 
		Matrix4x4::CreateXYZUVW(0,0,0,0,0,0), //6 
		Matrix4x4::CreateXYZUVW(0,0,0,0,0,0), //7 
		Matrix4x4::CreateXYZUVW(0,0,0,0,0,0)  //8
	};
};

