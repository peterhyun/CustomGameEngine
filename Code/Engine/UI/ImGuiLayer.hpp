#pragma once

class Window;
class Renderer;

struct ImGuiLayerConfig {
public:
	ImGuiLayerConfig(Window& windowToUse, Renderer& rendererToUse): m_windowToUse(windowToUse), m_rendererToUse(rendererToUse){};
	Window& m_windowToUse;
	Renderer& m_rendererToUse;
};

class ImGuiLayer {
public:
	ImGuiLayer(const ImGuiLayerConfig& config);
	void Startup();
	void Shutdown();

	void BeginFrame();
	void EndFrame();

private:
	ImGuiLayerConfig m_config;
};