#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <commctrl.h>
#include <algorithm>
#include "../../Containers.h"
#include "../MenuHandler.h"
#include "JointControl.h"
#include "EEControl.h"



class WindowCreator
{
public:
	WindowCreator(std::shared_ptr<WindowsContainer> windows_container, CALLBACKS& v_callbacks);
	~WindowCreator();

	LRESULT MsgProc(HWND m_hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);
		
	bool Init();

	bool AddViewport(std::string viewport_name, RECT viewport_location);
	
	bool AddDropdown(std::string menu_item_name, RECT v_location, std::vector<std::string> init_list);
	bool AddCheckBox(std::string menu_item_name, RECT v_location);
	bool AddSlider(std::string menu_item_name, RECT v_location);
	bool AddButton(std::string menu_item_name, RECT v_location, std::string label_text);
	bool AddLabel(std::string menu_item_name, RECT v_location, std::string label_text);
	bool AddEdit(std::string menu_item_name, RECT v_location, std::string edit_text);
	bool AddJointControl(std::string menu_item_name, RECT v_location, std::string mechanism_name, 
		std::string chain_name, std::string joint_name, bool prismatic_joint, int min_range, int max_range);
	bool AddEEControl(std::string menu_item_name, RECT v_location, std::string mechanism_name, std::string chain_name, int min_range, int max_range);
	bool RemoveJointControls();

protected:
	//WIN32 attributes
	HWND		m_hAppWnd;
	HINSTANCE   m_hAppInstance;
	UINT		m_ClientWidth;
	UINT		m_ClientHeight;
	std::string	m_AppTitle;
	DWORD		m_WndStyle;
	std::shared_ptr<WindowsContainer> windowsContainer;

private:
	
	void CloseApp();
	bool HandleControls(HWND m_hwnd, WPARAM w_param, LPARAM l_param, int u_msg);
	bool ChangeTextCallback(std::shared_ptr<CallbackData::BlankData> callback_data);

	bool TriggerInitMenuItemsCallback(std::shared_ptr<CallbackData::BlankData> callback_data);
	bool RedrawWindowCallback(std::shared_ptr<CallbackData::BlankData> callback_data);

	int dxWinWidth;
	int dxWinHeight;

	CALLBACKS& mCallbacks;
	bool mouseLDown = false;
	int firstClick = 1; //true
};

