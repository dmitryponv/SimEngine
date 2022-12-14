#pragma once
#include "../../Containers.h"

#define DIRECTINPUT_VERSION 0x0800


#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

class Direct3DInput
{
public:
	Direct3DInput(CALLBACKS& v_callbacks);
	//Direct3DInput(const Direct3DInput&);
	~Direct3DInput();

	bool Initialize(std::shared_ptr<WindowsContainer> windows_container);
	void Shutdown();
	bool Frame();

	bool IsEscapePressed();
	void GetMouseLocation(int&, int&);

private:
	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();

private:
	IDirectInput8* m_directInput;
	IDirectInputDevice8* m_keyboard;
	IDirectInputDevice8* m_mouse;

	unsigned char m_keyboardState[256];
	DIMOUSESTATE m_mouseState;

	int m_screenWidth, m_screenHeight;
	int m_mouseX, m_mouseY;

	CALLBACKS& mCallbacks;
};