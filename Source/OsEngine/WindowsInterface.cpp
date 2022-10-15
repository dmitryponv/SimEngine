#include "WindowsInterface.h"



WindowsInterface::WindowsInterface(std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks):
	windowsContainer(std::dynamic_pointer_cast<WindowsContainer>(os_container)),
	windowCreator(windowsContainer, v_callbacks)
{
}


WindowsInterface::~WindowsInterface()
{
	// Remove the windows.
	for (auto& v_window : windowsContainer->viewportWindows)
	{
		if (v_window.second != nullptr)
		{
			DestroyWindow(v_window.second->viewportWindow);
			v_window.second->viewportWindow = NULL;
		}
	}
}

bool WindowsInterface::Init()
{
	if(!windowCreator.Init())
		return false;
	return true;
}

bool WindowsInterface::AddViewport(std::string viewport_name, RECT viewport_location)
{
	if (!windowCreator.AddViewport(viewport_name, viewport_location))
		return false;
	return true;
}

bool WindowsInterface::AddDropdown(std::string menu_item_name, RECT v_location, std::vector<std::string> init_list)
{
	bool ret_val = true;
	if (!windowCreator.AddDropdown(menu_item_name, v_location, init_list))
		ret_val = false;
	return ret_val;
}

bool WindowsInterface::AddLabel(std::string menu_item_name, RECT v_location, std::string label_text)
{
	bool ret_val = true;
	if (!windowCreator.AddLabel(menu_item_name, v_location, label_text))
		ret_val = false;
	return ret_val;
}

bool WindowsInterface::AddSlider(std::string menu_item_name, RECT v_location)
{
	bool ret_val = true;
	if (!windowCreator.AddSlider(menu_item_name, v_location))
		ret_val = false;
	return ret_val;
}

bool WindowsInterface::AddButton(std::string menu_item_name, RECT v_location, std::string button_text)
{
	bool ret_val = true;
	if (!windowCreator.AddButton(menu_item_name, v_location, button_text))
		ret_val = false;
	return ret_val;
}

bool WindowsInterface::AddCheckBox(std::string menu_item_name, RECT v_location)
{
	bool ret_val = true;
	if (!windowCreator.AddCheckBox(menu_item_name, v_location))
		ret_val = false;
	return ret_val;
}

bool WindowsInterface::AddEdit(std::string menu_item_name, RECT v_location, std::string edit_text)
{
	bool ret_val = true;
	if (!windowCreator.AddEdit(menu_item_name, v_location, edit_text))
		ret_val = false;
	return ret_val;
}

bool WindowsInterface::AddJointControl(std::string menu_item_name, RECT v_location, std::string mechanism_name,
	std::string chain_name, std::string joint_name, bool prismatic_joint, int min_range, int max_range)
{
	bool ret_val = true;
	if (!windowCreator.AddJointControl(menu_item_name, v_location, mechanism_name, chain_name, joint_name, prismatic_joint, min_range, max_range))
		ret_val = false;
	return ret_val;
}

bool WindowsInterface::AddEEControl(std::string menu_item_name, RECT v_location, std::string mechanism_name,
	std::string chain_name, int min_range, int max_range)
{
	bool ret_val = true;
	if (!windowCreator.AddEEControl(menu_item_name, v_location, mechanism_name, chain_name, min_range, max_range))
		ret_val = false;
	return ret_val;
}

bool WindowsInterface::RemoveJointControls()
{
	bool ret_val = true;
	if (!windowCreator.RemoveJointControls())
		ret_val = false;
	return ret_val;
}
