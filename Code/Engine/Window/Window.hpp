#pragma once
#include "Engine/Math/IntVec2.hpp"
#include <string>

struct Vec2;

struct WindowConfig
{
	std::string m_windowTitle = "Protogame2D";
	float m_clientAspect = 2.0f;
	bool m_isFullscreen = false;
	bool m_showThiccFrame = false;
	IntVec2 m_windowSize;
	IntVec2 m_windowPosition;
};

class Window
{
public:
	Window(WindowConfig const& config);
	~Window();

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	WindowConfig const& GetConfig() const;
	static Window* GetWindowContext();

	void* GetHwnd() const;
	IntVec2 GetClientDimensions() const;

	void* GetForegroundWindow() const;

	bool GetXMLFileName(std::string& out_fileName, unsigned char keycodePressed = unsigned char(-1)) const;
	bool GetFBXFileName(std::string& out_fileName, unsigned char keycodePressed = unsigned char(-1)) const;
	bool GetGHCSFileName(std::string& out_fileName, unsigned char keycodePressed = unsigned char(-1)) const;
	bool GetPNGFileName(std::string& out_fileName, unsigned char keycodePressed = unsigned char(-1)) const;
	bool GetBVHFileName(std::string& out_fileName, unsigned char keycodePressed = unsigned char(-1)) const;

	bool GetDirectoryPath(std::string& out_directoryPath, unsigned char keycodePressed = unsigned char(-1)) const;
	std::string GetCurrentDirectoryName() const;

	bool IsFullScreen() const;

	static std::string PasteTextFromClipboard();

protected:
	void CreateOSWindow();
	void RunMessagePump();

protected:
	WindowConfig m_config;
	static Window* s_mainWindow;

	void* m_hwnd;
	float m_clientWidth;
	float m_clientHeight;
};