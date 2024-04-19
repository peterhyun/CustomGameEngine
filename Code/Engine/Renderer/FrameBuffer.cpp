#include "Engine/Renderer/FrameBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

//Including directx 11 header files and such
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

FrameBuffer::~FrameBuffer()
{
	DX_SAFE_RELEASE(m_renderTargetViewForFrameBuffer);
	DX_SAFE_RELEASE(m_shaderResourceViewForFrameBuffer);
	DX_SAFE_RELEASE(m_textureForRTV);
	DX_SAFE_RELEASE(m_textureForSRV);
}
