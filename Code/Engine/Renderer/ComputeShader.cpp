#include "Engine/Renderer/ComputeShader.hpp"
#include "Engine/Renderer/Renderer.hpp"
//Including directx 11 header files and such
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

ComputeShader::~ComputeShader()
{
    DX_SAFE_RELEASE(m_computeShader);
}

std::string ComputeShader::GetName() const
{
    return m_config.m_name;
}

ComputeShader::ComputeShader(const ComputeShaderConfig& config) : m_config(config)
{
}
