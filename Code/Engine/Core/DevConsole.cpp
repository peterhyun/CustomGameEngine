#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

DevConsole* g_theDevConsole = nullptr;

const Rgba8 DevConsole::ERROR = Rgba8(255, 0, 0, 255);
const Rgba8 DevConsole::WARNING = Rgba8(241, 235, 156, 255);
const Rgba8 DevConsole::INFO_MAJOR = Rgba8(151, 127, 215, 255);
const Rgba8 DevConsole::INFO_MINOR = Rgba8(156, 207, 231, 255);
const Rgba8 DevConsole::COMMAND_ECHO = Rgba8(245, 169, 203, 255);
const Rgba8 DevConsole::INPUT_TEXT = Rgba8(208, 182, 235, 255);
const Rgba8 DevConsole::INPUT_CARET = Rgba8(80, 80, 80, 255);
const Rgba8 DevConsole::BACKGROUND = Rgba8(60, 60, 60, 120);
const Rgba8 DevConsole::PARTOFYOURWORLD = Rgba8(168, 226, 220);

DevConsole::DevConsole(DevConsoleConfig const& config) : m_config(config)
{
	if (config.m_camera == nullptr || config.m_renderer == nullptr) {
		ERROR_AND_DIE("You must pass in a Camera and Renderer instance when constructing a DevConsole object");
	}
	m_caretStopwatch = new Stopwatch(0.5f);
	m_invLinesOnScreen = (1/config.m_numLinesOnScreen);
}

DevConsole::~DevConsole()
{
	delete m_caretStopwatch;
	m_caretStopwatch = nullptr;
}

void DevConsole::Startup()
{
	if (g_theEventSystem == nullptr) {
		ERROR_AND_DIE("g_theEventSystem should be instantiated before calling DevConsole::Startup()");
	}
	g_theEventSystem->SubscribeEventCallbackFunction("CharacterPressed", DevConsole::Event_CharInput);
	g_theEventSystem->SubscribeEventCallbackFunction("KeyPressed", DevConsole::Event_KeyPressed);
	g_theEventSystem->SubscribeEventCallbackFunction("help", DevConsole::Command_Help);
	g_theEventSystem->SubscribeEventCallbackFunction("clear", DevConsole::Command_Clear);
	g_theEventSystem->SubscribeEventCallbackFunction("Echo", DevConsole::Command_Echo);

	g_theEventSystem->SubscribeEventCallbackFunction("ExecuteXmlCommandScriptFile", DevConsole::Command_ExecuteXmlCommandScriptFile);

	m_config.m_renderer->CreateOrGetBitmapFont(m_config.m_fontName.c_str());
	m_caretStopwatch->Start();
}

void DevConsole::Shutdown()
{
	g_theEventSystem->UnsubscribeEventCallbackFunction("CharacterPressed", DevConsole::Event_CharInput);
	g_theEventSystem->UnsubscribeEventCallbackFunction("KeyPressed", DevConsole::Event_KeyPressed);
	g_theEventSystem->UnsubscribeEventCallbackFunction("help", DevConsole::Command_Help);
	g_theEventSystem->UnsubscribeEventCallbackFunction("clear", DevConsole::Command_Clear);
}

void DevConsole::BeginFrame()
{
	//Toggle caret visibility
	while (m_caretStopwatch->DecrementDurationIfElapsed()) {
		m_caretVisible = !m_caretVisible;
	}
}

void DevConsole::EndFrame()
{
}

void DevConsole::Execute(std::string const& consoleCommandText)
{
	/*
	//Remove all whitespaces after the first whitespace and before the double quote
	std::string consoleCommandTextCopy(consoleCommandText);
	bool firstWhiteSpaceMet = false;
	for (size_t i = 0; i < consoleCommandTextCopy.length(); i++) {
		if (consoleCommandTextCopy[i] == '"') {
			break;
		}
		else if (consoleCommandTextCopy[i] == ' ') {
			if (firstWhiteSpaceMet == false) {
				firstWhiteSpaceMet = true;
			}
			else {
				consoleCommandTextCopy.erase(i, 1); // Remove the whitespace
				i--; // Decrement i to account for the removed character
			}
		}
	}
	*/

	//Extract the commandString
	Strings commandArgumentsSeparated = SplitStringWithQuotes(consoleCommandText, ' ');
	HCIString commandString = commandArgumentsSeparated[0];

	//Check if the command is a valid command
	std::vector<HCIString> registeredCommands = g_theEventSystem->GetRegisteredCommandNames();
	if (std::find(registeredCommands.begin(), registeredCommands.end(), commandString) != registeredCommands.end()) {
		EventArgs args;
		if (commandArgumentsSeparated.size() > 1) {
			Strings argumentKeyValuePairs = SplitStringWithQuotes(commandArgumentsSeparated[1], '=');
			for (int i = 0; i < argumentKeyValuePairs.size() - 1; i += 2) {
				args.SetValue(argumentKeyValuePairs[i], argumentKeyValuePairs[i + 1]);
			}
		}
		float isEventConsumed = g_theEventSystem->FireEvent(commandString, args);
		if (isEventConsumed && commandString != "Echo") {
			g_theDevConsole->AddLine(DevConsole::COMMAND_ECHO, consoleCommandText);
		}
	}
	else {	//The current input is not registered as a valid command
		g_theDevConsole->AddLine(DevConsole::ERROR, Stringf("%s is not a registered command", commandString.c_str()));
	}

	m_commandHistory.push_back(consoleCommandText);
	if (m_commandHistory.size() > m_config.m_maxCommandHistory) {
		m_commandHistory.erase(m_commandHistory.begin());
	}
}

void DevConsole::AddLine(Rgba8 const& color, std::string const& text)
{
	m_mutex.lock();
	Strings textDelimitedWithNewLineChar = SplitStringOnDelimeter(text, '\n');
	for (int i = 0; i < textDelimitedWithNewLineChar.size() ; i++) {
		DevConsoleLine consoleLine;
		consoleLine.m_color = color;
		consoleLine.m_text = textDelimitedWithNewLineChar[i];
		m_lines.push_back(consoleLine);
	}
	m_mutex.unlock();
}

//The parameter 'bounds' is within the camera which shows the whole screen
void DevConsole::Render(AABB2 const& bounds)
{
	m_mutex.lock();
	//Don't render anything if Dev Console isn't open
	if (!m_isOpen) {
		m_mutex.unlock();
		return;
	}

	BitmapFont* devConBitmapFont = m_config.m_renderer->CreateOrGetBitmapFont(m_config.m_fontName.c_str());

	std::vector<Vertex_PCU> textVerticesForDevConsole;
	textVerticesForDevConsole.reserve(100);
	m_config.m_renderer->BeginCamera(*m_config.m_camera);
	
	std::vector<Vertex_PCU> verticesForBackgroundAndCursor;
	AddVertsForAABB2(verticesForBackgroundAndCursor, bounds, DevConsole::BACKGROUND);
	
	Vec2 devConsoleDimensions = bounds.GetDimensions();
	//First determine from which m_line index is visible in the AABB2 bounds
	float lineHeight = devConsoleDimensions.y * m_invLinesOnScreen;
	int numLinesAboveInputTextToBeRendered = ((int)m_config.m_numLinesOnScreen - 1 > m_lines.size() ) ? (int)m_lines.size() : (int)m_config.m_numLinesOnScreen - 1;

	//TODO: Calculate AABB2 for each line to call and call AddVertsForTextInBox2D accordingly
	for (int i = 0; i < numLinesAboveInputTextToBeRendered; i++) {
		AABB2 aabbForThisLine = bounds;
		aabbForThisLine.m_mins.y += lineHeight * (numLinesAboveInputTextToBeRendered - i);
		aabbForThisLine.m_maxs.y = aabbForThisLine.m_mins.y + lineHeight;
		DevConsoleLine lineToRender = m_lines[m_lines.size() - numLinesAboveInputTextToBeRendered + i];
		devConBitmapFont->AddVertsForTextInBox2D(textVerticesForDevConsole, aabbForThisLine, lineHeight, lineToRender.m_text, lineToRender.m_color, m_config.m_fontAspect, Vec2(0.0f, 0.0f), TextBoxMode::OVERRUN);
	}

	//Don't forget to AddVertsForTextInBox2D here for m_inputText!
	devConBitmapFont->AddVertsForTextInBox2D(
		textVerticesForDevConsole,
		AABB2(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.x, bounds.m_mins.y + lineHeight), 
		lineHeight,
		m_inputText,
		DevConsole::INPUT_TEXT,
		m_config.m_fontAspect,
		Vec2(0.0f, 0.0f),
		TextBoxMode::OVERRUN
	);

	if (m_caretVisible) {
		float singleCharacterFontWidth = lineHeight * m_config.m_fontAspect;
		Vec2 caretStartPosition = Vec2(bounds.m_mins.x + (float)m_caretPosition * singleCharacterFontWidth, bounds.m_mins.y);
		Vec2 caretEndPosition = caretStartPosition + Vec2(0.001f, lineHeight);
		AddVertsForLineSegment2D(verticesForBackgroundAndCursor, caretStartPosition, caretEndPosition, 4.0f, DevConsole::INPUT_CARET);
	}

	m_config.m_renderer->BindShader(nullptr);
	m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
	m_config.m_renderer->SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_DISABLED);
	m_config.m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	m_config.m_renderer->SetModelConstants(Mat44(), Rgba8::WHITE);
	m_config.m_renderer->BindTexture(nullptr);
	m_config.m_renderer->DrawVertexArray((int)verticesForBackgroundAndCursor.size(), verticesForBackgroundAndCursor.data());
	m_config.m_renderer->BindTexture(&devConBitmapFont->GetTexture());
	m_config.m_renderer->DrawVertexArray((int)textVerticesForDevConsole.size(), textVerticesForDevConsole.data());
	m_config.m_renderer->EndCamera(*m_config.m_camera);

	m_mutex.unlock();
}

void DevConsole::ToggleOpen()
{
	m_isOpen = !m_isOpen;
}

bool DevConsole::IsOpen()
{
	return m_isOpen;
}

void DevConsole::ExecuteXmlCommandScriptNode(const XmlElement& commandScriptXmlElement)
{
	const XmlElement* commandToExecute = commandScriptXmlElement.FirstChildElement();
	while (commandToExecute != nullptr) {
		std::string command;
		command += commandToExecute->Name();
		command += ' ';
		
		const XmlAttribute* attribute = commandToExecute->FirstAttribute();
		while (attribute != nullptr) {
			command += attribute->Name();
			command += '=';
			command += '\"';
			command += attribute->Value();
			command += '\"';
			command += ' ';
			attribute = attribute->Next();
		}
		Execute(command);
		commandToExecute = commandToExecute->NextSiblingElement();
	}
}

void DevConsole::ExecuteXmlCommandScriptFile(const std::string& commandScriptXmlFilePathName)
{
	XmlDocument xmldoc;
	if (xmldoc.LoadFile(commandScriptXmlFilePathName.c_str()) == XmlResult::XML_SUCCESS) {
		XmlElement* rootElementPtr = xmldoc.RootElement();
		ExecuteXmlCommandScriptNode(*rootElementPtr);
	}
	else {
		ERROR_RECOVERABLE(Stringf("XmlFile: %s was not loaded"));
	}
}

bool DevConsole::Event_KeyPressed(EventArgs& args)
{
	if (g_theDevConsole == nullptr) {
		ERROR_AND_DIE("Should instantiate g_theDevConsole before calling DevConsole::Event_KeyPressed()");
	}
	if (g_theEventSystem == nullptr) {
		ERROR_AND_DIE("Should instantiate g_theEventSystem before calling DevConsole::Event_KeyPressed()");
	}
	unsigned char keyCode = args.GetValue("KeyCode", (unsigned char)-1);
	if (keyCode == KEYCODE_TILDE) {
		g_theDevConsole->ToggleOpen();
	}

	//Do not accept any key inputs if dev console is NOT open. Do not consume the event
	if (!g_theDevConsole->m_isOpen) {
		return false;
	}

	if (keyCode == KEYCODE_ESC) {
		if (g_theDevConsole->m_inputText.size() > 0) {
			g_theDevConsole->m_inputText.clear();
			g_theDevConsole->m_caretPosition = 0;
		}
		else {
			g_theDevConsole->m_isOpen = false;
		}
	}
	else if (keyCode == KEYCODE_ENTER) {
		if (g_theDevConsole->m_inputText.size() > 0) {
			g_theDevConsole->Execute(g_theDevConsole->m_inputText);
			g_theDevConsole->m_inputText.clear();
			g_theDevConsole->m_historyIndex = -1;
			g_theDevConsole->m_caretPosition = 0;
		}
		else {
			g_theDevConsole->m_isOpen = false;
		}
	}
	else if (keyCode == KEYCODE_LEFTARROW) {
		if(g_theDevConsole->m_caretPosition > 0)
			g_theDevConsole->m_caretPosition--;
	}
	else if (keyCode == KEYCODE_RIGHTARROW) {
		if (g_theDevConsole->m_caretPosition < g_theDevConsole->m_inputText.size()) {
			g_theDevConsole->m_caretPosition++;
		}
	}
	else if (keyCode == KEYCODE_UPARROW) {
		if (g_theDevConsole->m_historyIndex == -1) {
			g_theDevConsole->m_historyIndex = (int)g_theDevConsole->m_commandHistory.size();
		}
		if (g_theDevConsole->m_historyIndex > 0) {
			g_theDevConsole->m_historyIndex--;
			g_theDevConsole->m_inputText = g_theDevConsole->m_commandHistory[g_theDevConsole->m_historyIndex];
			g_theDevConsole->m_caretPosition = (int)g_theDevConsole->m_inputText.size();
		}
	}
	else if (keyCode == KEYCODE_DOWNARROW) {
		if (g_theDevConsole->m_historyIndex <= (int)g_theDevConsole->m_commandHistory.size() - 1) {
			g_theDevConsole->m_historyIndex++;
			if (g_theDevConsole->m_historyIndex != (int)g_theDevConsole->m_commandHistory.size()) {
				g_theDevConsole->m_inputText = g_theDevConsole->m_commandHistory[g_theDevConsole->m_historyIndex];
			}
			else {	//Edge case
				g_theDevConsole->m_inputText.clear();
			}
			g_theDevConsole->m_caretPosition = (int)g_theDevConsole->m_inputText.size();
		}
	}
	else if (keyCode == KEYCODE_HOME) {
		g_theDevConsole->m_caretPosition = 0;
	}
	else if (keyCode == KEYCODE_END) {
		g_theDevConsole->m_caretPosition = (int)g_theDevConsole->m_inputText.size();
	}
	else if (keyCode == KEYCODE_DELETE) {
		if(g_theDevConsole->m_caretPosition < g_theDevConsole->m_inputText.size())
			g_theDevConsole->m_inputText.erase(g_theDevConsole->m_caretPosition, 1);
	}
	else if (keyCode == KEYCODE_BACKSPACE) {
		if (g_theDevConsole->m_caretPosition > 0) {
			g_theDevConsole->m_inputText.erase(g_theDevConsole->m_caretPosition - 1, 1);
			g_theDevConsole->m_caretPosition--;
		}
	}

	g_theDevConsole->m_caretVisible = true;
	g_theDevConsole->m_caretStopwatch->Restart();
	return true;
}

bool DevConsole::Event_CharInput(EventArgs& args)
{
	if (g_theDevConsole == nullptr) {
		ERROR_AND_DIE("Should instantiate g_theDevConsole before calling DevConsole::Event_CharInput()");
	}
	//Do NOT accept any char inputs if dev console isn't open
	if (!g_theDevConsole->m_isOpen) {
		return false;
	}
	unsigned char charInput = args.GetValue("CharInput", (unsigned char)-1);
	if ((charInput >= 32) && (charInput <= 126) && (charInput != KEYCODE_TILDE) && (charInput != '`')) {
		g_theDevConsole->m_inputText.insert(g_theDevConsole->m_caretPosition, 1, charInput);
		g_theDevConsole->m_caretPosition++;
	}
	return true;
}

bool DevConsole::Command_Clear(EventArgs& args)
{
	//Apparently we don't need EventArgs for Clear and Help right now. Maybe add support for it later.
	UNUSED(args);
	if (g_theDevConsole == nullptr) {
		ERROR_AND_DIE("Should instantiate g_theDevConsole before calling DevConsole::Command_Clear()");
	}
	g_theDevConsole->m_lines.clear();
	return true;
}

bool DevConsole::Command_Help(EventArgs& args)
{
	UNUSED(args);
	if (g_theDevConsole == nullptr) {
		ERROR_AND_DIE("Should instantiate g_theDevConsole before calling DevConsole::Command_Help()");
	}
	g_theDevConsole->AddLine(DevConsole::INFO_MINOR, "Registered Commands:");
	std::vector<HCIString> registeredCommands = g_theEventSystem->GetRegisteredCommandNames();
	for(int i = 0 ; i < registeredCommands.size() ; i++){
		g_theDevConsole->AddLine(DevConsole::INFO_MINOR, registeredCommands[i]);
	}
	return true;
}

bool DevConsole::Command_Echo(EventArgs& args)
{
	if (g_theDevConsole == nullptr) {
		ERROR_AND_DIE("Should instantiate g_theDevConsole before calling DevConsole::Command_Help()");
	}
	std::string message = args.GetValue("Message", std::string(""));	//Without std::string, compiler cannot deduce whether T should be std::string or const char *
	TrimString(message, '"');
	g_theDevConsole->AddLine(DevConsole::PARTOFYOURWORLD, message);
	return true;
}

bool DevConsole::Command_ExecuteXmlCommandScriptFile(EventArgs& args)
{
	if (g_theDevConsole == nullptr) {
		ERROR_AND_DIE("Should instantiate g_theDevConsole before calling DevConsole::Command_ExecuteXmlCommandScriptFile()");
	}
	std::string filePath = args.GetValue("File", std::string(""));	//Without std::string, compiler cannot deduce whether T should be std::string or const char *

	if (DoesFileExistOnDisk(filePath)) {
		g_theDevConsole->ExecuteXmlCommandScriptFile(filePath);
		return true;
	}
	else {
		return false;
	}
}
