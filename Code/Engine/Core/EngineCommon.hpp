#pragma once
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Multithread/JobSystem.hpp"
#include "Engine/Net/NetSystem.hpp"
#include "Engine/UI/ImGuiLayer.hpp"

#define UNUSED(x) (void)(x);

extern NamedProperties g_gameConfigBlackboard;
extern EventSystem* g_theEventSystem;
extern DevConsole* g_theDevConsole;
extern InputSystem* g_theInput;
extern JobSystem* g_theJobSystem;
extern NetSystem* g_theNetSystem;
#if defined ENGINE_ENABLE_IMGUI
extern ImGuiLayer* g_theImGuiLayer;
#endif