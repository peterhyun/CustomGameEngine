#include "Engine/Renderer/ComputeOutputBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

//Including directx 11 header files and such
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

ComputeOutputBuffer::~ComputeOutputBuffer()
{
	DX_SAFE_RELEASE(m_outputStructuredBuffer);
	DX_SAFE_RELEASE(m_outputUAV);
	DX_SAFE_RELEASE(m_outputSRV);
	//DX_SAFE_RELEASE(m_stagingBuffer);
}

ComputeOutputBuffer::ComputeOutputBuffer(size_t outputBufferByteSize) : m_outputBufferByteSize(outputBufferByteSize) {}