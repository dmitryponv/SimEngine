#include "PathReader.h"

PathReader::PathReader()
{
}

std::map<std::string, EpsonPath> PathReader::ParsePathFile(std::string input_file)
{
	std::map<std::string, EpsonPath> epson_paths;
	std::ifstream cFile(input_file);
	if (cFile.is_open())
	{
		std::string line;
		bool function_start = false;
		EpsonPath new_path;

		std::string current_path_array = "";
		float current_path_array_wait = 0.0F;
		while (getline(cFile, line)) {
			//line.erase(std::remove_if(line.begin(), line.end(), isspace),
			//	line.end());
			if (line[0] == '\'' || line.empty())  //Comment or empty space
				continue;
						

			if (line.substr(0, 4) == "Fend") //Function start
			{
				epson_paths.insert(std::map<std::string, EpsonPath>::value_type(new_path.function_name, new_path));
				new_path = EpsonPath();
				function_start = false;
			}

			if (function_start)
			{
				//Remove qutation marks
				line.erase(std::remove(line.begin(), line.end(), '\"'), line.end());
				//Remove leading and trailing white space and tabs
				///line.erase(line.begin(), std::find_if(line.begin(), line.end(), std::bind1st(std::not_equal_to<char>(), ' ')));
				///line.erase(std::find_if(line.rbegin(), line.rend(), std::bind1st(std::not_equal_to<char>(), ' ')).base(), line.end());
				///line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
				auto start = line.find_first_not_of(' ');
				auto end = line.find_last_not_of(' ');
				line = line.substr(start, (end - start) + 1);
				line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());

				//Split by space, comma, and parentheses
				std::vector<std::string> split_string;
				std::stringstream s_s_1(line);
				std::string tok_1, tok_2, tok_3, tok_4;

				while (getline(s_s_1, tok_1, ' ')) {
					std::stringstream s_s_2(tok_1);
					while (getline(s_s_2, tok_2, ',')) {
						std::stringstream s_s_3(tok_2);
						while (getline(s_s_3, tok_3, '(')) {
							std::stringstream s_s_4(tok_3);
							while (getline(s_s_4, tok_4, ')')) {
								split_string.push_back(tok_4);
							}
						}
					}
				}

				EpsonCall new_call;
				if (split_string.size() == 2 && split_string[0] == "Call") // Function call
					new_call.function_call = split_string[1];

				if (tok_2.length() > 1 && split_string[0] != "SetSpeed" && split_string[0] != "SetAccel")
					if (tok_2[tok_2.length() - 1] == ')' && split_string.size() == 2) // Function call
						new_call.function_call = split_string[0];

				if ((split_string.size() == 2 || split_string.size() == 3) && split_string[0] == "Go") // Point call
					new_call.point_call = split_string[1];

				if (split_string.size() == 2 && split_string[0] == "Tool") // Tool call
					new_call.tool_call = split_string[1];

				else if (split_string.size() >= 2 && (split_string[0] == "SetSpeed" || split_string[0] == "Speed")) // Set Speed
					sscanf(split_string[1].c_str(), "%f", &new_call.speed);

				else if (split_string.size() >= 2 && (split_string[0] == "SetAccel" || split_string[0] == "Accel")) // Set Accel
					sscanf(split_string[1].c_str(), "%f", &new_call.accel);

				else if (split_string.size() == 2 && split_string[0] == "Wait" && current_path_array == "") // Set Wait
				{
					sscanf(split_string[1].c_str(), "%f", &new_call.wait);
					if (new_call.wait == 0.0F)
						new_call.wait = 0.5F;
				}
				else if ((split_string.size() > 2 && split_string[0][split_string[0].length() - 1] == '$')) // Multiple Points array;
				{
					std::vector<std::string> pts_vector;
					for (int i = 2; i < split_string.size(); i++)
						pts_vector.push_back(split_string[i]);
					new_path.point_arrays.insert(std::map<std::string, std::vector<std::string>>::value_type(split_string[0], pts_vector));
				}

				else if (split_string.size() == 4 && split_string[0] == "ParseStr") // Rename point array call
				{
					if (new_path.point_arrays.find(split_string[1]) != new_path.point_arrays.end())
					{
						std::map<std::string, std::vector<std::string>>::iterator it = new_path.point_arrays.find(split_string[1]);
						if (it != new_path.point_arrays.end())
						{
							auto value = it->second;
							new_path.point_arrays.erase(it);
							new_path.point_arrays[split_string[2]] = value;
						}
					}
				}
				
				//Parsing the FOR loop
				else if (split_string.size() > 4 && split_string[0] == "Go" && split_string[3][split_string[3].length() - 1] == '$') // Pointarray convert to Go Calls
				{
					current_path_array = split_string[3];
				}
				else if (split_string.size() > 1 && split_string[0] == "Wait" && current_path_array != "")
				{
					sscanf(split_string[1].c_str(), "%f", &current_path_array_wait);
				}
				else if (split_string.size() > 0 && split_string[0] == "Next" && current_path_array != "")
				{
					new_call.point_array_call = current_path_array;
					//Add the point array calls as individual GO Calls
					if (new_path.point_arrays.find(new_call.point_array_call) != new_path.point_arrays.end())
					{
						std::vector<std::string> point_array = new_path.point_arrays.at(new_call.point_array_call);
						for (auto& go_call : point_array)
						{
							EpsonCall new_pt_call;
							new_pt_call.point_call = go_call;
							new_path.call_stack.push_back(new_pt_call);  //TODO: parse wait delays

							if (current_path_array_wait > 0.0F) // Wati statement in the for loop if present
							{
								EpsonCall new_wait_call;
								new_pt_call.wait = current_path_array_wait;
								new_path.call_stack.push_back(new_wait_call);
							}
						}
					}
					current_path_array = "";
					current_path_array_wait = 0.0F;
				}

				//Add call to stack
				if (new_call.function_call != "" || new_call.point_call != "" || new_call.tool_call != "" || /*new_call.point_array_call != "" ||*/ new_call.speed != 0 || new_call.accel != 0 || new_call.wait != 0)
					new_path.call_stack.push_back(new_call);
			}
			if (line.substr(0, 8) == "Function") //Function start
			{
				function_start = true;
				new_path.function_name = line.substr(9, line.length() - 1);
			}
		}

	}
	else {
		std::cerr << "Couldn't open config file for reading.\n";
	}
	return epson_paths;
}

//EpsonPath PathReader::CombinePaths(std::map<std::string, EpsonPath> input_paths, std::string starting_path_name)
//{
//	EpsonPath new_path;
//
//	if (input_paths.find(starting_path_name) != input_paths.end())
//	{
//		EpsonPath starting_path = input_paths.at(starting_path_name);
//		new_path = ExtractPaths(input_paths, starting_path.function_name);
//	}
//
//	return new_path;
//}

EpsonPath PathReader::ExtractPaths(std::map<std::string, EpsonPath> input_paths, std::string current_path_name)
{
	EpsonPath new_path;

	if (input_paths.find(current_path_name) != input_paths.end())
	{
		EpsonPath current_path = input_paths.at(current_path_name);
		for (auto& v_call : current_path.call_stack)
		{
			if (v_call.point_call != "")
				new_path.call_stack.push_back(v_call);
			else if (v_call.tool_call != "")
				new_path.call_stack.push_back(v_call);
			else if (v_call.point_array_call != "")
				new_path.call_stack.push_back(v_call);
			else if (v_call.speed != 0)
				new_path.call_stack.push_back(v_call);
			else if (v_call.accel != 0)
				new_path.call_stack.push_back(v_call);
			else if (v_call.wait != 0)
				new_path.call_stack.push_back(v_call);
			else if (v_call.function_call != "")
				if (input_paths.find(v_call.function_call) != input_paths.end())
				{
					EpsonPath next_path = input_paths.at(v_call.function_call);
					new_path.Append(ExtractPaths(input_paths, v_call.function_call));
				}
		}
	}

	return new_path;
}

PathReader::~PathReader()
{
}
