#pragma once
#include "Engine/UI/Overlay.hpp"
#include <vector>

struct UIManagerConfig {
	std::vector<Overlay*> m_overlays;
};

class UIManager {
public:
	UIManager(UIManagerConfig const& config): m_config(config) {};
	~UIManager() {};

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

private:
	UIManagerConfig m_config;
	Overlay* m_currentActiveOverlay = nullptr;
};