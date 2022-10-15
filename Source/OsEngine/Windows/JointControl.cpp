#include "JointControl.h"



JointControl::JointControl(CALLBACKS& v_callbacks, std::shared_ptr<HWND> menu_item, std::string mechanism_name, std::string chain_name, std::string joint_name, bool prismatic_joint, int min_range, int max_range):
	CustomControl(menu_item),
	mCallbacks(v_callbacks),
	mechanismName(mechanism_name),
	chainName(chain_name),
	jointName(joint_name),
	prismaticJoint(prismatic_joint),
	minRange(min_range),
	maxRange(max_range),
	hwndCustom(NULL),
	g_hInst(nullptr)
{
	controlName = "*" + mechanismName + "*" + chainName + "*" + jointName;
	mCallbacks["ChangeCtrl_" + controlName] = std::bind(&JointControl::ChangeCtrlCallback, this, std::placeholders::_1);   //CALLBACK INIT
}


JointControl::~JointControl()
{
	DestroyWindow(hwndCustom);
	DestroyWindow(hwndLabel);
	DestroyWindow(hwndUpDnEdtBdy);
	DestroyWindow(hwndUpDnCtl);
	DestroyWindow(hwndSlider);
	if(mCallbacks.size() != 0)
		if(mCallbacks.find("ChangeCtrl_" + controlName) != mCallbacks.end())
			mCallbacks.erase("ChangeCtrl_" + controlName);
}

bool JointControl::CreateControl(HWND m_hwnd, HINSTANCE* m_happ_instance, int v_right, int v_top)
{
	g_hInst = m_happ_instance;
	
	//Create Parent Subclass Window
	RegisterSubClass(*g_hInst, WC_EDIT, L"CustomJointControl", CustomDialogProcStatic);
	hwndCustom = CreateWindow(L"CustomJointControl", L"JointControl", WS_CHILD | WS_VISIBLE,
		v_right, v_top, 200, 50, m_hwnd, NULL, *g_hInst, NULL);

	//Create Child Controls
	//GetClientRect(hwndCustom, &rcClient);
	hwndLabel = CreateLabel(hwndCustom);      // Create a label for the Up-Down control.
	hwndUpDnEdtBdy = CreateUpDnBuddy(hwndCustom);  // Create an buddy window (an Edit control).
	hwndUpDnCtl = CreateUpDnCtl(hwndCustom, minRange, maxRange);    // Create an Up-Down control.
	hwndSlider = CreateSlider(hwndCustom, minRange, maxRange);
	
	SetWindowLongPtr(hwndCustom, GWLP_USERDATA, (LONG_PTR) this);

	SendMessage(hwndUpDnEdtBdy, WM_SETTEXT, (WPARAM)1, (LPARAM)L"0.0");

	return true;
}


LRESULT CALLBACK JointControl::CustomDialogProc(HWND m_hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{	
	HWND m_control = GetFocus();
	switch (u_msg)
	{
	case WM_NOTIFY:
		if (m_control == hwndUpDnEdtBdy)
		{
			if (((LPNMHDR)l_param)->code == UDN_DELTAPOS)
			{
				LPNMUPDOWN lp_nmud = (LPNMUPDOWN)l_param;
				LRESULT v_delta = lp_nmud->iDelta;
				controlPosition += v_delta;
				
				Move(controlPosition);
			}
		}
		else if (m_control == hwndSlider)
		{
			controlPosition = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
			Move(controlPosition);
		}
	case WM_COMMAND:
		if (HIWORD(w_param) == EN_UPDATE && m_control == hwndUpDnEdtBdy)
		{
			wchar_t txt[40];
			GetWindowText(hwndUpDnEdtBdy, txt, sizeof(txt));
			if (CheckEnter(txt, sizeof(txt)))
			{
				swscanf_s(txt, L"%f", &controlPosition);
				Move(controlPosition);
			}
		}
		break;
	}
	return DefWindowProc(m_hwnd, u_msg, w_param, l_param);
}

//CALLBACK FUNCTION
bool JointControl::ChangeCtrlCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::Float> move_position = std::dynamic_pointer_cast<CallbackData::Float>(callback_data);
	//Move(move_position->mNumber);
	SendMessage(hwndUpDnEdtBdy, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->mNumber).c_str()));
	SendMessage(hwndSlider, TBM_SETPOS, (WPARAM)1, (LPARAM)move_position->mNumber);	//Set text in WC_EDIT
	SendMessage(hwndUpDnEdtBdy, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(move_position->mNumber).c_str()));
	return true;
}

HWND JointControl::CreateUpDnBuddy(HWND m_hwnd)
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
		100, 0,
		80, 23,
		m_hwnd,
		NULL,
		*g_hInst,
		NULL);

	return (hControl);
}

HWND JointControl::CreateUpDnCtl(HWND m_hwnd, int min_range, int max_range)
{
	iCex.dwICC = ICC_UPDOWN_CLASS;    // Set the Initialization Flag value.
	InitCommonControlsEx(&iCex);      // Initialize the Common Controls Library.

	hControl = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING,
		UPDOWN_CLASS,
		NULL,
		WS_CHILDWINDOW | WS_VISIBLE
		| UDS_AUTOBUDDY | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK, //| UDS_SETBUDDYINT,
		0, 0,
		0, 0,         // Set to zero to automatically size to fit the buddy window.
		m_hwnd,
		NULL,
		*g_hInst,
		NULL);

	SendMessage(hControl, UDM_SETRANGE, 0, MAKELPARAM(max_range, min_range));    // Sets the controls direction 
																		   // and range.
	return (hControl);
}

HWND JointControl::CreateLabel(HWND m_hwnd)
{
	std::string v_name = chainName + ":" + jointName;
	std::wstring w_name = std::wstring((v_name).begin(), (v_name).end());
		
	hControl = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING,
		WC_STATIC,
		w_name.c_str(),
		WS_CHILDWINDOW | WS_VISIBLE | SS_RIGHT,
		0, 0,
		100, 23,
		m_hwnd,
		NULL,
		*g_hInst,
		NULL);

	return (hControl);
}

HWND JointControl::CreateSlider(HWND hwndParent, int min_range, int max_range)
{
	hControl = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING,
		TRACKBAR_CLASS,
		L"Slider:",
		WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE,
		0,20,
		100,23,
		hwndParent,
		NULL,
		*g_hInst,
		NULL);

	SendMessage(hControl, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(min_range, max_range));
	SendMessage(hControl, TBM_SETPOS, (WPARAM)1, 0);

	return (hControl);
}


void JointControl::Move(float v_number)
{
	if (abs(v_number - lastControlPosition) > 0.001)
	{
		lastControlPosition = v_number;
		//Change Slider Position
		SendMessage(hwndSlider, TBM_SETPOS, (WPARAM)1, (LPARAM)v_number);
		//Set text in WC_EDIT
		SendMessage(hwndUpDnEdtBdy, WM_SETTEXT, (WPARAM)1,  (LPARAM)(std::to_wstring(v_number).c_str()));

		// CALLBACK EVENT
		
		if (mCallbacks.find("MoveJointCallback") != mCallbacks.end())
		{
			std::shared_ptr<CallbackData::JointMotion> joint_data = std::make_shared<CallbackData::JointMotion>(mechanismName, chainName, jointName, v_number, prismaticJoint);
			mCallbacks.at("MoveJointCallback")(std::dynamic_pointer_cast<CallbackData::BlankData>(joint_data));
		}

	}
	else
		SendMessage(hwndUpDnEdtBdy, WM_SETTEXT, (WPARAM)1, (LPARAM)(std::to_wstring(v_number).c_str()));
}