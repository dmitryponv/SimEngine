#include "EEControl.h"



EEControl::EEControl(CALLBACKS& v_callbacks, std::shared_ptr<HWND> menu_item, std::string mechanism_name, std::string chain_name, int min_range, int max_range):
	CustomControl(menu_item),
	mCallbacks(v_callbacks),
	mechanismName(mechanism_name),
	chainName(chain_name),
	minRange(min_range),
	maxRange(max_range),
	hwndCustom(NULL),
	g_hInst(nullptr)
{
	controlName = mechanismName + "*" + chainName + "*EEControl";
	mCallbacks["ChangeEE_" + controlName] = std::bind(&EEControl::ChangeCtrlCallback, this, std::placeholders::_1);   //CALLBACK INIT
	mCallbacks["GetEE_" + controlName] = std::bind(&EEControl::GetCtrlCallback, this, std::placeholders::_1);   //CALLBACK INIT
}


EEControl::~EEControl()
{
	DestroyWindow(hwndCustom);
	DestroyWindow(hwndLabel);
	DestroyWindow(hwndUpDnEdtBdy1);
	DestroyWindow(hwndUpDnCtl1);
	DestroyWindow(hwndSlider1);
	DestroyWindow(hwndUpDnEdtBdy2);
	DestroyWindow(hwndUpDnCtl2);
	DestroyWindow(hwndSlider2);
	DestroyWindow(hwndUpDnEdtBdy3);
	DestroyWindow(hwndUpDnCtl3);
	DestroyWindow(hwndSlider3);
	DestroyWindow(hwndUpDnEdtBdy4);
	DestroyWindow(hwndUpDnCtl4);
	DestroyWindow(hwndSlider4);
	DestroyWindow(hwndUpDnEdtBdy5);
	DestroyWindow(hwndUpDnCtl5);
	DestroyWindow(hwndSlider5);
	DestroyWindow(hwndUpDnEdtBdy6);
	DestroyWindow(hwndUpDnCtl6);
	DestroyWindow(hwndSlider6);
	if (mCallbacks.size() != 0)
	{
		if (mCallbacks.find("ChangeEE_" + controlName) != mCallbacks.end())
			mCallbacks.erase("ChangeEE_" + controlName);
		if (mCallbacks.find("GetEE_" + controlName) != mCallbacks.end())
			mCallbacks.erase("GetEE_" + controlName);

	}
}

bool EEControl::CreateControl(HWND m_hwnd, HINSTANCE* m_happ_instance, int v_right, int v_top)
{
	g_hInst = m_happ_instance;
	
	//Create Parent Subclass Window
	RegisterSubClass(*g_hInst, WC_EDIT, L"CustomEEControl", CustomDialogProcStatic);
	hwndCustom = CreateWindow(L"CustomEEControl", L"EEControl", WS_CHILD | WS_VISIBLE,
		v_right, v_top, 200, 200, m_hwnd, NULL, *g_hInst, NULL);

	//Create Child Controls
	//GetClientRect(hwndCustom, &rcClient);
	
	hwndLabel = CreateLabel(hwndCustom, chainName + ":EE", 0, 0);      // Create a label for the Up-Down control.

	int v_height = 0;
	hwndLabel1 = CreateLabel(hwndCustom, "X", 100, v_height);
	hwndUpDnEdtBdy1 = CreateUpDnBuddy(hwndCustom, v_height);  // Create an buddy window (an Edit control).
	hwndUpDnCtl1 = CreateUpDnCtl(hwndCustom, minRange, maxRange, v_height);    // Create an Up-Down control.
	hwndSlider1 = CreateSlider(hwndCustom, minRange, maxRange, v_height);
	v_height += 25;
	hwndLabel2 = CreateLabel(hwndCustom, "Y", 100, v_height);
	hwndUpDnEdtBdy2 = CreateUpDnBuddy(hwndCustom, v_height);  // Create an buddy window (an Edit control).
	hwndUpDnCtl2 = CreateUpDnCtl(hwndCustom, minRange, maxRange, v_height);    // Create an Up-Down control.
	hwndSlider2 = CreateSlider(hwndCustom, minRange, maxRange, v_height);
	v_height += 25;
	hwndLabel3 = CreateLabel(hwndCustom, "Z", 100, v_height);
	hwndUpDnEdtBdy3 = CreateUpDnBuddy(hwndCustom, v_height);  // Create an buddy window (an Edit control).
	hwndUpDnCtl3 = CreateUpDnCtl(hwndCustom, minRange, maxRange, v_height);    // Create an Up-Down control.
	hwndSlider3 = CreateSlider(hwndCustom, minRange, maxRange, v_height);
	v_height += 25;
	hwndLabel4 = CreateLabel(hwndCustom, "aX", 100, v_height);
	hwndUpDnEdtBdy4 = CreateUpDnBuddy(hwndCustom, v_height);  // Create an buddy window (an Edit control).
	hwndUpDnCtl4 = CreateUpDnCtl(hwndCustom, minRange, maxRange, v_height);    // Create an Up-Down control.
	hwndSlider4 = CreateSlider(hwndCustom, minRange, maxRange, v_height);
	v_height += 25;
	hwndLabel5 = CreateLabel(hwndCustom, "aY", 100, v_height);
	hwndUpDnEdtBdy5 = CreateUpDnBuddy(hwndCustom, v_height);  // Create an buddy window (an Edit control).
	hwndUpDnCtl5 = CreateUpDnCtl(hwndCustom, minRange, maxRange, v_height);    // Create an Up-Down control.
	hwndSlider5 = CreateSlider(hwndCustom, minRange, maxRange, v_height);
	v_height += 25;
	hwndLabel6 = CreateLabel(hwndCustom, "aZ", 100, v_height);
	hwndUpDnEdtBdy6 = CreateUpDnBuddy(hwndCustom, v_height);  // Create an buddy window (an Edit control).
	hwndUpDnCtl6 = CreateUpDnCtl(hwndCustom, minRange, maxRange, v_height);    // Create an Up-Down control.
	hwndSlider6 = CreateSlider(hwndCustom, minRange, maxRange, v_height);
	
	SetWindowLongPtr(hwndCustom, GWLP_USERDATA, (LONG_PTR) this);

	SendMessage(hwndUpDnEdtBdy1, WM_SETTEXT, (WPARAM)1, (LPARAM)L"0.0");
	SendMessage(hwndUpDnEdtBdy2, WM_SETTEXT, (WPARAM)1, (LPARAM)L"0.0");
	SendMessage(hwndUpDnEdtBdy3, WM_SETTEXT, (WPARAM)1, (LPARAM)L"0.0");
	SendMessage(hwndUpDnEdtBdy4, WM_SETTEXT, (WPARAM)1, (LPARAM)L"0.0");
	SendMessage(hwndUpDnEdtBdy5, WM_SETTEXT, (WPARAM)1, (LPARAM)L"0.0");
	SendMessage(hwndUpDnEdtBdy6, WM_SETTEXT, (WPARAM)1, (LPARAM)L"0.0");

	return true;
}


LRESULT CALLBACK EEControl::CustomDialogProc(HWND m_hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{	
	HWND m_control = GetFocus();
	switch (u_msg)
	{
	case WM_NOTIFY:
		if (m_control == hwndUpDnEdtBdy1)
		{
			if (((LPNMHDR)l_param)->code == UDN_DELTAPOS)
			{
				LPNMUPDOWN lp_nmud = (LPNMUPDOWN)l_param;
				LRESULT v_delta = lp_nmud->iDelta;
				eeX_Position += v_delta;
				
				Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
			}
		}
		else if (m_control == hwndSlider1)
		{
			eeX_Position = SendMessage(hwndSlider1, TBM_GETPOS, 0, 0);
			Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
		}

		else if (m_control == hwndUpDnEdtBdy2)
		{
			if (((LPNMHDR)l_param)->code == UDN_DELTAPOS)
			{
				LPNMUPDOWN lp_nmud = (LPNMUPDOWN)l_param;
				LRESULT v_delta = lp_nmud->iDelta;
				eeY_Position += v_delta;

				Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
			}
		}
		else if (m_control == hwndSlider2)
		{
			eeY_Position = SendMessage(hwndSlider2, TBM_GETPOS, 0, 0);
			Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
		}

		else if (m_control == hwndUpDnEdtBdy3)
		{
			if (((LPNMHDR)l_param)->code == UDN_DELTAPOS)
			{
				LPNMUPDOWN lp_nmud = (LPNMUPDOWN)l_param;
				LRESULT v_delta = lp_nmud->iDelta;
				eeZ_Position += v_delta;

				Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
			}
		}
		else if (m_control == hwndSlider3)
		{
			eeZ_Position = SendMessage(hwndSlider3, TBM_GETPOS, 0, 0);
			Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
		}
		
		else if (m_control == hwndUpDnEdtBdy4)
		{
			if (((LPNMHDR)l_param)->code == UDN_DELTAPOS)
			{
				LPNMUPDOWN lp_nmud = (LPNMUPDOWN)l_param;
				LRESULT v_delta = lp_nmud->iDelta;
				eeX_Rotation += v_delta;

				Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
			}
		}
		else if (m_control == hwndSlider4)
		{
			eeX_Rotation = SendMessage(hwndSlider4, TBM_GETPOS, 0, 0);
			Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
		}

		else if (m_control == hwndUpDnEdtBdy5)
		{
			if (((LPNMHDR)l_param)->code == UDN_DELTAPOS)
			{
				LPNMUPDOWN lp_nmud = (LPNMUPDOWN)l_param;
				LRESULT v_delta = lp_nmud->iDelta;
				eeY_Rotation += v_delta;

				Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
			}
		}
		else if (m_control == hwndSlider5)
		{
			eeY_Rotation = SendMessage(hwndSlider5, TBM_GETPOS, 0, 0);
			Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
		}

		else if (m_control == hwndUpDnEdtBdy6)
		{
			if (((LPNMHDR)l_param)->code == UDN_DELTAPOS)
			{
				LPNMUPDOWN lp_nmud = (LPNMUPDOWN)l_param;
				LRESULT v_delta = lp_nmud->iDelta;
				eeZ_Rotation += v_delta;

				Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
			}
		}
		else if (m_control == hwndSlider6)
		{
			eeZ_Rotation = SendMessage(hwndSlider6, TBM_GETPOS, 0, 0);
			Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
		}


	case WM_COMMAND:
		if (HIWORD(w_param) == EN_UPDATE && m_control == hwndUpDnEdtBdy1)
		{
			wchar_t txt[40];
			GetWindowText(hwndUpDnEdtBdy1, txt, sizeof(txt));
			if (CheckEnter(txt, sizeof(txt)))
			{
				swscanf_s(txt, L"%f", &eeX_Position);
				Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
			}
		}
		else if (HIWORD(w_param) == EN_UPDATE && m_control == hwndUpDnEdtBdy2)
		{
			wchar_t txt[40];
			GetWindowText(hwndUpDnEdtBdy2, txt, sizeof(txt));
			if (CheckEnter(txt, sizeof(txt)))
			{
				swscanf_s(txt, L"%f", &eeY_Position);
				Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
			}
		}

		else if (HIWORD(w_param) == EN_UPDATE && m_control == hwndUpDnEdtBdy3)
		{
			wchar_t txt[40];
			GetWindowText(hwndUpDnEdtBdy3, txt, sizeof(txt));
			if (CheckEnter(txt, sizeof(txt)))
			{
				swscanf_s(txt, L"%f", &eeZ_Position);
				Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
			}
		}

		else if (HIWORD(w_param) == EN_UPDATE && m_control == hwndUpDnEdtBdy4)
		{
			wchar_t txt[40];
			GetWindowText(hwndUpDnEdtBdy4, txt, sizeof(txt));
			if (CheckEnter(txt, sizeof(txt)))
			{
				swscanf_s(txt, L"%f", &eeX_Rotation);
				Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
			}
		}

		else if (HIWORD(w_param) == EN_UPDATE && m_control == hwndUpDnEdtBdy5)
		{
			wchar_t txt[40];
			GetWindowText(hwndUpDnEdtBdy5, txt, sizeof(txt));
			if (CheckEnter(txt, sizeof(txt)))
			{
				swscanf_s(txt, L"%f", &eeY_Rotation);
				Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
			}
		}

		else if (HIWORD(w_param) == EN_UPDATE && m_control == hwndUpDnEdtBdy6)
		{
			wchar_t txt[40];
			GetWindowText(hwndUpDnEdtBdy6, txt, sizeof(txt));
			if (CheckEnter(txt, sizeof(txt)))
			{
				swscanf_s(txt, L"%f", &eeZ_Rotation);
				Move(eeX_Position, eeY_Position, eeZ_Position, eeX_Rotation, eeY_Rotation, eeZ_Rotation);
			}
		}
		break;
	}
	return DefWindowProc(m_hwnd, u_msg, w_param, l_param);
}

//CALLBACK FUNCTION
bool EEControl::ChangeCtrlCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::EEMotion> move_position = std::dynamic_pointer_cast<CallbackData::EEMotion>(callback_data);
	//Move(move_position->mNumber);
	SendMessage(hwndUpDnEdtBdy1, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->xPos).c_str()));
	SendMessage(hwndSlider1, TBM_SETPOS, (WPARAM)1, (LPARAM)move_position->xPos);	//Set text in WC_EDIT
	SendMessage(hwndUpDnEdtBdy1, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->xPos).c_str()));

	SendMessage(hwndUpDnEdtBdy2, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->yPos).c_str()));
	SendMessage(hwndSlider2, TBM_SETPOS, (WPARAM)1, (LPARAM)move_position->yPos);	//Set text in WC_EDIT
	SendMessage(hwndUpDnEdtBdy2, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->yPos).c_str()));

	SendMessage(hwndUpDnEdtBdy3, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->zPos).c_str()));
	SendMessage(hwndSlider3, TBM_SETPOS, (WPARAM)1, (LPARAM)move_position->zPos);	//Set text in WC_EDIT
	SendMessage(hwndUpDnEdtBdy3, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->zPos).c_str()));

	SendMessage(hwndUpDnEdtBdy4, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->xRot).c_str()));
	SendMessage(hwndSlider4, TBM_SETPOS, (WPARAM)1, (LPARAM)move_position->xRot);	//Set text in WC_EDIT
	SendMessage(hwndUpDnEdtBdy4, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->xRot).c_str()));

	SendMessage(hwndUpDnEdtBdy5, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->yRot).c_str()));
	SendMessage(hwndSlider5, TBM_SETPOS, (WPARAM)1, (LPARAM)move_position->yRot);	//Set text in WC_EDIT
	SendMessage(hwndUpDnEdtBdy5, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->yRot).c_str()));

	SendMessage(hwndUpDnEdtBdy6, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->zRot).c_str()));
	SendMessage(hwndSlider6, TBM_SETPOS, (WPARAM)1, (LPARAM)move_position->zRot);	//Set text in WC_EDIT
	SendMessage(hwndUpDnEdtBdy6, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->zRot).c_str()));

	eeX_Position = move_position->xPos;
	eeY_Position = move_position->yPos;
	eeZ_Position = move_position->zPos;
	eeX_Rotation = move_position->xRot;
	eeY_Rotation = move_position->yRot;
	eeZ_Rotation = move_position->zRot;

	return true;
}

//CALLBACK FUNCTION
bool EEControl::GetCtrlCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::EEMotion> move_position = std::dynamic_pointer_cast<CallbackData::EEMotion>(callback_data);
	move_position->xPos = eeX_Position;
	move_position->yPos = eeY_Position;
	move_position->zPos = eeZ_Position;
	move_position->xRot = eeX_Rotation;
	move_position->yRot = eeY_Rotation;
	move_position->zRot = eeZ_Rotation;
	return true;
}

HWND EEControl::CreateUpDnBuddy(HWND m_hwnd, int v_height)
{
	//hwndLabel = NULL;

	iCex.dwICC = ICC_STANDARD_CLASSES;    // Set the Initialization Flag value.
	InitCommonControlsEx(&iCex);          // Initialize the Common Controls Library to use 
										  // Buttons, Edit Controls, Static Controls, Listboxes, 
										  // Comboboxes, and  Scroll Bars.

	hControl = CreateWindowEx(WS_EX_CLIENTEDGE,    //Extended window styles.
		WC_EDIT,
		NULL,
		WS_CHILD | WS_VISIBLE,     // Edit control styles.
		100, 0 + v_height,
		80, 23,
		m_hwnd,
		NULL,
		*g_hInst,
		NULL);

	return (hControl);
}

HWND EEControl::CreateUpDnCtl(HWND m_hwnd, int min_range, int max_range, int v_height)
{
	iCex.dwICC = ICC_UPDOWN_CLASS;    // Set the Initialization Flag value.
	InitCommonControlsEx(&iCex);      // Initialize the Common Controls Library.

	hControl = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING,
		UPDOWN_CLASS,
		NULL,
		WS_CHILDWINDOW | WS_VISIBLE
		| UDS_AUTOBUDDY | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK, //| UDS_SETBUDDYINT,
		0, 0 + v_height,
		0, 0,         // Set to zero to automatically size to fit the buddy window.
		m_hwnd,
		NULL,
		*g_hInst,
		NULL);

	SendMessage(hControl, UDM_SETRANGE, 0, MAKELPARAM(max_range, min_range));    // Sets the controls direction 
																		   // and range.
	return (hControl);
}

HWND EEControl::CreateLabel(HWND m_hwnd, std::string v_name, int v_x, int v_y)
{	
	std::wstring w_name = std::wstring((v_name).begin(), (v_name).end());
		
	hControl = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING,
		WC_STATIC,
		w_name.c_str(),
		WS_CHILDWINDOW | WS_VISIBLE | SS_RIGHT,
		v_x, v_y,
		100, 23,
		m_hwnd,
		NULL,
		*g_hInst,
		NULL);

	return (hControl);
}

HWND EEControl::CreateSlider(HWND hwndParent, int min_range, int max_range, int v_height)
{
	hControl = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING,
		TRACKBAR_CLASS,
		L"Slider:",
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,
		0,20+v_height,
		100,23,
		hwndParent,
		NULL,
		*g_hInst,
		NULL);

	SendMessage(hControl, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(min_range, max_range));
	SendMessage(hControl, TBM_SETPOS, (WPARAM)1, 0);

	return (hControl);
}


void EEControl::Move(float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot)
{
	SendMessage(hwndSlider1, TBM_SETPOS, (WPARAM)1, (LPARAM)x_pos);
	SendMessage(hwndUpDnEdtBdy1, WM_SETTEXT, (WPARAM)1,  (LPARAM)(std::to_wstring(x_pos).c_str()));

	SendMessage(hwndSlider2, TBM_SETPOS, (WPARAM)1, (LPARAM)y_pos);
	SendMessage(hwndUpDnEdtBdy2, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(y_pos).c_str()));

	SendMessage(hwndSlider3, TBM_SETPOS, (WPARAM)1, (LPARAM)z_pos);
	SendMessage(hwndUpDnEdtBdy3, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(z_pos).c_str()));

	SendMessage(hwndSlider4, TBM_SETPOS, (WPARAM)1, (LPARAM)x_rot);
	SendMessage(hwndUpDnEdtBdy4, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(x_rot).c_str()));

	SendMessage(hwndSlider5, TBM_SETPOS, (WPARAM)1, (LPARAM)y_rot);
	SendMessage(hwndUpDnEdtBdy5, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(y_rot).c_str()));

	SendMessage(hwndSlider6, TBM_SETPOS, (WPARAM)1, (LPARAM)z_rot);
	SendMessage(hwndUpDnEdtBdy6, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(z_rot).c_str()));
	
	float new_sum = abs(x_pos) + abs(y_pos) + abs(z_pos) + abs(x_rot) + abs(y_rot) + abs(z_rot);
	// CALLBACK EVENT	
	if (abs(new_sum - lastSum) > 0.01)
	{
		lastSum = new_sum;
		if (mCallbacks.find("PositionEECallback") != mCallbacks.end())
		{
			std::shared_ptr<CallbackData::EEMotion> joint_data = std::make_shared<CallbackData::EEMotion>(mechanismName, chainName, x_pos, y_pos, z_pos, x_rot, y_rot, z_rot);
			mCallbacks.at("PositionEECallback")(std::dynamic_pointer_cast<CallbackData::BlankData>(joint_data));
		}
	}
}