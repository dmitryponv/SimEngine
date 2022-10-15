#include "PointReader.h"

PointReader::PointReader()
{
}

std::map<std::string, EpsonPoint> PointReader::ParsePointFile(std::string input_file)
{
	std::map<std::string, EpsonPoint> epson_points;
	std::ifstream cFile(input_file);
	if (cFile.is_open())
	{
		std::string line;
		bool point_start = false;
		EpsonPoint new_point;
		while (getline(cFile, line)) {
			line.erase(std::remove_if(line.begin(), line.end(), isspace),line.end());
			if (line[0] == '\'' || line.empty())  //Comment or empty space
				continue;
						

			if (line[0] == '}') //Point end
			{
				epson_points.insert(std::map<std::string, EpsonPoint>::value_type(new_point.label, new_point));
				point_start = false;
			}
			if (point_start)
			{
				int index = epson_points.size() - 1;
				auto delimiterPos = line.find("=");
				auto name = line.substr(0, delimiterPos);
				auto value = line.substr(delimiterPos + 1);
				if (name == "nNumber")
					sscanf(value.c_str(), "%d", &new_point.number);
				if (name == "nLocal")
					sscanf(value.c_str(), "%d", &new_point.local);
				if (name == "sLabel")
					if(value.length() > 2)
						new_point.label = value.substr(1, value.length()-2);
				if (name == "rX")
					sscanf(value.c_str(), "%f", &new_point.rX);
				if (name == "rY")
					sscanf(value.c_str(), "%f", &new_point.rY);
				if (name == "rZ")
					sscanf(value.c_str(), "%f", &new_point.rZ);
				if (name == "rU")
					sscanf(value.c_str(), "%f", &new_point.rU);
				if (name == "rV")
					sscanf(value.c_str(), "%f", &new_point.rV);
				if (name == "rW")
					sscanf(value.c_str(), "%f", &new_point.rW);
			}
			if (line[line.length() - 1] == '{') //Point start
			{
				point_start = true;
			}
		}

	}
	else {
		std::cerr << "Couldn't open config file for reading.\n";
	}
	return epson_points;
}

PointReader::~PointReader()
{
}
