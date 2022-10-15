#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "../Containers.h"

class Serializer
{
public:
	explicit Serializer(Returns& e_returns, std::string object_name = "");
	~Serializer();

	bool LoadParameter(std::string v_name, std::string& v_value);
	bool SaveParameter(std::string v_name, std::string v_value);

	bool LoadInt(std::string v_name, int& v_value);
	bool SaveInt(std::string v_name, int v_value);

	bool LoadFloat(std::string v_name, float& v_value);
	bool SaveFloat(std::string v_name, float v_value);

	bool RenameFile(std::string object_name);
	bool ClearFile();

private:
	Returns& eReturns;

	std::string fileName = "";
	std::fstream setupFile;
};

