#pragma once

class ComputeOutputBuffer {
	friend class Renderer;

public:
	~ComputeOutputBuffer();

private:
	ComputeOutputBuffer(size_t outputBufferByteSize);

private:
	size_t m_outputBufferByteSize = 0;

	struct ID3D11Buffer* m_outputStructuredBuffer = nullptr;
	struct ID3D11UnorderedAccessView* m_outputUAV = nullptr;
	struct ID3D11ShaderResourceView* m_outputSRV = nullptr;

	//struct ID3D11Buffer* m_stagingBuffer = nullptr;
};