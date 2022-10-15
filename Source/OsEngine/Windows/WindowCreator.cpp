#include "WindowCreator.h"


namespace
{
	//Forwards mesgs to User Defined Proc function
	WindowCreator* g_pApp = nullptr;
}

WindowCreator::WindowCreator(std::shared_ptr<WindowsContainer> windows_container, CALLBACKS& v_callbacks):
	windowsContainer(windows_container),
	mCallbacks(v_callbacks)
{
	m_hAppWnd = NULL;
	m_AppTitle = "Mechatronic Manager";
	m_WndStyle = WS_OVERLAPPEDWINDOW;
	g_pApp = this;
}


WindowCreator::~WindowCreator()
{
}

LRESULT CALLBACK MainWndProc(HWND m_hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{
	if (g_pApp)
		return g_pApp->MsgProc(m_hwnd, u_msg, w_param, l_param);
	else
		return DefWindowProc(m_hwnd, u_msg, w_param, l_param);
}

bool WindowCreator::Init()
{
	//WNDCLASSEX
	WNDCLASSEX w_cex;
	ZeroMemory(&w_cex, sizeof(WNDCLASSEX));
	w_cex.cbClsExtra = 0;
	w_cex.cbWndExtra = 0;
	w_cex.cbSize = sizeof(WNDCLASSEX);
	w_cex.style = CS_HREDRAW | CS_VREDRAW;
	w_cex.hInstance = windowsContainer->hInstance;
	w_cex.lpfnWndProc = MainWndProc;
	w_cex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	w_cex.hCursor = LoadCursor(NULL, IDC_ARROW);
	w_cex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	w_cex.lpszMenuName = NULL; //CHANGE LATER TO HAVE A MENU
	w_cex.lpszClassName = L"RenderManagerWNDCLASS";
	w_cex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&w_cex))
	{
		OutputDebugString(L"FAILED TO CREATE WINDOW CLASS\n");
		return false;
	}

	RECT v_r = { 0,0, windowsContainer->clientWidth,  windowsContainer->clientHeight };
	AdjustWindowRect(&v_r, m_WndStyle, FALSE);
	UINT v_width = v_r.right - v_r.left;
	UINT v_height = v_r.bottom - v_r.top;

	UINT v_x = GetSystemMetrics(SM_CXSCREEN) / 2 - v_width / 2;
	UINT v_y = GetSystemMetrics(SM_CYSCREEN) / 2 - v_height / 2;


	m_hAppWnd = CreateWindow(L"RenderManagerWNDCLASS", TOWSTRING(m_AppTitle).c_str(), m_WndStyle, v_x, v_y, v_width, v_height, NULL, NULL, windowsContainer->hInstance, NULL);

	
	if (!m_hAppWnd)
	{
		OutputDebugString(L"FAILED TO CREATE WINDOW\n");
		return false;
	}

	ShowWindow(m_hAppWnd, SW_SHOW);

	windowsContainer->mainWindow = m_hAppWnd;


	mCallbacks["TriggerInitMenuItemsCallback"] = std::bind(&WindowCreator::TriggerInitMenuItemsCallback, this, std::placeholders::_1);  //CALLBACK INIT
	mCallbacks["RedrawWindowCallback"] = std::bind(&WindowCreator::RedrawWindowCallback, this, std::placeholders::_1);  //CALLBACK INIT

	return true;
}

bool WindowCreator::AddViewport(std::string viewport_name, RECT viewport_location)
{
	//Child Window to Draw Viewport
	dxWinWidth = viewport_location.right;// -viewport_location.left;
	dxWinHeight = viewport_location.bottom;// -viewport_location.top;
	std::shared_ptr<WindowContainer> viewport_ptr = std::make_shared<WindowContainer>();
	viewport_ptr->viewportWindow = CreateWindowEx(NULL, L"Static", NULL, WS_CHILD | WS_VISIBLE, viewport_location.left, viewport_location.top, dxWinWidth, dxWinHeight, m_hAppWnd, (HMENU)1, NULL, NULL);
	viewport_ptr->viewportWidth = dxWinWidth;
	viewport_ptr->viewportHeight = dxWinHeight;
	windowsContainer->viewportWindows.insert(std::map<std::string, std::shared_ptr<WindowContainer>>::value_type(viewport_name, viewport_ptr));

	return true;
}


bool WindowCreator::AddDropdown(std::string menu_item_name, RECT v_location, std::vector<std::string> init_list)
{
	std::shared_ptr<HWND> menu_item_ptr = std::make_shared<HWND>();
	std::string item_type = "ComboBox";
	*menu_item_ptr = CreateWindowEx(NULL, TOLPCWSTR(item_type), L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, v_location.left, v_location.top, v_location.right, v_location.bottom, m_hAppWnd, (HMENU)1, NULL, NULL);
	SendDlgItemMessage(*menu_item_ptr, 1, CB_RESETCONTENT, 0, 0);
	for(auto& item_name : init_list)
		SendMessage(*menu_item_ptr, CB_ADDSTRING, 0, (LPARAM)TOLPCWSTR(item_name));
	SendMessage(*menu_item_ptr, CB_SETCURSEL, 0, 0); //highlight/select the first option
	windowsContainer->menuItems.insert(std::map<std::string, std::pair<std::string, std::shared_ptr<CustomControl>>>::value_type(menu_item_name, std::make_pair(item_type,std::make_shared<CustomControl>(menu_item_ptr))));
	return true;
}

bool WindowCreator::AddCheckBox(std::string menu_item_name, RECT v_location)
{
	std::shared_ptr<HWND> menu_item_ptr = std::make_shared<HWND>();
	std::string item_type = "CheckBox";
	*menu_item_ptr = CreateWindowEx(NULL, L"BUTTON", L"CheckBox", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, v_location.left, v_location.top, v_location.right, v_location.bottom, m_hAppWnd,	(HMENU)222, NULL, NULL);
	windowsContainer->menuItems.insert(std::map<std::string, std::pair<std::string, std::shared_ptr<CustomControl>>>::value_type(menu_item_name, std::make_pair(item_type, std::make_shared<CustomControl>(menu_item_ptr))));
	return true;
}

bool WindowCreator::AddSlider(std::string menu_item_name, RECT v_location)
{
	std::shared_ptr<HWND> menu_item_ptr = std::make_shared<HWND>();
	std::string item_type = "Trackbar Control";
	*menu_item_ptr = CreateWindowEx(0,TRACKBAR_CLASS, TOLPCWSTR(item_type),	WS_CHILD |WS_VISIBLE |TBS_AUTOTICKS |TBS_ENABLESELRANGE,v_location.left, v_location.top, v_location.right, v_location.bottom, m_hAppWnd, (HMENU)1, NULL, NULL);	
	windowsContainer->menuItems.insert(std::map<std::string, std::pair<std::string, std::shared_ptr<CustomControl>>>::value_type(menu_item_name, std::make_pair(item_type, std::make_shared<CustomControl>(menu_item_ptr))));
	return true;
}

bool WindowCreator::AddButton(std::string menu_item_name, RECT v_location, std::string label_text)
{
	std::shared_ptr<HWND> menu_item_ptr = std::make_shared<HWND>();
	std::string item_type = "Button";
	*menu_item_ptr = CreateWindowEx(0, L"BUTTON", TOLPCWSTR(label_text), BS_DEFPUSHBUTTON | WS_VISIBLE | WS_BORDER | WS_CHILD, v_location.left, v_location.top, v_location.right, v_location.bottom, m_hAppWnd, (HMENU)1, NULL, NULL);
	windowsContainer->menuItems.insert(std::map<std::string, std::pair<std::string, std::shared_ptr<CustomControl>>>::value_type(menu_item_name, std::make_pair(item_type, std::make_shared<CustomControl>(menu_item_ptr))));
	mCallbacks[menu_item_name + "_change_text"] = std::bind(&WindowCreator::ChangeTextCallback, this, std::placeholders::_1);  //CALLBACK INIT
	return true;
}

bool WindowCreator::AddLabel(std::string menu_item_name, RECT v_location, std::string label_text)
{
	std::shared_ptr<HWND> menu_item_ptr = std::make_shared<HWND>();
	std::string item_type = "Label";
	*menu_item_ptr = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING, L"Static", TOLPCWSTR(label_text), WS_CHILDWINDOW | WS_VISIBLE | SS_RIGHT, v_location.left, v_location.top, v_location.right, v_location.bottom, m_hAppWnd, NULL, NULL, NULL);	
	windowsContainer->menuItems.insert(std::map<std::string, std::pair<std::string, std::shared_ptr<CustomControl>>>::value_type(menu_item_name, std::make_pair(item_type, std::make_shared<CustomControl>(menu_item_ptr))));
	//Add a callback for changing label 
	mCallbacks[menu_item_name + "_change_text"] = std::bind(&WindowCreator::ChangeTextCallback, this, std::placeholders::_1);  //CALLBACK INIT
	return true;
}

bool WindowCreator::AddEdit(std::string menu_item_name, RECT v_location, std::string edit_text)
{
	std::shared_ptr<HWND> menu_item_ptr = std::make_shared<HWND>();
	std::string item_type = "Edit";
	*menu_item_ptr = CreateWindowEx(WS_EX_CLIENTEDGE, L"Edit", TOLPCWSTR(edit_text), WS_CHILDWINDOW | WS_VISIBLE | ES_NUMBER, v_location.left, v_location.top, v_location.right, v_location.bottom, m_hAppWnd, NULL, NULL, NULL);
	windowsContainer->menuItems.insert(std::map<std::string, std::pair<std::string, std::shared_ptr<CustomControl>>>::value_type(menu_item_name, std::make_pair(item_type, std::make_shared<CustomControl>(menu_item_ptr))));
	//Add a callback for changing label 
	mCallbacks[menu_item_name + "_change_text"] = std::bind(&WindowCreator::ChangeTextCallback, this, std::placeholders::_1);  //CALLBACK INIT
	return true;
}

bool WindowCreator::AddJointControl(std::string menu_item_name, RECT v_location, std::string mechanism_name, std::string chain_name, std::string joint_name, bool prismatic_joint, int min_range, int max_range)
{
	std::shared_ptr<HWND> menu_item_ptr = std::make_shared<HWND>();
	std::shared_ptr<JointControl> custom_control = std::make_shared<JointControl>(mCallbacks, menu_item_ptr, mechanism_name, chain_name, joint_name, prismatic_joint, min_range, max_range);
	custom_control->CreateControl(m_hAppWnd, &m_hAppInstance, v_location.left, v_location.top);
	windowsContainer->menuItems.insert(std::map<std::string, std::pair<std::string, std::shared_ptr<CustomControl>>>::value_type(menu_item_name, std::make_pair("Joint Control", custom_control)));
	return true;
}

bool WindowCreator::AddEEControl(std::string menu_item_name, RECT v_location, std::string mechanism_name, std::string chain_name, int min_range, int max_range)
{
	std::shared_ptr<HWND> menu_item_ptr = std::make_shared<HWND>();
	std::shared_ptr<EEControl> custom_control = std::make_shared<EEControl>(mCallbacks, menu_item_ptr, mechanism_name, chain_name, min_range, max_range);
	custom_control->CreateControl(m_hAppWnd, &m_hAppInstance, v_location.left, v_location.top);
	windowsContainer->menuItems.insert(std::map<std::string, std::pair<std::string, std::shared_ptr<CustomControl>>>::value_type(menu_item_name, std::make_pair("EE Control", custom_control)));
	return true;
}

bool WindowCreator::RemoveJointControls()
{
	for (std::map<std::string, std::pair<std::string, std::shared_ptr<CustomControl>>>::iterator it = windowsContainer->menuItems.begin(); it != windowsContainer->menuItems.end();)
	{
		if ((it->second.first) == "Joint Control")
		{
			it = windowsContainer->menuItems.erase(it);
		}
		else if ((it->second.first) == "EE Control")
		{
			it = windowsContainer->menuItems.erase(it);
		}
		else
		{
			it++;
		}
	}
	return true;
}

void WindowCreator::CloseApp()
{
}

LRESULT WindowCreator::MsgProc(HWND m_hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{
	switch (u_msg)
	{
	case WM_CREATE:
	{
		break;
	}
	case WM_KEYDOWN:
	{
		break;
	}
	case WM_COMMAND:
	{
		HandleControls(m_hwnd, w_param, l_param, WM_COMMAND);
		break;
	}
	case WM_CLOSE:
	{
		DestroyWindow(m_hwnd);
		break;
	}
	case WM_DESTROY:
	{
		CloseApp();
		PostQuitMessage(0);
		return 0;
	}
	case WM_NOTIFY:
	{
		break;
	}
	//case WM_KILLFOCUS:
	//{
	//	SetFocus(m_hAppWnd);
	//	break;
	//}
	case WM_LBUTTONDOWN:
	{
		mouseLDown = true;
		break;
	}
	case WM_LBUTTONUP:
	{
		mouseLDown = false;
		firstClick = 1;
		break;
	}
	case WM_MOUSEMOVE:
	{
		if (mouseLDown)
		{
			SetFocus(m_hAppWnd);
			//CALLBACK EVENT
			if (mCallbacks.find("Mouse") != mCallbacks.end())
			{
				std::shared_ptr<CallbackData::Mouse> mouse_position = std::make_shared<CallbackData::Mouse>((int)LOWORD(l_param), (int)HIWORD(l_param), 0, firstClick);
				mCallbacks.at("Mouse")(std::dynamic_pointer_cast<CallbackData::BlankData>(mouse_position));
			}
			firstClick = 0;
		}
		break;
	}

	case WM_MOUSEWHEEL:
	{
		short z_delta = GET_WHEEL_DELTA_WPARAM(w_param);		
		SetFocus(m_hAppWnd);
		//CALLBACK EVENT
		if (mCallbacks.find("MouseWheel") != mCallbacks.end())
		{
			std::shared_ptr<CallbackData::Mouse> mouse_position = std::make_shared<CallbackData::Mouse>(0, 0, z_delta, 0);
			mCallbacks.at("MouseWheel")(std::dynamic_pointer_cast<CallbackData::BlankData>(mouse_position));
		}
		break;
	}

	case WM_HSCROLL:
	{
		HandleControls(m_hwnd, w_param, l_param, WM_HSCROLL);
		break;
	}

	case WM_PAINT:
	{
		//CALLBACK EVENT
		if (mCallbacks.find("DisplayCameraImage") != mCallbacks.end())
			mCallbacks.at("DisplayCameraImage")(std::shared_ptr<CallbackData::BlankData>());
	}

	default:
		return DefWindowProc(m_hwnd, u_msg, w_param, l_param);
	}
}

bool WindowCreator::HandleControls(HWND m_hwnd, WPARAM w_param, LPARAM l_param, int u_msg)
{
	HWND menu_item_hwnd = GetFocus();
	if (menu_item_hwnd == NULL)
		return false; 
	auto v_it = std::find_if(windowsContainer->menuItems.begin(), windowsContainer->menuItems.end(),
		[menu_item_hwnd](const std::pair<std::string, std::pair<std::string, std::shared_ptr<CustomControl>>> & t) -> bool {
			return *t.second.second->menuItem == menu_item_hwnd;
		}
	);

	if (v_it == windowsContainer->menuItems.end())
		return false;
	
	std::string menu_item_name = v_it->first;
	std::string item_type = v_it->second.first;

	if(u_msg == WM_COMMAND)  //Drop Down Box
	{
		if (item_type == "ComboBox")
		{
			if (HIWORD(w_param) == CBN_SELCHANGE)
			{
				//Get the name of the drop down value selected
				//CALLBACK EVENT
				if (mCallbacks.find(menu_item_name + "_selchange") != mCallbacks.end())
				{
					char selected_value[50];
					GetWindowTextA(menu_item_hwnd, selected_value, 50);
					std::string selected_value_text(selected_value);
					std::shared_ptr<CallbackData::ComboBoxEntry> combo_box_data = std::make_shared<CallbackData::ComboBoxEntry>(menu_item_name, selected_value_text);
					mCallbacks.at(menu_item_name + "_selchange")(std::dynamic_pointer_cast<CallbackData::BlankData>(combo_box_data));
					SetFocus(NULL);
				}
			}
			else if (HIWORD(w_param) == CBN_DROPDOWN) 
			{
				if (mCallbacks.find(menu_item_name + "_dropdown") != mCallbacks.end())
				{
					std::shared_ptr<CallbackData::ComboBoxList> combo_box_data = std::make_shared<CallbackData::ComboBoxList>(menu_item_name);
					mCallbacks.at(menu_item_name + "_dropdown")(std::dynamic_pointer_cast<CallbackData::BlankData>(combo_box_data));
					//Combine with event to update combo box list on combo box open (no need to separate this)

					SendMessage(menu_item_hwnd, CB_DELETESTRING, (WPARAM)0, (LPARAM)0); //Clear combo box

					for (auto& v_item : combo_box_data->comboItems)
						SendMessage(menu_item_hwnd, CB_ADDSTRING, 0, (LPARAM)v_item.c_str());

					SetFocus(NULL);
				}
			}
		}
		else if (item_type == "CheckBox" && (LOWORD(w_param) == 222 /*IDB_CHECKBOX*/) && (HIWORD(w_param) == BN_CLICKED))  //Checkbox
		{
			bool selected = SendDlgItemMessage(m_hwnd, 222, BM_GETCHECK, 0, 0);
			// CALLBACK EVENT
			if (mCallbacks.find(menu_item_name) != mCallbacks.end())
			{
				std::shared_ptr<CallbackData::CheckBox> checkbox_data = std::make_shared<CallbackData::CheckBox>(menu_item_name, selected);
				mCallbacks.at(menu_item_name)(std::dynamic_pointer_cast<CallbackData::BlankData>(checkbox_data));
				SetFocus(NULL);
			}
		}

		else if (item_type == "Button" && HIWORD(w_param) == BN_CLICKED)
		{
			// CALLBACK EVENT
			if (mCallbacks.find(menu_item_name) != mCallbacks.end())
			{
				std::shared_ptr<CallbackData::Button> button_data = std::make_shared<CallbackData::Button>(menu_item_name);
				mCallbacks.at(menu_item_name)(std::dynamic_pointer_cast<CallbackData::BlankData>(button_data));
				SetFocus(NULL);
			}
		}
		return true;
	}
	else if ((LOWORD(w_param) == TB_THUMBTRACK || LOWORD(w_param) == TB_ENDTRACK) && u_msg == WM_HSCROLL)  //Slider
	{ 	
		//float slider_val = (float)HIWORD(w_param)/100.0F;
		float slider_val = SendMessage(menu_item_hwnd, TBM_GETPOS, 0, 0) / 100.0F;
		// CALLBACK EVENT
		if (mCallbacks.find(menu_item_name) != mCallbacks.end())
		{
			std::shared_ptr<CallbackData::Slider> slider_data = std::make_shared<CallbackData::Slider>(menu_item_name, slider_val);
			mCallbacks.at(menu_item_name)(std::dynamic_pointer_cast<CallbackData::BlankData>(slider_data));
			SetFocus(NULL);
		}
	}
	else
		return false;
}

//CALLBACK FUNCTION
bool WindowCreator::ChangeTextCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::Text> item_data = std::dynamic_pointer_cast<CallbackData::Text>(callback_data);

	if (item_data == nullptr)
		return false;

	if (windowsContainer->menuItems.find(item_data->itemName) != windowsContainer->menuItems.end())
	{
		std::shared_ptr<HWND> menu_item = windowsContainer->menuItems.at(item_data->itemName).second->menuItem;
		std::string item_type = windowsContainer->menuItems.at(item_data->itemName).first;
		if (/*item_type == "Label" && */menu_item != nullptr)
		{
			SetWindowText(*menu_item, TOLPCWSTR(item_data->itemText));

			//ShowWindow(*menu_item, SW_HIDE);
			//ShowWindow(*menu_item, SW_SHOW);
		}
	}

	return true;
}

//CALLBACK FUNCTION
bool WindowCreator::TriggerInitMenuItemsCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	for (auto& menu_item : windowsContainer->menuItems)
	{
		auto hwnd_ptr = menu_item.second.second->menuItem;
		std::string item_type = menu_item.second.first;
		int ctrl_id = GetDlgCtrlID(*hwnd_ptr);
		if (hwnd_ptr != nullptr && item_type == "ComboBox")
		{
			SetFocus(*hwnd_ptr);
			SendMessageA(m_hAppWnd, WM_COMMAND, MAKEWPARAM(ctrl_id, CBN_SELCHANGE), (LPARAM)GetDlgItem(*hwnd_ptr, ctrl_id));
		}
	}

	return true;
}

//CALLBACK CALLBACK
bool WindowCreator::RedrawWindowCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	RedrawWindow(m_hAppWnd, NULL, NULL, RDW_INVALIDATE | RDW_INTERNALPAINT);
	//InvalidateRect(m_hAppWnd, NULL, TRUE);
	//RedrawWindow(m_hAppWnd, NULL, NULL, RDW_INTERNALPAINT);
	return true;
}