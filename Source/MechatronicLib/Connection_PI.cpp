#include "Connection_PI.h"

Connection_PI::Connection_PI(Returns& e_returns) : eReturns(e_returns)
{
}


Connection_PI::~Connection_PI()
{

}

bool Connection_PI::DisconnectPort()
{
	PI_CloseConnection(iD);

	return true;
}

bool Connection_PI::ConnectPort(const char* com_port, int baud_rate)
{
	char connectedUsbController[1024];
	int noDevices = PI_EnumerateUSB(connectedUsbController, 1024, "");
	std::cout << "Found " << noDevices << " controllers connected via USB, connecting to the first one:" << std::endl << connectedUsbController << std::endl;
	char* pos = strchr(connectedUsbController, '\n');
	if (pos)
	{
		*pos = '\0';
	}

	iD = PI_ConnectUSB(connectedUsbController);
	connType = "USB";

	if (iD < 0)
	{
		std::cout << "Unable to connect.";
		return 1;
	}

	try
	{
		PrintControllerIdentification(iD);

		std::string sAxis = "1";
		SetServoState(iD, sAxis, SERVO_ON);

		//std::cout << "Successfully determined min (" << minPosLimit << ") and max (" << maxPosLimit << ") position limits." << std::endl;
	}

	catch (std::runtime_error e)
	{
		CloseConnectionWithComment(iD, e.what());
		return 1;
	}


	return true;
}