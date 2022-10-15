#pragma once
#include <windows.h>
#include <string> 
#include <memory>

class CustomControl
{
public:

	CustomControl(std::shared_ptr<HWND> menu_item):
		menuItem(menu_item)
	{};

	std::shared_ptr<HWND> menuItem = NULL;
	
	//bool CreateControl(HWND m_hwnd, HINSTANCE* m_happ_instance, int v_right, int v_top);
	//
	//
	//// this static method is registered with your window class
	//static LRESULT CALLBACK CustomDialogProcStatic(HWND m_hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
	//{
	//	auto pThis = (CustomControl*)GetWindowLongPtr(m_hwnd, GWLP_USERDATA);
	//	if (pThis != NULL)
	//		return pThis->CustomDialogProc(m_hwnd, u_msg, w_param, l_param);
	//	return DefWindowProc(m_hwnd, u_msg, w_param, l_param);
	//}
	//
	//LRESULT CALLBACK CustomDialogProc(HWND m_hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);

private:


	//HWND hwndCustom;
	//
	//HWND hControl = NULL;  //Handles to the controls.
	//HWND hwndLabel = NULL;
	//
	//WNDPROC RegisterSubClass(HINSTANCE h_instance, LPCTSTR parent_class, LPCTSTR child_class_name, WNDPROC child_proc) {
	//	WNDCLASSEX  two_way_struct;
	//	WNDPROC		parent_wnd_proc;
	//
	//	if (GetClassInfoEx(NULL, parent_class, &two_way_struct)) {
	//		parent_wnd_proc = two_way_struct.lpfnWndProc; // save the original message handler 
	//
	//		two_way_struct.cbSize = sizeof(WNDCLASSEX); // does not always get filled properly
	//		two_way_struct.hInstance = h_instance;
	//		two_way_struct.lpszClassName = child_class_name;
	//		two_way_struct.lpfnWndProc = child_proc;
	//
	//		/* Register the window class, and if it fails return 0 */
	//		if (RegisterClassEx(&two_way_struct))
	//			return parent_wnd_proc; // returns the parent class WndProc pointer;
	//								  // subclass MUST call it instead of DefWindowProc();
	//								  // if you do not save it, this function is wasted
	//	}
	//	return 0;
	//}
};

