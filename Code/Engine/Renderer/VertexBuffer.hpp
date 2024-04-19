#pragma once
class VertexBuffer {
	friend class Renderer;

public:
	virtual ~VertexBuffer();
	unsigned int GetStride() const;

private:
	VertexBuffer(size_t size, unsigned int stride, bool isTriangleList = true);
	VertexBuffer(const VertexBuffer& copy) = delete;

private:
	struct ID3D11Buffer* m_buffer = nullptr;
	size_t m_size = 0;
	unsigned int m_stride = 0;
	bool m_isTriangleList = true;
};