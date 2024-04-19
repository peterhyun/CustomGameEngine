#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include <Windows.h>

const unsigned char KEYCODE_F1 = VK_F1;
const unsigned char KEYCODE_F2 = VK_F2;
const unsigned char KEYCODE_F3 = VK_F3;
const unsigned char KEYCODE_F4 = VK_F4;
const unsigned char KEYCODE_F5 = VK_F5;
const unsigned char KEYCODE_F6 = VK_F6;
const unsigned char KEYCODE_F7 = VK_F7;
const unsigned char KEYCODE_F8 = VK_F8;
const unsigned char KEYCODE_F9 = VK_F9;
const unsigned char KEYCODE_F10 = VK_F10;
const unsigned char KEYCODE_F11 = VK_F11;
const unsigned char KEYCODE_ESC = VK_ESCAPE;
const unsigned char KEYCODE_SPACE = VK_SPACE;
const unsigned char KEYCODE_LMB = VK_LBUTTON;
const unsigned char KEYCODE_RMB = VK_RBUTTON;
const unsigned char KEYCODE_MMB = VK_MBUTTON;
const unsigned char KEYCODE_SHIFT = VK_SHIFT;
//For DevConsole support
const unsigned char KEYCODE_TILDE = 0xC0;
const unsigned char KEYCODE_UPARROW = VK_UP;
const unsigned char KEYCODE_DOWNARROW = VK_DOWN;
const unsigned char KEYCODE_LEFTARROW = VK_LEFT;
const unsigned char KEYCODE_RIGHTARROW = VK_RIGHT;
const unsigned char KEYCODE_ENTER = VK_RETURN;
const unsigned char KEYCODE_BACKSPACE = VK_BACK;
const unsigned char KEYCODE_INSERT = VK_INSERT;
const unsigned char KEYCODE_DELETE = VK_DELETE;
const unsigned char KEYCODE_HOME = VK_HOME;
const unsigned char KEYCODE_END = VK_END;

const unsigned char KEYCODE_LEFTSQUAREBRACKET = VK_OEM_4;
const unsigned char KEYCODE_RIGHTSQUAREBRACKET = VK_OEM_6;

const unsigned char KEYCODE_COMMA = VK_OEM_COMMA;
const unsigned char KEYCODE_PERIOD = VK_OEM_PERIOD;
const unsigned char KEYCODE_SEMICOLON = VK_OEM_1;
const unsigned char KEYCODE_SINGLEQUOTE = VK_OEM_7;

const unsigned char KEYCODE_CONTROL = VK_CONTROL;

const unsigned char KEYCODE_ALT = VK_MENU;

const unsigned char KEYCODE_TAB = VK_TAB;

InputSystem* g_theInput = nullptr;

InputSystem::InputSystem(InputSystemConfig const& config):
	m_config(config)
{
	for (int keyIdx = 0; keyIdx < NUM_KEYCODES; keyIdx++) {
		m_keyStates[keyIdx].m_pressedThisFrame = false;
		m_keyStates[keyIdx].m_pressedLastFrame = false;
	}
	for (int ctrlIdx = 0; ctrlIdx < NUM_XBOX_CONTROLLERS; ctrlIdx++) {
		m_controllers[ctrlIdx].m_id = ctrlIdx;
	}
}

InputSystem::~InputSystem()
{
}

bool InputSystem::Event_KeyPressed(EventArgs& args)
{
	if (g_theInput == nullptr) {
		ERROR_AND_DIE("You need to instantiate g_theInput before calling InputSystem::Event_KeyPressed()");
	}
	unsigned char keyCode = args.GetValue("KeyCode", (unsigned char)-1);
	return g_theInput->HandleKeyPressed(keyCode);
}

bool InputSystem::Event_KeyReleased(EventArgs& args)
{
	if (g_theInput == nullptr) {
		ERROR_AND_DIE("You need to instantiate g_theInput before calling InputSystem::Event_KeyReleased()");
	}
	unsigned char keyCode = args.GetValue("KeyCode", (unsigned char)-1);
	return g_theInput->HandleKeyReleased(keyCode);
}

bool InputSystem::Event_WheelScrolled(EventArgs& args)
{
	if (g_theInput == nullptr) {
		ERROR_AND_DIE("You need to instantiate g_theInput before calling InputSystem::Event_WheelScrolled()");
	}
	int wheelScroll = args.GetValue("WheelScroll", 0);
	return g_theInput->HandleWheelScrolled(wheelScroll);
}

void InputSystem::Startup()
{
	g_theEventSystem->SubscribeEventCallbackFunction("KeyPressed", InputSystem::Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFunction("KeyReleased", InputSystem::Event_KeyReleased);
	g_theEventSystem->SubscribeEventCallbackFunction("WheelScrolled", InputSystem::Event_WheelScrolled);
}

void InputSystem::Shutdown()
{
}

void InputSystem::BeginFrame()
{
	for (int ctrlIdx = 0; ctrlIdx < NUM_XBOX_CONTROLLERS; ctrlIdx++) {
		m_controllers[ctrlIdx].Update();	//Controllers should be updated here!
	}

	if (m_mouseState.m_desiredHidden != m_mouseState.m_currentHidden) {
		m_mouseState.m_currentHidden = m_mouseState.m_desiredHidden;
		if (m_mouseState.m_currentHidden) {
			while (::ShowCursor(false) >= 0) {};
		}
		else {
			while (::ShowCursor(true) < 0) {};
		}
	}

	//Setup general variables for mouse cursor
	HWND currentWindowHandle = ::GetActiveWindow();
	RECT clientRectangle;
	::GetClientRect(currentWindowHandle, &clientRectangle);
	POINT centerOfClient;
	centerOfClient.x = (clientRectangle.right + clientRectangle.left) / 2;
	centerOfClient.y = (clientRectangle.top + clientRectangle.bottom) / 2;

	//Get our current cursor client position this frame
	POINT clientCursorPos;
	::GetCursorPos(&clientCursorPos);
	::ScreenToClient(currentWindowHandle, &clientCursorPos);

	//Zero our cursor client delta if our relative mode has changed
	if (m_mouseState.m_desiredRelative != m_mouseState.m_currentRelative) {
		m_mouseState.m_currentRelative = m_mouseState.m_desiredRelative;
		m_mouseState.m_cursorClientDelta = IntVec2::ZERO;
		if (m_mouseState.m_currentRelative) {
			POINT centerOfClientScreenCoords = centerOfClient;
			::ClientToScreen(currentWindowHandle, &centerOfClientScreenCoords);	
			if (::SetCursorPos(centerOfClientScreenCoords.x, centerOfClientScreenCoords.y)) {
				clientCursorPos = centerOfClient;
				m_mouseState.m_cursorClientPosition = IntVec2(centerOfClient.x, centerOfClient.y);
			}
			else {
				::GetCursorPos(&clientCursorPos);
				::ScreenToClient(currentWindowHandle, &clientCursorPos);
				m_mouseState.m_cursorClientPosition = IntVec2(clientCursorPos.x, clientCursorPos.y);
			}
		}
	}
	
	//If we are in relative mode, calculate and store the cursor delta and reset the cursor client position to the center of the window client region
	if (m_mouseState.m_currentRelative) {
		m_mouseState.m_cursorClientDelta = IntVec2(clientCursorPos.x, clientCursorPos.y) - m_mouseState.m_cursorClientPosition;

		POINT centerOfClientScreenCoords = centerOfClient;
		::ClientToScreen(currentWindowHandle, &centerOfClientScreenCoords);
		if (::SetCursorPos(centerOfClientScreenCoords.x, centerOfClientScreenCoords.y)) {
			m_mouseState.m_cursorClientPosition = IntVec2(centerOfClient.x, centerOfClient.y);
		}
		else {
			::GetCursorPos(&clientCursorPos);
			::ScreenToClient(currentWindowHandle, &clientCursorPos);
			m_mouseState.m_cursorClientPosition = IntVec2(clientCursorPos.x, clientCursorPos.y);
		}
	}
	else {	//Supporting non-relative modes
		m_mouseState.m_cursorClientDelta = IntVec2(0, 0);
		m_mouseState.m_cursorClientPosition = IntVec2(clientCursorPos.x, clientCursorPos.y);
	}
}

void InputSystem::EndFrame()
{
	for (int keyIdx = 0; keyIdx < NUM_KEYCODES; keyIdx++) {
		m_keyStates[keyIdx].m_pressedLastFrame = m_keyStates[keyIdx].m_pressedThisFrame;
	}
	m_wheelScrollAmount = 0;
}

bool InputSystem::WasKeyJustPressed(unsigned char keyCode)
{
	return (m_keyStates[keyCode].m_pressedThisFrame) && (!m_keyStates[keyCode].m_pressedLastFrame);
}

bool InputSystem::WasKeyJustReleased(unsigned char keyCode)
{
	return (!m_keyStates[keyCode].m_pressedThisFrame) && (m_keyStates[keyCode].m_pressedLastFrame);
}

bool InputSystem::IsKeyDown(unsigned char keyCode)
{
	return m_keyStates[keyCode].m_pressedThisFrame;
}

bool InputSystem::HandleKeyPressed(unsigned char keyCode)
{
	if (keyCode < NUM_KEYCODES) {
		m_keyStates[keyCode].m_pressedThisFrame = true;
		return true;
	}
	ERROR_RECOVERABLE(Stringf("KeyCode %d is not handled in HandleKeyPressed", keyCode));
	return false;
}

bool InputSystem::HandleKeyReleased(unsigned char keyCode)
{
	if (keyCode < NUM_KEYCODES) {
		m_keyStates[keyCode].m_pressedThisFrame = false;
		return true;
	}
	ERROR_RECOVERABLE(Stringf("KeyCode %d is not handled in HandleKeyReleased", keyCode));
	return false;
}

bool InputSystem::HandleWheelScrolled(unsigned int wheelScrollAmount)
{
	m_wheelScrollAmount = wheelScrollAmount;
	return true;
}

bool InputSystem::WasWheelScrolled(int* out_wheelScrollAmount) const
{
	if (m_wheelScrollAmount == 0) {
		return false;
	}
	if (out_wheelScrollAmount) {
		*out_wheelScrollAmount = m_wheelScrollAmount;
	}
	return true;
}

XboxController const& InputSystem::GetController(int controllerID)
{
	return m_controllers[controllerID];
}

void InputSystem::SetCursorMode(bool hidden, bool relative)
{
	m_mouseState.m_desiredHidden = hidden;
	m_mouseState.m_desiredRelative = relative;
}

bool InputSystem::IsCursorHidden() const
{
	CURSORINFO cursorInfo = { sizeof(CURSORINFO) };

	if (GetCursorInfo(&cursorInfo)) {
		// Check if the cursor is hidden (invisible)
		return (cursorInfo.flags & CURSOR_SHOWING) == 0;
	}

	// If GetCursorInfo fails, assume the cursor is not hidden
	ERROR_RECOVERABLE("GetCursorInfo() failed in IsCursorHidden()");
	return false;
}

bool InputSystem::IsCursorVisible() const
{
	CURSORINFO cursorInfo = { sizeof(CURSORINFO) };

	if (GetCursorInfo(&cursorInfo)) {
		// Check if the cursor is hidden (invisible)
		return (cursorInfo.flags & CURSOR_SHOWING) != 0;
	}

	// If GetCursorInfo fails, assume the cursor is not hidden
	ERROR_RECOVERABLE("GetCursorInfo() failed in IsCursorVisible()");
	return false;
}

IntVec2 InputSystem::GetCursorClientDelta() const
{
	return m_mouseState.m_cursorClientDelta;
}

IntVec2 InputSystem::GetCursorClientPos() const
{
	return m_mouseState.m_cursorClientPosition;
}

Vec2 InputSystem::GetNormalizedCursorPos() const
{
	RECT clientRectangle;
	::GetClientRect(::GetActiveWindow(), &clientRectangle);

	if (clientRectangle.right == clientRectangle.left || clientRectangle.bottom == clientRectangle.top) {	//Avoid Nans
		return Vec2(0.0f, 0.0f);
	}

	float cursorNormalizedX = float(m_mouseState.m_cursorClientPosition.x) / float(clientRectangle.right - clientRectangle.left);
	float cursorNormalizedY = float(m_mouseState.m_cursorClientPosition.y) / float(clientRectangle.bottom - clientRectangle.top);
	return Vec2(cursorNormalizedX, 1.f - cursorNormalizedY);
}
