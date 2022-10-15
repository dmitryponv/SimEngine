#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "../Containers.h"

class ToolReader
{
public:
	ToolReader();

	std::map<std::string, EpsonPoint> ParseToolFile(std::string input_file);

	~ToolReader();
};