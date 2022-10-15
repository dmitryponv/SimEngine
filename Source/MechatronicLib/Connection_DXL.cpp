#include "Connection_DXL.h"

Connection_DXL::Connection_DXL(Returns& e_returns) : eReturns(e_returns)
{
}


Connection_DXL::~Connection_DXL()
{
	if (portHandler != nullptr)
	{
		DebugOut("Closing COM port\n");
		portHandler->closePort();
		portHandler = nullptr;

		if (packetHandler)
		{
			packetHandler = nullptr;
			delete packetHandler;
		}
	}
}

bool Connection_DXL::DisconnectPort()
{
	if (portHandler == nullptr)
	{
		eReturns.Throw("COM", "Port already closed");
		return false;
	}

	if (packetHandler)
	{
		packetHandler = nullptr;
		delete packetHandler;
	}

	DebugOut("Closing COM port\n");
	portHandler->closePort();
	portHandler = nullptr;
	return true;
}

bool Connection_DXL::ConnectPort(const char* com_port, int baud_rate)
{
	if (portHandler)
	{
		portHandler->closePort();
		portHandler = nullptr;
		eReturns.Throw("COM", "Port already opened");
		return false;
	}

	//Correct Windows Com Port labels beyond COM9
#if defined(_WIN32) || defined(_WIN64)
	char com_port_new[10];
	sprintf_s(com_port_new, 10, "\\\\.\\%s", com_port);
	portHandler.reset(dynamixel::PortHandler::getPortHandler(com_port_new));
#else
	portHandler.reset(dynamixel::PortHandler::getPortHandler(com_port));
#endif

	int dxl_comm_result = COMM_TX_FAIL;             // Communication result


	uint8_t dxl_error = 0;                          // Dynamixel error
	int32_t dxl_present_position = 0;               // Present position


	if (portHandler->openPort())
	{
		DebugOut("Succeeded to open the port: %s\n", com_port);
	}
	else
	{
		portHandler->closePort();
		portHandler = nullptr;
		eReturns.Throw("COM", "Failed to open the port");
		return false;
	}

	// Set port baudrate
	if (portHandler->setBaudRate(baud_rate))
	{
		DebugOut("Succeeded to change the baudrate to: %d\n", baud_rate);
	}
	else
	{
		portHandler->closePort();
		portHandler = nullptr;
		eReturns.Throw("COM", "Failed to change the baudrate");
		return false;
	}

	//Get Packet Handler
	packetHandler = dynamixel::PacketHandler::getPacketHandler(2.0F);

	return true;
}

///READ AND WRITE FUNCTIONS
bool Connection_DXL::WriteByte(int device_id, std::string servo_name, int v_addr, uint8_t v_value)
{
	if (v_addr == -1)
	{
		eReturns.Throw(servo_name, "Write Address is -1");
		return false;
	}

	if (portHandler == nullptr)
	{
		eReturns.Throw(servo_name, "Port Handler is NULL");
		return false;
	}

	uint8_t dxl_error = 0;
	int dxl_comm_result = 0;

	{
		std::unique_lock<std::mutex> lock(mMutex);
		packetHandler->write1ByteTxRx(portHandler.get(), device_id, v_addr, v_value, &dxl_error);
	}
	CommandDelay(10);

	if (dxl_comm_result != COMM_SUCCESS)
	{
		DebugOut("Servo: %s WriteByte dxl_comm_result Error %i\n", servo_name, dxl_comm_result);
		eReturns.Throw(servo_name, "DXL Write Error");
		packetHandler->printTxRxResult(dxl_comm_result);
		return false;
	}
	else if (dxl_error != 0)
	{
		DebugOut("Servo: %s WriteByte dxl_error Error %i\n", servo_name, dxl_error);
		eReturns.Throw(servo_name, "DXL Write Error");
		packetHandler->printRxPacketError(dxl_error);
		return false;
	}

	return true;
}
bool Connection_DXL::WriteBytes(int device_id, std::string servo_name, int v_addr, uint32_t v_value, bool only_2_bytes)
{
	if (v_addr == -1)
	{
		eReturns.Throw(servo_name, "Write Address is -1");
		return false;
	}

	if (!portHandler)
	{
		eReturns.Throw(servo_name, "Port Handler is NULL");
		return false;
	}

	uint8_t dxl_error = 0;
	int dxl_comm_result = 0;

	{
		std::unique_lock<std::mutex> lock(mMutex);
		if (protocolVersion == 1 || only_2_bytes)
			dxl_comm_result = packetHandler->write2ByteTxRx(portHandler.get(), device_id, v_addr, v_value, &dxl_error);
		else
			dxl_comm_result = packetHandler->write4ByteTxRx(portHandler.get(), device_id, v_addr, v_value, &dxl_error);
	}
	CommandDelay(10);

	if (dxl_comm_result != COMM_SUCCESS)
	{
		DebugOut("Servo: %s WriteBytes dxl_comm_result Error %i\n", servo_name, dxl_comm_result);
		eReturns.Throw(servo_name, "DXL Write Error");
		packetHandler->printTxRxResult(dxl_comm_result);
		return false;
	}
	else if (dxl_error != 0)
	{
		DebugOut("Servo: %s WriteBytes dxl_error Error %i\n", servo_name, dxl_error);
		eReturns.Throw(servo_name, "DXL Write Error");
		packetHandler->printRxPacketError(dxl_error);
		return false;
	}

	return true;
}
bool Connection_DXL::ReadByte(int device_id, std::string servo_name, int v_addr, uint8_t& v_value)
{
	if (v_addr == -1)
	{
		eReturns.Throw(servo_name, "Read Address is -1");
		return false;
	}

	if (portHandler == nullptr)
	{
		eReturns.Throw(servo_name, "Port Handler is NULL");
		return false;
	}

	uint8_t dxl_error = 0;
	int dxl_comm_result = 0;
	{
		std::unique_lock<std::mutex> lock(mMutex);
		dxl_comm_result = packetHandler->read1ByteTxRx(portHandler.get(), device_id, v_addr, &v_value, &dxl_error);
	}
	CommandDelay(10);

	if (dxl_comm_result != COMM_SUCCESS)
	{
		DebugOut("Servo: %s ReadByte dxl_comm_result Error %i\n", servo_name, dxl_comm_result);
		eReturns.Throw(servo_name, "DXL Read Error");
		packetHandler->printTxRxResult(dxl_comm_result);
		return false;
	}
	else if (dxl_error != 0)
	{
		DebugOut("Servo: %s ReadByte dxl_error Error %i\n", servo_name, dxl_error);
		eReturns.Throw(servo_name, "DXL Read Error");
		packetHandler->printRxPacketError(dxl_error);
		return false;
	}

	return true;
}
bool Connection_DXL::ReadBytes(int device_id, std::string servo_name, int v_addr, uint32_t& v_value, bool only_2_bytes)
{
	if (v_addr == -1)
	{
		eReturns.Throw(servo_name, "Read Address is -1");
		return false;
	}

	if (portHandler == nullptr)
	{
		eReturns.Throw(servo_name, "Port Handler is NULL");
		return false;
	}

	uint8_t dxl_error = 0;
	int dxl_comm_result = 0;

	{
		std::unique_lock<std::mutex> lock(mMutex);
		if (protocolVersion == 1 || only_2_bytes)
		{
			uint16_t v_data;
			dxl_comm_result = packetHandler->read2ByteTxRx(portHandler.get(), device_id, v_addr, &v_data, &dxl_error);
			v_value = static_cast<uint32_t>(v_data);
		}
		else
			dxl_comm_result = packetHandler->read4ByteTxRx(portHandler.get(), device_id, v_addr, &v_value, &dxl_error);
	}
	CommandDelay(10);

	if (dxl_comm_result != COMM_SUCCESS)
	{
		DebugOut("Servo: %s ReadBytes dxl_comm_result Error %i\n", servo_name, dxl_comm_result);
		eReturns.Throw(servo_name, "DXL Read Error");
		packetHandler->printTxRxResult(dxl_comm_result);
		return false;
	}
	else if (dxl_error != 0)
	{
		DebugOut("Servo: %s ReadBytes dxl_error Error %i\n", servo_name, dxl_error);
		eReturns.Throw(servo_name, "DXL Read Error");
		packetHandler->printRxPacketError(dxl_error);
		return false;
	}

	return true;
}

void Connection_DXL::CommandDelay(uint16_t ms_delay)
{
#ifdef __linux__
	usleep(ms_delay * 1000);
#elif defined(_WIN32) || defined(_WIN64)
	Sleep(ms_delay);
#endif
}