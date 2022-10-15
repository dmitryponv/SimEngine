#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>  
#include "../Containers.h"

class PathReader
{
public:
	PathReader();

	std::map<std::string, EpsonPath> ParsePathFile(std::string input_file);
	~PathReader();


	EpsonPath ExtractPaths(std::map<std::string, EpsonPath> input_paths, std::string starting_path_name);

//private:
	//EpsonPath CombinePaths(std::map<std::string, EpsonPath> input_paths, std::string current_path_name);

};