#pragma once
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Math/IntVec2.hpp"

extern unsigned char const KEYCODE_F1;
extern unsigned char const KEYCODE_F2;
extern unsigned char const KEYCODE_F3;
extern unsigned char const KEYCODE_F4;
extern unsigned char const KEYCODE_F5;
extern unsigned char const KEYCODE_F6;
extern unsigned char const KEYCODE_F7;
extern unsigned char const KEYCODE_F8;
extern unsigned char const KEYCODE_F9;
extern unsigned char const KEYCODE_F10;
extern unsigned char const KEYCODE_F11;
extern unsigned char const KEYCODE_ESC;
extern unsigned char const KEYCODE_SPACE;
extern unsigned char const KEYCODE_ENTER;
extern unsigned char const KEYCODE_LMB;
extern unsigned char const KEYCODE_RMB;
extern unsigned char const KEYCODE_MMB;
extern unsigned char const KEYCODE_UPARROW;
extern unsigned char const KEYCODE_DOWNARROW;
extern unsigned char const KEYCODE_RIGHTARROW;
extern unsigned char const KEYCODE_LEFTARROW;
extern unsigned char const KEYCODE_BACKSPACE;
extern unsigned char const KEYCODE_SHIFT;
//For DevConsole support
extern unsigned char const KEYCODE_TILDE;
extern unsigned char const KEYCODE_UPARROW;
extern unsigned char const KEYCODE_DOWNARROW;
extern unsigned char const KEYCODE_LEFTARROW;
extern unsigned char const KEYCODE_RIGHTARROW;
extern unsigned char const KEYCODE_ENTER;
extern unsigned char const KEYCODE_BACKSPACE;
extern unsigned char const KEYCODE_INSERT;
extern unsigned char const KEYCODE_DELETE;
extern unsigned char const KEYCODE_HOME;
extern unsigned char const KEYCODE_END;

extern unsigned char const KEYCODE_LEFTSQUAREBRACKET;
extern unsigned char const KEYCODE_RIGHTSQUAREBRACKET;

extern unsigned char const KEYCODE_COMMA;
extern unsigned char const KEYCODE_PERIOD;
extern unsigned char const KEYCODE_SEMICOLON;
extern unsigned char const KEYCODE_SINGLEQUOTE;

extern unsigned char const KEYCODE_CONTROL;

extern unsigned char const KEYCODE_ALT;

extern unsigned char const KEYCODE_TAB;

constexpr int NUM_KEYCODES = 256;
constexpr int NUM_XBOX_CONTROLLERS = 4;

class NamedProperties;
typedef NamedProperties EventArgs;

class InputSystem;

extern InputSystem* g_theInput;

struct InputSystemConfig
{

};

struct MouseState
{
	IntVec2 m_cursorClientPosition;
	IntVec2 m_cursorClientDelta;

	bool m_currentHidden = false;
	bool m_desiredHidden = false;

	bool m_currentRelative = false;
	bool m_desiredRelative = false;
};

class InputSystem {
public:
	InputSystem( InputSystemConfig const& config);
	~InputSystem();

	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_KeyReleased(EventArgs& args);
	static bool Event_WheelScrolled(EventArgs& args);

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();
	bool WasKeyJustPressed(unsigned char keyCode);
	bool WasKeyJustReleased(unsigned char keyCode);
	bool IsKeyDown(unsigned char keyCode);
	bool HandleKeyPressed(unsigned char keyCode);
	bool HandleKeyReleased(unsigned char keyCode);
	bool HandleWheelScrolled(unsigned int wheelScroll);
	bool WasWheelScrolled(int* out_mouseScrollAmount = nullptr) const;
	XboxController const& GetController(int controllerID);

	void SetCursorMode(bool hidden, bool relative);
	bool IsCursorHidden() const;
	bool IsCursorVisible() const;
	IntVec2 GetCursorClientDelta() const;
	IntVec2 GetCursorClientPos() const;
	Vec2 GetNormalizedCursorPos() const;

protected:
	KeyButtonState m_keyStates[NUM_KEYCODES];
	XboxController m_controllers[NUM_XBOX_CONTROLLERS];
	InputSystemConfig m_config;
	MouseState m_mouseState;
	int m_wheelScrollAmount = 0;
};