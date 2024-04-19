#include "Engine/UI/UIManager.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void UIManager::Startup()
{
	GUARANTEE_OR_DIE(m_config.m_overlays.size() > 0, "No overlays in UI Manager!");
	m_currentActiveOverlay = m_config.m_overlays[0];
}

void UIManager::Shutdown()
{
}

void UIManager::BeginFrame()
{
}

void UIManager::EndFrame()
{
}
