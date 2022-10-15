#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "../Containers.h"

class PointReader
{
public:
	PointReader();

	std::map<std::string, EpsonPoint> ParsePointFile(std::string input_file);

	~PointReader();
};