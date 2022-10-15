#pragma once

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif

#include <stdio.h>

#include "../Containers.h"
#include <memory>

#define THREADING
#ifdef THREADING
#include <thread>
#endif

#include "PI_sdk/PI_GCS2_DLL.h"
#include "PI_sdk/PI_GCS2_DLL_PF.h"


const BOOL SERVO_ON = TRUE;
const BOOL SERVO_OFF = FALSE;

class Connection_PI: public Connection
{
public:
	Connection_PI(Returns& e_returns);
	~Connection_PI();

	bool ConnectPort(const char* com_port, int baud_rate);
	bool DisconnectPort();

private:
	Returns& eReturns;

	enum ConnectionType
	{
		Dialog,
		RS232,
		TCPIP,
		USB
	};

	std::string connType;
	
	void PrintControllerIdentification(int iD)
	{
		char szIDN[200];

		if (!PI_qIDN(iD, szIDN, 199))
		{
			std::runtime_error("qIDN failed. Exiting.");
		}
		std::cout << "qIDN returned: " << szIDN << std::endl;
	}

	void CloseConnectionWithComment(int iD, std::string comment)
	{
		std::cout << comment.c_str() << std::endl;

		ReportError(iD);
		PI_CloseConnection(iD);
	}

	void ReportError(int iD)
	{
		int err = PI_GetError(iD);
		char szErrMsg[300];

		if (PI_TranslateError(err, szErrMsg, 299))
		{
			std::cout << "Error " << err << " occurred: " << szErrMsg << std::endl;
		}
	}

	void SetServoState(int ID, std::string axis, BOOL state)
	{
		if (!PI_SVO(ID, axis.c_str(), &state))
		{
			throw std::runtime_error("SVO failed. Exiting");
		}

		std::cout << "Turned servo " << (state ? "on" : "off") << std::endl;
	}
};

