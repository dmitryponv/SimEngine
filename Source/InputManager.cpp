#include "InputManager.h"

InputManager::InputManager(std::shared_ptr<BlankContainer> os_container, CALLBACKS& v_callbacks):
	osContainer(os_container),
	mCallbacks(v_callbacks)
{
}

InputManager::~InputManager()
{
}

bool InputManager::Init()
{
	mCallbacks["Mouse"] = std::bind(&InputManager::MouseCallback, this, std::placeholders::_1);   //CALLBACK INIT
	mCallbacks["MouseWheel"] = std::bind(&InputManager::MouseWheelCallback, this, std::placeholders::_1);   //CALLBACK INIT
	mCallbacks["Keyboard"] = std::bind(&InputManager::KeyboardCallback, this, std::placeholders::_1);   //CALLBACK INIT
	return true;
}

//CALLBACK FUNCTION
bool InputManager::MouseCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{

	std::shared_ptr<CallbackData::Mouse> mouse_data = std::dynamic_pointer_cast<CallbackData::Mouse>(callback_data);
	
	if (mouse_data == nullptr)
		return false;

	int mouse_x_pos = mouse_data->posX;
	int mouse_y_pos = mouse_data->posY;
	int first_click = mouse_data->firstClick;
	DebugOut("Mouse Pos: %d %d\r\n", mouse_x_pos, mouse_y_pos);

	if (first_click == 0)
	{
		float d_x = mouse_x_pos - mouseXposPrev;
		float d_y = mouse_y_pos - mouseYposPrev;
		float d_w = sqrt(d_x*d_x + d_y*d_y);
		const float scale = 0.01;

		Quaternion key_quat = Quaternion(std::make_pair(Vector4(0.0F , d_y * scale, d_x * scale), d_w * scale));

		if (key_quat.Undefined())
			return false;

		auto axis_angle = key_quat.ToAxisAngle();
		osContainer->camMoveMatrix = osContainer->camMoveMatrix * Matrix4x4::CreateFromAxisAngle(axis_angle);

		DebugOut("Rot Determinant = %f, ", osContainer->camMoveMatrix.DetRotation());
	}
	mouseXposPrev = mouse_x_pos;
	mouseYposPrev = mouse_y_pos;
	return true;
}

//CALLBACK FUNCTION
bool InputManager::KeyboardCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::Keyboard> keyboard_data = std::dynamic_pointer_cast<CallbackData::Keyboard>(callback_data);

	if (keyboard_data == nullptr)
		return false;

	if (keyboard_data->kbData[DIK_D] & 0x80)
	{
		osContainer->camMoveMatrix = Matrix4x4::CreateTranslation(1.0F, 0.0F, 0.0F) *  osContainer->camMoveMatrix;
		return true;
	}
	if (keyboard_data->kbData[DIK_A] & 0x80)
	{
		osContainer->camMoveMatrix = Matrix4x4::CreateTranslation(-1.0F, 0.0F, 0.0F) *  osContainer->camMoveMatrix;
		return true;
	}
	if (keyboard_data->kbData[DIK_W] & 0x80)
	{
		osContainer->camMoveMatrix = Matrix4x4::CreateTranslation(0.0F, 1.0F, 0.0F) *  osContainer->camMoveMatrix;
		return true;
	}
	if (keyboard_data->kbData[DIK_S] & 0x80)
	{
		osContainer->camMoveMatrix = Matrix4x4::CreateTranslation(0.0F, -1.0F, 0.0F) *  osContainer->camMoveMatrix;
		return true;
	}
	if (keyboard_data->kbData[DIK_LSHIFT] & 0x80)
	{
		osContainer->camMoveMatrix = Matrix4x4::CreateTranslation(0.0F, 0.0F, 1.0F) *  osContainer->camMoveMatrix;
		return true;
	}
	if (keyboard_data->kbData[DIK_LCONTROL] & 0x80)
	{
		osContainer->camMoveMatrix = Matrix4x4::CreateTranslation(0.0F, 0.0F, -1.0F) *  osContainer->camMoveMatrix;
		return true;
	}
	return true;
}

//CALLBACK FUNCTION
bool InputManager::MouseWheelCallback(std::shared_ptr<CallbackData::BlankData> callback_data)
{
	std::shared_ptr<CallbackData::Mouse> mouse_data = std::dynamic_pointer_cast<CallbackData::Mouse>(callback_data);
	if (mouse_data == nullptr)
		return false;

	if (mouse_data->wheelZ == 120)
	{
		osContainer->camMoveMatrix = Matrix4x4::CreateRotX(5) * osContainer->camMoveMatrix;
		//osContainer->camMoveMatrix = Matrix4x4::CreateTranslation(0.0F, 0.01F, -0.01F) * osContainer->camMoveMatrix;
		return true;
	}
	else if (mouse_data->wheelZ == -120)
	{
		osContainer->camMoveMatrix = Matrix4x4::CreateRotX(-5) * osContainer->camMoveMatrix;
		//osContainer->camMoveMatrix = Matrix4x4::CreateTranslation(0.0F, -0.01F, 0.01F) * osContainer->camMoveMatrix;
		return true;
	}
	return true;
}

