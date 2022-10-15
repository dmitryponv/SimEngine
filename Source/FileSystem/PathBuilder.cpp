#include "PathBuilder.h"



PathBuilder::PathBuilder()
{
}


PathBuilder::~PathBuilder()
{
}

std::vector<EpsonPoint> PathBuilder::CreateDiscretePath(EpsonPath current_path, std::map<std::string, EpsonPoint> input_points, float time_step)
{
	std::vector<EpsonPoint> cont_points;
	EpsonPoint previous_point;
	float curr_accel = 1;
	float curr_speed = 1;
	Matrix4x4 curr_tool_transform;
	for (auto& v_call : current_path.call_stack)
	{
		if (v_call.speed > 0.0F)
		{
			curr_speed = v_call.speed;
		}
		if (v_call.accel > 0.0F)
		{
			curr_accel = v_call.accel;
		}
		if (v_call.tool_call != "")
		{
			if(v_call.tool_call == "TOOL_HT_FRONT")
				curr_tool_transform = TOOL_HT_FRONT;
			else if (v_call.tool_call == "TOOL_HT_LEFT")
				curr_tool_transform = TOOL_HT_LEFT;
			else if (v_call.tool_call == "TOOL_HT_RIGHT")
				curr_tool_transform = TOOL_HT_RIGHT;
		}

		if (v_call.point_call != "")
		{
			std::string current_point_name = v_call.point_call;

			if (input_points.find(current_point_name) != input_points.end())
			{
				EpsonPoint current_point = input_points.at(current_point_name);
				
				if (previous_point.label != "")
				{
					float distance_between_points = sqrt(pow(current_point.rX - previous_point.rX, 2) + pow(current_point.rY - previous_point.rY, 2) + pow(current_point.rZ - previous_point.rZ, 2)
					+ pow(current_point.rU - previous_point.rU, 2) + pow(current_point.rV - previous_point.rV, 2) + pow(current_point.rW - previous_point.rW, 2));

					float time_to_reach = sqrt(2.0F * distance_between_points / curr_accel); //TODO: add max velocity
					float i_step_size = time_step / time_to_reach;
					for (float i = 0.0F; i <= 1.0F; i += i_step_size)
					{
						EpsonPoint temp_point;
						temp_point.rX = previous_point.rX + i * (current_point.rX - previous_point.rX);
						temp_point.rY = previous_point.rY + i * (current_point.rY - previous_point.rY);
						temp_point.rZ = previous_point.rZ + i * (current_point.rZ - previous_point.rZ);
						temp_point.rU = previous_point.rU + i * (current_point.rU - previous_point.rU);
						temp_point.rV = previous_point.rV + i * (current_point.rV - previous_point.rV);
						temp_point.rW = previous_point.rW + i * (current_point.rW - previous_point.rW);
						temp_point.local = current_point.local;

						GetPointWorldMatrix(temp_point, curr_tool_transform);

						cont_points.push_back(temp_point);
					}
				}
				previous_point = current_point;
			}
			else
				return std::vector<EpsonPoint>(); //Should not have any missing points
		}

		if (v_call.wait > 0.0F)
		{
			for (int i = 0; i < v_call.wait; i += time_step)
			{
				cont_points.push_back(previous_point);
			}
		}

	}

	return cont_points;
}

bool PathBuilder::GetPointWorldMatrix(EpsonPoint & current_point, Matrix4x4 curr_tool_transform)
{
	Matrix4x4 point_transform = Matrix4x4::CreateXYZUVW(current_point.rX, current_point.rY, current_point.rZ, current_point.rU, current_point.rV, current_point.rW);
	current_point.point_world_matrix = point_transform * curr_tool_transform * LOCALS[current_point.local];
	return true;
}
