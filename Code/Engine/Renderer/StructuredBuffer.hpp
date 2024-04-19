#pragma once

class StructuredBuffer
{
	friend class Renderer; // Only the Renderer can create new StructuredBuffer objects!

public:
	~StructuredBuffer();

private:
	StructuredBuffer(size_t size) : m_size(size) {}; // can't instantiate directly; must ask Renderer to do it for you
	StructuredBuffer(StructuredBuffer const& copy) = delete; // No copying allowed!  This represents GPU memory.

protected:
	size_t m_size = 0;
	struct ID3D11Buffer* m_structuredBuffer = nullptr;
	struct ID3D11ShaderResourceView* m_shaderResourceViewForStructuredBuffer = nullptr;
};