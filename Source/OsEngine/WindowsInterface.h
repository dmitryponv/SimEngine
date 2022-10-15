#pragma once
#include "Windows/WindowCreator.h"
#include "../Containers.h"


class WindowsInterface
{
public:
	WindowsInterface(std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks);
	~WindowsInterface();

	bool Init();

	bool AddViewport(std::string viewport_name, RECT viewport_location);	
	bool AddDropdown(std::string menu_item_name, RECT v_location, std::vector<std::string> init_list);
	bool AddLabel(std::string menu_item_name, RECT v_location, std::string label_text);
	bool AddSlider(std::string menu_item_name, RECT v_location);
	bool AddButton(std::string menu_item_name, RECT v_location, std::string button_text);
	bool AddCheckBox(std::string menu_item_name, RECT v_location);
	bool AddEdit(std::string menu_item_name, RECT v_location, std::string edit_text);
	bool AddJointControl(std::string menu_item_name, RECT v_location, std::string mechanism_name,
		std::string chain_name, std::string joint_name, bool prismatic_joint, int min_range, int max_range);
	bool AddEEControl(std::string menu_item_name, RECT v_location, std::string mechanism_name,
		std::string chain_name, int min_range, int max_range);
	bool RemoveJointControls();

private:

	std::shared_ptr<WindowsContainer> windowsContainer;
	WindowCreator windowCreator;

};

