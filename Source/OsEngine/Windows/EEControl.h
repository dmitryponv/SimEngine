#pragma once
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>  
#include <string> 
#include "../../Containers.h"
#include "CustomControl.h"

#pragma comment(lib, "comctl32.lib")

#ifndef TOWSTRING
#define TOSTRING(x) (std::string((x).begin(), (x).end()))
#define TOWSTRING(x) (std::wstring((x).begin(), (x).end()))
#endif

class EEControl: public CustomControl
{
public:
	EEControl(CALLBACKS& v_callbacks, std::shared_ptr<HWND> menu_item, std::string mechanism_name, std::string chain_name, int min_range, int max_range);
	~EEControl();
	
	bool CreateControl(HWND m_hwnd, HINSTANCE* m_happ_instance, int v_right, int v_top);


	// this static method is registered with your window class
	static LRESULT CALLBACK CustomDialogProcStatic(HWND m_hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
	{
		auto pThis = (EEControl*)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);
		if (pThis != NULL)
			return pThis->CustomDialogProc(m_hwnd, u_msg, w_param, l_param);
		return DefWindowProc(m_hwnd, u_msg, w_param, l_param);
	}

	LRESULT CALLBACK CustomDialogProc(HWND m_hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);

	void Zero() { Move(0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F); }

	//void MoveToEnteredValue()
	//{
	//	float eeX_Position = 0.0F;
	//	wchar_t txt[12];
	//	GetWindowText(hwndUpDnEdtBdy1, txt, sizeof(txt));
	//	swscanf_s(txt, L"%f", &eeX_Position);
	//
	//
	//
	//	Move(control_position);
	//}
private:

	bool ChangeCtrlCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool GetCtrlCallback(std::shared_ptr<CallbackData::BlankData> callback_data);

	CALLBACKS& mCallbacks;

	INITCOMMONCONTROLSEX iCex;    // Structure for control initialization.

	//RECT rcClient;               // Client area of parent window (Dialog Box).

	HWND hwndCustom;

	HWND hControl = NULL;  //Handles to the controls.
	HWND hwndLabel = NULL;

	HWND hwndLabel1 = NULL;
	HWND hwndUpDnEdtBdy1 = NULL;
	HWND hwndUpDnCtl1 = NULL;
	HWND hwndSlider1 = NULL;

	HWND hwndLabel2 = NULL;
	HWND hwndUpDnEdtBdy2 = NULL;
	HWND hwndUpDnCtl2 = NULL;
	HWND hwndSlider2 = NULL;

	HWND hwndLabel3 = NULL;
	HWND hwndUpDnEdtBdy3 = NULL;
	HWND hwndUpDnCtl3 = NULL;
	HWND hwndSlider3 = NULL;

	HWND hwndLabel4 = NULL;
	HWND hwndUpDnEdtBdy4 = NULL;
	HWND hwndUpDnCtl4 = NULL;
	HWND hwndSlider4 = NULL;

	HWND hwndLabel5 = NULL;
	HWND hwndUpDnEdtBdy5 = NULL;
	HWND hwndUpDnCtl5 = NULL;
	HWND hwndSlider5 = NULL;

	HWND hwndLabel6 = NULL;
	HWND hwndUpDnEdtBdy6 = NULL;
	HWND hwndUpDnCtl6 = NULL;
	HWND hwndSlider6 = NULL;


	HWND CreateLabel(HWND m_hwnd, std::string v_name, int v_x, int v_y);
	HWND CreateUpDnBuddy(HWND m_hwnd, int v_height);
    HWND CreateUpDnCtl(HWND m_hwnd, int min_range, int max_range, int v_height);
	HWND CreateSlider(HWND m_hwnd, int min_range, int max_range, int v_height);

	std::string mechanismName;
	std::string chainName;
	std::string controlName;

	int minRange;
	int maxRange;

	HINSTANCE* g_hInst;

	//float controlPosition = 0.0F;
	//float lastControlPosition = 0.0F;
	
	float eeX_Position = 0.0F;
	float eeY_Position = 0.0F;
	float eeZ_Position = 0.0F;
	float eeX_Rotation = 0.0F;
	float eeY_Rotation = 0.0F;
	float eeZ_Rotation = 0.0F;

	float lastSum = 0;

	WNDPROC RegisterSubClass(HINSTANCE h_instance, LPCTSTR parent_class, LPCTSTR child_class_name, WNDPROC child_proc) {
		WNDCLASSEX  two_way_struct;
		WNDPROC		parent_wnd_proc;

		if (GetClassInfoEx(NULL, parent_class, &two_way_struct)) {
			parent_wnd_proc = two_way_struct.lpfnWndProc; // save the original message handler 

			two_way_struct.cbSize = sizeof(WNDCLASSEX); // does not always get filled properly
			two_way_struct.hInstance = h_instance;
			two_way_struct.lpszClassName = child_class_name;
			two_way_struct.lpfnWndProc = child_proc;

			/* Register the window class, and if it fails return 0 */
			if (RegisterClassEx(&two_way_struct))
				return parent_wnd_proc; // returns the parent class WndProc pointer;
									  // subclass MUST call it instead of DefWindowProc();
									  // if you do not save it, this function is wasted
		}
		return 0;
	}


	bool CheckEnter(wchar_t* v_text, int v_length)
	{
		for (int i = 0; i < v_length; i++)
			if (v_text[i] == '\r')
				return true;
		return false;
	}

	void Move(float x_pos, float y_pos, float z_pos, float x_rot, float y_rot, float z_rot);
};

