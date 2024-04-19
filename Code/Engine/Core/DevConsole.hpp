#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <string>
#include <vector>
#include <mutex>

class Renderer;
class Camera;
class BitmapFont;
class Stopwatch;
struct AABB2;
class DevConsole;
class NamedProperties;

extern DevConsole* g_theDevConsole;

typedef NamedProperties EventArgs;

//Stores the text and color for an individual line of text
struct DevConsoleLine
{
	Rgba8 m_color;
	std::string m_text;
};

//Dev console defaults. A Renderer and Camera must be provided
struct DevConsoleConfig
{
	Renderer* m_renderer = nullptr;
	Camera* m_camera = nullptr;
	std::string m_fontName = "Data/Fonts/SquirrelFixedFont";
	float m_fontAspect = 0.7f;
	float m_numLinesOnScreen = 40.0f;
	int m_maxCommandHistory = 128;
};

class DevConsole
{
public:
	DevConsole(DevConsoleConfig const& config);
	~DevConsole();

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void Execute(std::string const& consoleCommandText);
	void AddLine(Rgba8 const& color, std::string const& text);
	void Render(AABB2 const& bounds);
	                                                                                                                                       
	void ToggleOpen();
	bool IsOpen();

	void ExecuteXmlCommandScriptFile(const std::string& commandScriptXmlFilePathName);

	static const Rgba8 ERROR;
	static const Rgba8 WARNING;
	static const Rgba8 INFO_MAJOR;
	static const Rgba8 INFO_MINOR;
	static const Rgba8 COMMAND_ECHO;
	static const Rgba8 INPUT_TEXT;
	static const Rgba8 INPUT_CARET;
	static const Rgba8 BACKGROUND;
	static const Rgba8 PARTOFYOURWORLD;

	static bool Event_KeyPressed(EventArgs& args);
	static bool Event_CharInput(EventArgs& args);
	static bool Command_Clear(EventArgs& args);
	static bool Command_Help(EventArgs& args);
	static bool Command_Echo(EventArgs& args);
	static bool Command_ExecuteXmlCommandScriptFile(EventArgs& args);

protected:
	void ExecuteXmlCommandScriptNode(const XmlElement& commandScriptXmlElement);

protected:
	DevConsoleConfig m_config;
	bool m_isOpen = false;
	std::vector<DevConsoleLine> m_lines;
	std::string m_inputText;
	int m_caretPosition = 0;
	bool m_caretVisible = true;
	Stopwatch* m_caretStopwatch = nullptr;
	std::vector<std::string> m_commandHistory;
	int m_historyIndex = -1;
	float m_invLinesOnScreen = 0.0f;

	std::mutex m_mutex;
};