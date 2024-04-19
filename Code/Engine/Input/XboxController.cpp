#include "Engine/Input/XboxController.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput9_1_0")

constexpr float MAX_JOYSTICKRAWVAL = 32768.f;
constexpr float INV_MAX_JOYSTICKRAWVAL = (1.0 / MAX_JOYSTICKRAWVAL);
constexpr float NORMALIZED_INNER_DEAD_ZONE = 0.3f;
constexpr float NORMALIZED_OUTER_DEAD_ZONE = 0.95f;

XboxController::XboxController()
{
	m_leftStick.SetDeadZoneThresholds(NORMALIZED_INNER_DEAD_ZONE, NORMALIZED_OUTER_DEAD_ZONE);
	m_rightStick.SetDeadZoneThresholds(NORMALIZED_INNER_DEAD_ZONE, NORMALIZED_OUTER_DEAD_ZONE);
}

XboxController::~XboxController()
{
}

bool XboxController::IsConnected() const
{
	return m_isConnected;
}

int XboxController::GetControllerID() const
{
	return m_id;
}

AnalogJoystick const& XboxController::GetLeftStick() const
{
	return m_leftStick;
}

AnalogJoystick const& XboxController::GetRightStick() const
{
	return m_rightStick;
}

float XboxController::GetLeftTrigger() const
{
	return m_leftTrigger;
}

float XboxController::GetRightTrigger() const
{
	return m_rightTrigger;
}

KeyButtonState const& XboxController::GetButton(XboxButtonID buttonID) const
{
	return m_buttons[(int)buttonID];
}

bool XboxController::IsButtonDown(XboxButtonID buttonID) const
{
	return m_buttons[(int)buttonID].m_pressedThisFrame;
}

bool XboxController::WasButtonJustPressed(XboxButtonID buttonID) const
{
	return (!m_buttons[(int)buttonID].m_pressedLastFrame) && m_buttons[(int)buttonID].m_pressedThisFrame;
}

bool XboxController::WasButtonJustReleased(XboxButtonID buttonID) const
{
	return m_buttons[(int)buttonID].m_pressedLastFrame && (!m_buttons[(int)buttonID].m_pressedThisFrame);
}

void XboxController::Update()
{
	for (int buttonIndex = 0; buttonIndex < (int)XboxButtonID::NUM_XBOX_BUTTONS; buttonIndex++) {
		m_buttons[buttonIndex].m_pressedLastFrame = m_buttons[buttonIndex].m_pressedThisFrame;
	}
	XINPUT_STATE xboxControllerState = {};
	DWORD errorStatus = XInputGetState(m_id, &xboxControllerState);
	if (errorStatus == ERROR_SUCCESS)
	{
		/*
		DebuggerPrintf("XBox controller #%d reports:                                        \n", m_id);
		DebuggerPrintf("  wButtons=%5d (0x%08x), bLeftTrigger=%3d, bRightTrigger=%3d        \n",
			xboxControllerState.Gamepad.wButtons, xboxControllerState.Gamepad.wButtons,
			xboxControllerState.Gamepad.bLeftTrigger, xboxControllerState.Gamepad.bRightTrigger);
		DebuggerPrintf("  sThumbLX=%6d    sThumbLY=%6d                                \n", xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY);
		DebuggerPrintf("  sThumbRX=%6d    sThumbRY=%6d                                \n", xboxControllerState.Gamepad.sThumbRX, xboxControllerState.Gamepad.sThumbRY);

		bool isButtonADown = (xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) == XINPUT_GAMEPAD_A;
		DebuggerPrintf("Button A is %s                                                      \n", isButtonADown ? "DOWN" : "UP");
		*/
		UpdateJoystick(m_leftStick, xboxControllerState.Gamepad.sThumbLX, xboxControllerState.Gamepad.sThumbLY);
		UpdateJoystick(m_rightStick, xboxControllerState.Gamepad.sThumbRX, xboxControllerState.Gamepad.sThumbRY);
		UpdateTrigger(m_leftTrigger, xboxControllerState.Gamepad.bLeftTrigger);
		UpdateTrigger(m_rightTrigger, xboxControllerState.Gamepad.bRightTrigger);
		for (int buttonID = (int)XboxButtonID::XBOX_DPAD_UP; buttonID < (int)XboxButtonID::NUM_XBOX_BUTTONS; buttonID++) {
			//determine button flag
			switch ((XboxButtonID)buttonID) {
				case XboxButtonID::XBOX_DPAD_UP:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP);
					break;
				case XboxButtonID::XBOX_DPAD_DOWN:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
					break;
				case XboxButtonID::XBOX_DPAD_LEFT:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
					break;
				case XboxButtonID::XBOX_DPAD_RIGHT:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);
					break;
				case XboxButtonID::XBOX_START:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_START);
					break;
				case XboxButtonID::XBOX_SELECT:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_BACK);
					break;
				case XboxButtonID::XBOX_LEFT_THUMB:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB);
					break;
				case XboxButtonID::XBOX_RIGHT_THUMB:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB);
					break;
				case XboxButtonID::XBOX_LEFT_SHOULDER:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
					break;
				case XboxButtonID::XBOX_RIGHT_SHOULDER:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
					break;
				case XboxButtonID::XBOX_A:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_A);
					break;
				case XboxButtonID::XBOX_B:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_B);
					break;
				case XboxButtonID::XBOX_X:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_X);
					break;
				case XboxButtonID::XBOX_Y:
					UpdateButton((XboxButtonID)buttonID, xboxControllerState.Gamepad.wButtons, XINPUT_GAMEPAD_Y);
					break;
				default:
					break;
			}
		}
	}
	else if (errorStatus == ERROR_DEVICE_NOT_CONNECTED)
	{
		Reset();
	}
	else
	{
		/*
		DebuggerPrintf("Xbox controller #%i reports error code %u (0x%08x).                  \n", m_id, errorStatus, errorStatus);
		*/
	}
}

void XboxController::Reset()
{
	m_leftStick.Reset();
	m_rightStick.Reset();
	m_leftTrigger = 0.f;
	m_rightTrigger = 0.f;
	for (int buttonIndex = 0; buttonIndex < (int)XboxButtonID::NUM_XBOX_BUTTONS; buttonIndex++) {
		m_buttons[buttonIndex].m_pressedThisFrame = false;
		m_buttons[buttonIndex].m_pressedLastFrame = false;
	}
}

void XboxController::UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY)
{
	out_joystick.UpdatePosition(rawX * INV_MAX_JOYSTICKRAWVAL, rawY * INV_MAX_JOYSTICKRAWVAL);
}

void XboxController::UpdateTrigger(float& out_triggerValue, unsigned char rawValue)
{
	out_triggerValue = rawValue / static_cast<float>(MAXBYTE);
}

//buttonFlags should be State.wButton
//buttonFlag is XINPUT_GAMEPAD_A defined in Xinput.h; You need to extract info from buttonFlags
void XboxController::UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag)
{
	m_buttons[(int)buttonID].m_pressedThisFrame = ((buttonFlags & buttonFlag) == buttonFlag);
	/*
	if ((buttonFlags & buttonFlag) == buttonFlag) {
		DebuggerPrintf("buttonFlags: (0x%08x)\n", buttonFlags);
		DebuggerPrintf("buttonFlag: (0x%08x)\n", buttonFlag);
		DebuggerPrintf("buttonID: %d\n", (int)buttonID);
	}
	*/
}
