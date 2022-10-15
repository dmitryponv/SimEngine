#pragma once

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif

#include <stdio.h>

#include "dynamixel_sdk.h"    
#include "../Containers.h"
#include <memory>

class Connection_DXL: public Connection
{
public:
	Connection_DXL(Returns& e_returns);
	~Connection_DXL();

	bool ConnectPort(const char* com_port, int baud_rate);
	bool DisconnectPort();

	bool WriteByte(int device_id, std::string servo_name, int v_addr, uint8_t v_value);
	bool WriteBytes(int device_id, std::string servo_name, int v_addr, uint32_t v_value, bool only_2_bytes = false);
	bool ReadByte(int device_id, std::string servo_name, int v_addr, uint8_t& v_value);
	bool ReadBytes(int device_id, std::string servo_name, int v_addr, uint32_t& v_value, bool only_2_byte = false);
	void CommandDelay(uint16_t ms_delay);

private:
	Returns& eReturns;

	std::shared_ptr<dynamixel::PortHandler> portHandler;
	dynamixel::PacketHandler* packetHandler;
	
	int getch()
	{
#ifdef __linux__
		struct termios oldt, newt;
		int ch;
		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
		ch = getchar();
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
		return ch;
#elif defined(_WIN32) || defined(_WIN64)
		return _getch();
#endif
	}

	int kbhit(void)
	{
#ifdef __linux__
		struct termios oldt, newt;
		int ch;
		int oldf;

		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
		oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
		fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

		ch = getchar();

		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
		fcntl(STDIN_FILENO, F_SETFL, oldf);

		if (ch != EOF)
		{
			ungetc(ch, stdin);
			return 1;
		}

		return 0;
#elif defined(_WIN32) || defined(_WIN64)
		return _kbhit();
#endif
	}

	int protocolVersion = 2;	

};

