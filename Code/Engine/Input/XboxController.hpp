#pragma once
#include "Engine/Input/AnalogJoystick.hpp"
#include "Engine/Input/KeyButtonState.hpp"

enum class XboxButtonID {
	XBOX_BUTTON_INVALID = -1,
	XBOX_DPAD_UP = 0,
	XBOX_DPAD_DOWN = 1,
	XBOX_DPAD_LEFT = 2,
	XBOX_DPAD_RIGHT = 3,
	XBOX_START = 4,
	XBOX_SELECT = 5,
	XBOX_LEFT_THUMB = 6,
	XBOX_RIGHT_THUMB = 7,
	XBOX_LEFT_SHOULDER = 8,
	XBOX_RIGHT_SHOULDER = 9,
	XBOX_A = 10,
	XBOX_B = 11,
	XBOX_X = 12,
	XBOX_Y = 13,
	NUM_XBOX_BUTTONS = 14
};

class XboxController {
	friend class InputSystem;

public:
	XboxController();
	~XboxController();
	bool IsConnected() const;
	int GetControllerID() const;
	AnalogJoystick const& GetLeftStick() const;
	AnalogJoystick const& GetRightStick() const;
	float GetLeftTrigger() const;
	float GetRightTrigger() const;
	KeyButtonState const& GetButton(XboxButtonID buttonID) const;
	bool IsButtonDown(XboxButtonID buttonID) const;
	bool WasButtonJustPressed(XboxButtonID buttonID) const;
	bool WasButtonJustReleased(XboxButtonID buttonID) const;

private:
	void Update();
	void Reset();
	void UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY);
	void UpdateTrigger(float& out_triggerValue, unsigned char rawValue);
	void UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag);

private:
	int m_id = -1;
	bool m_isConnected = false;
	float m_leftTrigger = 0.f;
	float m_rightTrigger = 0.f;
	KeyButtonState m_buttons[(int)XboxButtonID::NUM_XBOX_BUTTONS];
	AnalogJoystick m_leftStick;
	AnalogJoystick m_rightStick;
};