#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/Renderer.hpp"

//Including directx 11 header files and such
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

Shader::Shader(const ShaderConfig& config) : m_config(config)
{
}

Shader::~Shader()
{
	DX_SAFE_RELEASE(m_inputLayout);
	DX_SAFE_RELEASE(m_pixelShader);
	DX_SAFE_RELEASE(m_vertexShader);
}

const std::string& Shader::GetName() const
{
	return m_config.m_name;
}
