#include "Engine/Window/Window.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/EngineBuildPreferences.hpp"

#ifdef ENGINE_ENABLE_IMGUI
#include "ThirdParty/imgui/imgui_impl_win32.h"
#endif

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <commdlg.h>
#include <shlobj.h>

Window* Window::s_mainWindow = nullptr;

#ifdef ENGINE_ENABLE_IMGUI
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

LRESULT CALLBACK WindowsMessageHandlingProcedure(HWND windowHandle, UINT wmMessageCode, WPARAM wParam, LPARAM lParam)
{
	Window* windowContext = Window::GetWindowContext();
	GUARANTEE_OR_DIE(windowContext != nullptr, "WindowContext was null!");

	switch (wmMessageCode) {
		case WM_CLOSE:
		{
			g_theEventSystem->FireEvent("quit");
			return 0;
		}
		case WM_LBUTTONDOWN:
		{
#ifdef ENGINE_ENABLE_IMGUI
			if (ImGui_ImplWin32_WndProcHandler(windowHandle, wmMessageCode, wParam, lParam) == 0 && ImGui::GetIO().WantCaptureMouse)
				return 0;
#endif
			EventArgs args;
			args.SetValue("KeyCode", KEYCODE_LMB);
			g_theEventSystem->FireEvent("KeyPressed", args);
			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			EventArgs args;
			args.SetValue("KeyCode", KEYCODE_RMB);
			g_theEventSystem->FireEvent("KeyPressed", args);
			return 0;
		}
		case WM_MBUTTONDOWN:
		{
			EventArgs args;
			args.SetValue("KeyCode", KEYCODE_MMB);
			g_theEventSystem->FireEvent("KeyPressed", args);
			return 0;
		}
		case WM_KEYDOWN:
		{
#ifdef ENGINE_ENABLE_IMGUI
			if (ImGui_ImplWin32_WndProcHandler(windowHandle, wmMessageCode, wParam, lParam) == 0 && ImGui::GetIO().WantCaptureMouse)
				return 0;
#endif
			EventArgs args;
			args.SetValue("KeyCode", (unsigned char)wParam);
			g_theEventSystem->FireEvent("KeyPressed", args);
			return 0;
		}
		case WM_LBUTTONUP: //Mouse keydown/keyup works different from keyboards. They produce different wParams for keydown/keyup so you have to come up with your own args for mouse events.
		{
#ifdef ENGINE_ENABLE_IMGUI
			if (ImGui_ImplWin32_WndProcHandler(windowHandle, wmMessageCode, wParam, lParam) == 0 && ImGui::GetIO().WantCaptureMouse)
				return 0;
#endif
			EventArgs args;
			args.SetValue("KeyCode", KEYCODE_LMB);
			g_theEventSystem->FireEvent("KeyReleased", args);
			return 0;
		}
		case WM_RBUTTONUP:
		{
			EventArgs args;
			args.SetValue("KeyCode", KEYCODE_RMB);
			g_theEventSystem->FireEvent("KeyReleased", args);
			return 0;
		}
		case WM_MBUTTONUP:
		{
			EventArgs args;
			args.SetValue("KeyCode", KEYCODE_MMB);
			g_theEventSystem->FireEvent("KeyReleased", args);
			return 0;
		}
		case WM_KEYUP:
		{
			EventArgs args;
			args.SetValue("KeyCode", (unsigned char)wParam);
			g_theEventSystem->FireEvent("KeyReleased", args);
#ifdef ENGINE_ENABLE_IMGUI
			if (ImGui_ImplWin32_WndProcHandler(windowHandle, wmMessageCode, wParam, lParam) == 0 && ImGui::GetIO().WantCaptureMouse)
				return 0;
#endif
			return 0;
		}
		case WM_CHAR:
		{
			EventArgs args;
			args.SetValue("CharInput", (unsigned char)wParam);
			g_theEventSystem->FireEvent("CharacterPressed", args);
#ifdef ENGINE_ENABLE_IMGUI
			if (ImGui_ImplWin32_WndProcHandler(windowHandle, wmMessageCode, wParam, lParam) == 0 && ImGui::GetIO().WantCaptureMouse)
				return 0;
#endif
			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			EventArgs args;
			args.SetValue("WheelScroll", (int)wParam);
			g_theEventSystem->FireEvent("WheelScrolled", args);
			return 0;
		}
		case WM_SYSKEYDOWN:
		{
			EventArgs args;
			args.SetValue("KeyCode", (unsigned char)wParam);
			g_theEventSystem->FireEvent("KeyPressed", args);
			return 0;
		}
		case WM_SYSKEYUP:
		{
			EventArgs args;
			args.SetValue("KeyCode", (unsigned char)wParam);
			g_theEventSystem->FireEvent("KeyReleased", args);
			return 0;
		}
	}
	// Send back to Windows any unhandled/unconsumed messages we want other apps to see (e.g. play/pause in music apps, etc.)
	return DefWindowProc(windowHandle, wmMessageCode, wParam, lParam);
}

Window::Window(WindowConfig const& config): m_config(config)
{
	s_mainWindow = this;
}

Window::~Window()
{
}

void Window::Startup()
{
	CreateOSWindow();
}

void Window::BeginFrame()
{
	RunMessagePump();
}

void Window::EndFrame()
{
}

void Window::Shutdown()
{
}

WindowConfig const& Window::GetConfig() const
{
	return m_config;
}

Window* Window::GetWindowContext()
{
	return s_mainWindow;
}

void* Window::GetHwnd() const
{
	return m_hwnd;
}

IntVec2 Window::GetClientDimensions() const
{
	return IntVec2((int)m_clientWidth, (int)m_clientHeight);
}

void* Window::GetForegroundWindow() const
{
	return ::GetForegroundWindow();
}

bool Window::GetXMLFileName(std::string& out_fileName, unsigned char keycodePressed) const
{
	std::string currentDirectoryBeforeOpening = GetCurrentDirectoryName();

	OPENFILENAMEA openFileNameA = {};
	openFileNameA.lStructSize = sizeof(OPENFILENAME);
	openFileNameA.hwndOwner = static_cast<HWND>(m_hwnd);

	openFileNameA.lpstrFilter = "XML Files (*.xml)\0*.xml\0\0";

	char fileNameBuffer[512] = {};
	openFileNameA.lpstrFile = fileNameBuffer;
	openFileNameA.nMaxFile = 500;

	std::string initialDir = (currentDirectoryBeforeOpening + std::string("\\Data\\Models"));
	openFileNameA.lpstrInitialDir = initialDir.c_str();
	openFileNameA.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	bool wasGettingFileNameSuccessful = GetOpenFileNameA(&openFileNameA);

	//std::string directoryAfterOpening = GetCurrentDirectoryName();

	SetCurrentDirectoryA(currentDirectoryBeforeOpening.c_str());

	if (keycodePressed != (unsigned char)-1) {
		EventArgs args;
		args.SetValue("KeyCode", keycodePressed);
		g_theEventSystem->FireEvent("KeyReleased", args);
	}
	//GetCurrentDirectoryA(sizeof(currentDirectoryBuffer), currentDirectoryBuffer);

	DWORD error = GetLastError();
	char errorMessageBuffer[1024] = {};
	FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, 0, errorMessageBuffer, sizeof(errorMessageBuffer), NULL);
	if (error) {
		DebuggerPrintf((std::string("Error while opening xml file: ") + std::string(errorMessageBuffer)).c_str());
	}

	LPSTR fileData = openFileNameA.lpstrFile;
	out_fileName = std::string(fileData);
	return wasGettingFileNameSuccessful;
}

bool Window::GetFBXFileName(std::string& out_fileName, unsigned char keycodePressed) const
{
	std::string currentDirectoryBeforeOpening = GetCurrentDirectoryName();

	OPENFILENAMEA openFileNameA = {};
	openFileNameA.lStructSize = sizeof(OPENFILENAME);
	openFileNameA.hwndOwner = static_cast<HWND>(m_hwnd);

	openFileNameA.lpstrFilter = "FBX Files (*.fbx)\0*.fbx\0\0";

	char fileNameBuffer[512] = {};
	openFileNameA.lpstrFile = fileNameBuffer;
	openFileNameA.nMaxFile = 500;

	std::string initialDir = (currentDirectoryBeforeOpening + std::string("\\Data\\Models"));
	openFileNameA.lpstrInitialDir = initialDir.c_str();
	openFileNameA.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	bool wasGettingFileNameSuccessful = GetOpenFileNameA(&openFileNameA);

	//std::string directoryAfterOpening = GetCurrentDirectoryName();

	SetCurrentDirectoryA(currentDirectoryBeforeOpening.c_str());

	if (keycodePressed != (unsigned char)-1) {
		EventArgs args;
		args.SetValue("KeyCode", keycodePressed);
		g_theEventSystem->FireEvent("KeyReleased", args);
	}
	//GetCurrentDirectoryA(sizeof(currentDirectoryBuffer), currentDirectoryBuffer);

	DWORD error = GetLastError();
	char errorMessageBuffer[1024] = {};
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, 0, errorMessageBuffer, sizeof(errorMessageBuffer), NULL);
	if (error) {
		DebuggerPrintf((std::string("Error while opening fbx file: ") + std::string(errorMessageBuffer)).c_str());
	}

	LPSTR fileData = openFileNameA.lpstrFile;
	out_fileName = std::string(fileData);
	return wasGettingFileNameSuccessful;
}

bool Window::GetDirectoryPath(std::string& out_directoryPath, unsigned char keycodePressed) const
{
	IFileDialog* pfd;
	if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd))))
	{
		DWORD dwOptions;
		if (SUCCEEDED(pfd->GetOptions(&dwOptions)))
		{
			pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
		}
		if (SUCCEEDED(pfd->Show(NULL)))
		{
			if (keycodePressed != (unsigned char)-1) {
				EventArgs args;
				args.SetValue("KeyCode", keycodePressed);
				g_theEventSystem->FireEvent("KeyReleased", args);
			}
			IShellItem* psi;
			if (SUCCEEDED(pfd->GetResult(&psi)))
			{
				PWSTR wpath;
				if (SUCCEEDED(psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &wpath)))
				{
					// Convert the wide character path to a regular string
					int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, NULL, 0, NULL, NULL);
					char* path = new char[bufferSize];
					WideCharToMultiByte(CP_UTF8, 0, wpath, -1, path, bufferSize, NULL, NULL);
					out_directoryPath = path;

					CoTaskMemFree(wpath);
					delete[] path;
					psi->Release();
					pfd->Release();
					return true;
				}
				psi->Release();
			}
		}
		pfd->Release();
	}
	return false;
}

bool Window::GetGHCSFileName(std::string& out_fileName, unsigned char keycodePressed) const
{
	std::string currentDirectoryBeforeOpening = GetCurrentDirectoryName();

	OPENFILENAMEA openFileNameA = {};
	openFileNameA.lStructSize = sizeof(OPENFILENAME);
	openFileNameA.hwndOwner = static_cast<HWND>(m_hwnd);

	openFileNameA.lpstrFilter = "GHCS Files (*.ghcs)\0*.ghcs\0\0";

	char fileNameBuffer[512] = {};
	openFileNameA.lpstrFile = fileNameBuffer;
	openFileNameA.nMaxFile = 500;

	std::string initialDir = (currentDirectoryBeforeOpening + std::string("\\Data\\ConvexScenes"));
	openFileNameA.lpstrInitialDir = initialDir.c_str();
	openFileNameA.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	bool wasGettingFileNameSuccessful = GetOpenFileNameA(&openFileNameA);

	//std::string directoryAfterOpening = GetCurrentDirectoryName();

	SetCurrentDirectoryA(currentDirectoryBeforeOpening.c_str());

	if (keycodePressed != (unsigned char)-1) {
		EventArgs args;
		args.SetValue("KeyCode", keycodePressed);
		g_theEventSystem->FireEvent("KeyReleased", args);
	}
	//GetCurrentDirectoryA(sizeof(currentDirectoryBuffer), currentDirectoryBuffer);

	DWORD error = GetLastError();
	char errorMessageBuffer[1024] = {};
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, 0, errorMessageBuffer, sizeof(errorMessageBuffer), NULL);
	if (error) {
		DebuggerPrintf((std::string("Error while opening xml file: ") + std::string(errorMessageBuffer)).c_str());
	}

	LPSTR fileData = openFileNameA.lpstrFile;
	out_fileName = std::string(fileData);
	return wasGettingFileNameSuccessful;
}

bool Window::GetPNGFileName(std::string& out_fileName, unsigned char keycodePressed) const
{
	std::string currentDirectoryBeforeOpening = GetCurrentDirectoryName();

	OPENFILENAMEA openFileNameA = {};
	openFileNameA.lStructSize = sizeof(OPENFILENAME);
	openFileNameA.hwndOwner = static_cast<HWND>(m_hwnd);

	openFileNameA.lpstrFilter = "PNG Files (*.png)\0*.png\0\0";

	char fileNameBuffer[512] = {};
	openFileNameA.lpstrFile = fileNameBuffer;
	openFileNameA.nMaxFile = 500;

	std::string initialDir = (currentDirectoryBeforeOpening + std::string("\\Data\\Sprites"));
	openFileNameA.lpstrInitialDir = initialDir.c_str();
	openFileNameA.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	bool wasGettingFileNameSuccessful = GetOpenFileNameA(&openFileNameA);

	//std::string directoryAfterOpening = GetCurrentDirectoryName();

	SetCurrentDirectoryA(currentDirectoryBeforeOpening.c_str());

	if (keycodePressed != (unsigned char)-1) {
		EventArgs args;
		args.SetValue("KeyCode", keycodePressed);
		g_theEventSystem->FireEvent("KeyReleased", args);
	}
	//GetCurrentDirectoryA(sizeof(currentDirectoryBuffer), currentDirectoryBuffer);

	DWORD error = GetLastError();
	char errorMessageBuffer[1024] = {};
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, 0, errorMessageBuffer, sizeof(errorMessageBuffer), NULL);
	if (error) {
		DebuggerPrintf((std::string("Error while opening png file: ") + std::string(errorMessageBuffer)).c_str());
	}

	LPSTR fileData = openFileNameA.lpstrFile;
	out_fileName = std::string(fileData);
	return wasGettingFileNameSuccessful;
}

bool Window::GetBVHFileName(std::string& out_fileName, unsigned char keycodePressed) const
{
	std::string currentDirectoryBeforeOpening = GetCurrentDirectoryName();

	OPENFILENAMEA openFileNameA = {};
	openFileNameA.lStructSize = sizeof(OPENFILENAME);
	openFileNameA.hwndOwner = static_cast<HWND>(m_hwnd);

	openFileNameA.lpstrFilter = "BVH Files (*.bvh)\0*.bvh\0\0";

	char fileNameBuffer[512] = {};
	openFileNameA.lpstrFile = fileNameBuffer;
	openFileNameA.nMaxFile = 500;

	std::string initialDir = (currentDirectoryBeforeOpening + std::string("\\Data\\BVHFiles"));
	openFileNameA.lpstrInitialDir = initialDir.c_str();
	openFileNameA.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	bool wasGettingFileNameSuccessful = GetOpenFileNameA(&openFileNameA);

	//std::string directoryAfterOpening = GetCurrentDirectoryName();

	SetCurrentDirectoryA(currentDirectoryBeforeOpening.c_str());

	if (keycodePressed != (unsigned char)-1) {
		EventArgs args;
		args.SetValue("KeyCode", keycodePressed);
		g_theEventSystem->FireEvent("KeyReleased", args);
	}
	//GetCurrentDirectoryA(sizeof(currentDirectoryBuffer), currentDirectoryBuffer);

	DWORD error = GetLastError();
	char errorMessageBuffer[1024] = {};
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, 0, errorMessageBuffer, sizeof(errorMessageBuffer), NULL);
	if (error) {
		DebuggerPrintf((std::string("Error while opening bvh file: ") + std::string(errorMessageBuffer)).c_str());
	}

	LPSTR fileData = openFileNameA.lpstrFile;
	out_fileName = std::string(fileData);
	return wasGettingFileNameSuccessful;
}

std::string Window::GetCurrentDirectoryName() const
{
	char currentDirectoryBuffer[512] = {};
	GetCurrentDirectoryA(sizeof(currentDirectoryBuffer), currentDirectoryBuffer);
	return std::string(currentDirectoryBuffer);
}

bool Window::IsFullScreen() const
{
	return m_config.m_isFullscreen;
}

std::string Window::PasteTextFromClipboard()
{
	if (!OpenClipboard(nullptr)) {
		return "";
	}

	HANDLE hData = GetClipboardData(CF_TEXT);
	if (hData == nullptr) {
		CloseClipboard();
		return "";
	}

	char* pszText = static_cast<char*>(GlobalLock(hData));
	if (pszText == nullptr) {
		CloseClipboard();
		return "";
	}

	std::string text(pszText);

	GlobalUnlock(hData);
	CloseClipboard();

	return text;
}

void Window::CreateOSWindow()
{
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset(&windowClassDescription, 0, sizeof(windowClassDescription));
	windowClassDescription.cbSize = sizeof(windowClassDescription);
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast<WNDPROC>(WindowsMessageHandlingProcedure); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle(NULL);
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT("Simple Window Class");
	RegisterClassEx(&windowClassDescription);

	RECT clientRect;
	DWORD windowStyleFlags = 0;
	if (m_config.m_isFullscreen) {
		//fill out here
		windowStyleFlags = WS_POPUP;
		// Get desktop rect, dimensions, aspect
		RECT desktopRect;
		HWND desktopWindowHandle = GetDesktopWindow();
		GetClientRect(desktopWindowHandle, &desktopRect);
		float desktopWidth = (float)(desktopRect.right - desktopRect.left);
		float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);

		m_clientWidth = desktopWidth;
		m_clientHeight = desktopHeight;
		clientRect.left = 0;
		clientRect.top = 0;
		clientRect.right = (int)m_clientWidth;
		clientRect.bottom = (int)m_clientHeight;
	}
	else {
		windowStyleFlags = WS_CAPTION | WS_BORDER | WS_SYSMENU | WS_OVERLAPPED;
		if (m_config.m_showThiccFrame) {
			windowStyleFlags |= WS_THICKFRAME;
		}
		// Get desktop rect, dimensions, aspect
		RECT desktopRect;
		HWND desktopWindowHandle = GetDesktopWindow();
		GetClientRect(desktopWindowHandle, &desktopRect);
		float desktopWidth = (float)(desktopRect.right - desktopRect.left);
		float desktopHeight = (float)(desktopRect.bottom - desktopRect.top);
		float desktopAspect = desktopWidth / desktopHeight;

		if (m_config.m_windowSize == IntVec2()) {
			// Calculate maximum client size (as some % of desktop size)
			constexpr float maxClientFractionOfDesktop = 0.90f;
			m_clientWidth = desktopWidth * maxClientFractionOfDesktop;
			m_clientHeight = desktopHeight * maxClientFractionOfDesktop;
			if (m_config.m_clientAspect > desktopAspect)
			{
				// Client window has a wider aspect than desktop; shrink client height to match its width
				m_clientHeight = m_clientWidth / m_config.m_clientAspect;
			}
			else
			{
				// Client window has a taller aspect than desktop; shrink client width to match its height
				m_clientWidth = m_clientHeight * m_config.m_clientAspect;
			}

			// Calculate client rect bounds by centering the client area
			float clientMarginX = 0.5f * (desktopWidth - m_clientWidth);
			float clientMarginY = 0.5f * (desktopHeight - m_clientHeight);
			clientRect.left = (int)clientMarginX;
			clientRect.right = clientRect.left + (int)m_clientWidth;
			clientRect.top = (int)clientMarginY;
			clientRect.bottom = clientRect.top + (int)m_clientHeight;
		}
		else {
			m_clientWidth = (float)m_config.m_windowSize.x;
			m_clientHeight = (float)m_config.m_windowSize.y;

			clientRect.left = m_config.m_windowPosition.x;
			clientRect.right = clientRect.left + (int)m_clientWidth;
			clientRect.top = m_config.m_windowPosition.y;
			clientRect.bottom = clientRect.top + (int)m_clientHeight;
		}
	}
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx(&windowRect, windowStyleFlags, FALSE, windowStyleExFlags);

	WCHAR windowTitle[1024];
	MultiByteToWideChar(GetACP(), 0, m_config.m_windowTitle.c_str(), -1, windowTitle, sizeof(windowTitle) / sizeof(windowTitle[0]));
	HWND windowHandle = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		(HINSTANCE)NULL,
		NULL);
	

	ShowWindow(windowHandle, SW_SHOW);
	SetForegroundWindow(windowHandle);
	SetFocus(windowHandle);

	m_hwnd = windowHandle;

	//g_displayDeviceContext = GetDC(m_windowHandle);

	HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
	SetCursor(cursor);
}

void Window::RunMessagePump()
{
	MSG queuedMessage;
	for (;; )
	{
		const BOOL wasMessagePresent = PeekMessage(&queuedMessage, NULL, 0, 0, PM_REMOVE);
		if (!wasMessagePresent)
		{
			break;
		}

		TranslateMessage(&queuedMessage);
		DispatchMessage(&queuedMessage); // This tells Windows to call our "WindowsMessageHandlingProcedure" (a.k.a. "WinProc") function
	}
}