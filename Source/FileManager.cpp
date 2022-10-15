#include "FileManager.h"



FileManager::FileManager(CALLBACKS& v_callbacks):
	mCallbacks(v_callbacks)
{
}


FileManager::~FileManager()
{
}

std::vector<std::string> FileManager::GetComPorts()
{
	std::vector<std::string> com_ports;
	//Update COM port selection combobox with active ports
	if (CEnumerateSerial::UsingCreateFile(comPorts))
	{
		for (int i = 0; i < comPorts.m_nSize; i++)
		{
			char v_temp[6];
			sprintf_s(v_temp, 6, "COM%u", comPorts[i]);
			com_ports.push_back(v_temp);
		}
	}
	return com_ports;
}

std::vector<std::string> FileManager::GetTargetNames()
{
	std::vector<std::string> model_files;

	WIN32_FIND_DATA search_data;
	memset(&search_data, 0, sizeof(WIN32_FIND_DATA));
	HANDLE handle = FindFirstFile(L"Inputs/data/*", &search_data);
	while (handle != INVALID_HANDLE_VALUE)
	{
		//cout << "\n" << search_data.cFileName;
		if (std::wcsstr(search_data.cFileName, L".mdl") != NULL) //if it has mdl extention
		{
			std::wstring w_s(search_data.cFileName);
			std::string model_name = std::string((w_s).begin(), (w_s).end()-4);
			model_files.push_back(model_name);
			//SendMessage(xmlFileComboBox, CB_ADDSTRING, 0, (LPARAM)search_data.cFileName);
		}
		if (FindNextFile(handle, &search_data) == FALSE)
			break;
	}

	//need to move board to the front of the vector
	sort(model_files.begin(), model_files.end(), ([](const std::string& val_1, const std::string& val_2) {	return (val_1[0] == 'b' && val_1[1] == 'o'); }));

	return model_files;
}

