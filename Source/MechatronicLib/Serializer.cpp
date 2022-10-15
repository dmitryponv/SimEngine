#include "Serializer.h"


Serializer::Serializer(Returns& e_returns, std::string object_name) : eReturns(e_returns)
{
	if (object_name != "")
	{
		fileName = "setup\\" + object_name;
		//Create file if it doesnt exist
		setupFile.open(fileName, std::fstream::in | std::fstream::out | std::fstream::app);
		setupFile.close();
	}
}


Serializer::~Serializer()
{
}

bool Serializer::LoadParameter(std::string v_name, std::string & v_value)
{
	v_value = "";
	if (fileName == "")
	{
		eReturns.Throw(fileName, "No filename for serializer");
		return false;
	}

	std::string v_line;

	setupFile.open(fileName, std::fstream::in);
	if (!setupFile)
	{
		setupFile.close();
		eReturns.Throw(fileName, "Could not open serializer file");
		return false;
	}

	while (std::getline(setupFile, v_line))
	{
		std::istringstream v_iss(v_line);
		std::string v_a, v_b;
		if (!(v_iss >> v_a >> v_b))
		{
			setupFile.close();
			eReturns.Throw(fileName, "Could not write to serializer file");
			return false;
		} // error
		if (v_a == v_name)
		{
			v_value = v_b;
			setupFile.close();
			return true;
		}
	}
	setupFile.close();
	return false;
}

bool Serializer::SaveParameter(std::string v_name, std::string v_value)
{
	setupFile.open(fileName, std::fstream::in);
	//if (!setupFile)
	//{
	//	setupFile.close();
	//	eReturns.Throw(fileName, "Could not open serializer file");
	//	return false;
	//}

	if (setupFile) //If the file exists, read first, then write to it
	{
		std::vector<std::string> v_lines;

		std::string v_line;
		while (std::getline(setupFile, v_line))
		{
			std::istringstream v_iss(v_line);
			std::string v_a, v_b;
			if (!(v_iss >> v_a >> v_b))
			{
				//Only read lines containing two strings
				eReturns.Throw(fileName, "Serializer file has some invalid parameters");
				continue;
			} // error
			v_lines.push_back(v_line);
		}
		setupFile.close();

		bool b_exists = false;
		for (auto& v_line : v_lines)
		{
			std::istringstream v_iss(v_line);
			std::string v_a, v_b;
			if (!(v_iss >> v_a >> v_b))
			{
				eReturns.Throw(fileName, "Serializer file has some invalid parameters");
				continue;
			} // error
			if (v_a == v_name)
			{
				b_exists = true;
				v_line = v_name + " " + v_value;
			}
		}
		if (!b_exists)
			v_lines.push_back(v_name + " " + v_value);

		setupFile.open(fileName, std::fstream::out);
		for (auto& v_line : v_lines)
		{
			setupFile << v_line << std::endl;
		}
		setupFile.close();
	}
	else //If file doesn't exist, create then write
	{
		setupFile.open(fileName, std::fstream::out);
		std::string v_line = v_name + " " + v_value;
		setupFile << v_line << std::endl;
		setupFile.close();
	}
	return true;
}

bool Serializer::LoadInt(std::string v_name, int& v_value)
{
	std::string load_string;
	LoadParameter(v_name, load_string);
	if (load_string == "")
	{
		v_value = 0;
		return false;
	}
	std::istringstream s_s(load_string);
	s_s >> v_value;
	return true;
}

bool Serializer::SaveInt(std::string v_name, int v_value)
{
	return SaveParameter(v_name, std::to_string(v_value));
}


bool Serializer::LoadFloat(std::string v_name, float& v_value)
{
	std::string load_string;
	LoadParameter(v_name, load_string);
	if (load_string == "")
	{
		v_value = 0.0F;
		return false;
	}
	std::istringstream s_s(load_string);
	s_s >> v_value;
	return true;
}

bool Serializer::SaveFloat(std::string v_name, float v_value)
{
	return SaveParameter(v_name, std::to_string(v_value));
}

bool Serializer::RenameFile(std::string object_name)
{
	object_name = "setup\\" + object_name;
	if (fileName == object_name)
		return true;

	bool rc = true;
	if(fileName != "")
		rc = std::rename(fileName.c_str(), object_name.c_str());
	fileName = object_name;
	
	if(rc)
		eReturns.Throw(fileName, "Serializer file has some invalid parameters");
	return rc;
}

bool Serializer::ClearFile()
{
	//return fclose(fopen("file.txt", "w"));
	setupFile.open(fileName, std::ofstream::out | std::ofstream::trunc);
	setupFile.close();
	return true;
}
