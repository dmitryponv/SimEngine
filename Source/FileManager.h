#pragma once
#include "FileSystem\PointReader.h"
#include "FileSystem\PathReader.h"
#include "FileSystem\PathBuilder.h"

#include "Includes/enumser.h"

class FileManager
{
public:
	FileManager(CALLBACKS& v_callbacks);
	~FileManager();
	
	std::vector<std::string> GetTargetNames();
	std::vector<std::string> GetComPorts();
private:
	//PointReader mPointReader;
	//PathReader mPathReader;
	//PathBuilder mPathBuilder;

	CALLBACKS& mCallbacks;

	//std::map<std::string, EpsonPoint> epsonPoints;
	//std::map<std::string, EpsonPath> epsonPaths;


	CEnumerateSerial::CPortsArray comPorts;
	CEnumerateSerial::CNamesArray comNames;
};

