#pragma once
class IndexBuffer {
	friend class Renderer;

public:
	virtual ~IndexBuffer();

private:
	IndexBuffer(size_t size);
	IndexBuffer(const IndexBuffer& copy) = delete;

	//unsigned int GetStride() const;

	struct ID3D11Buffer* m_buffer = nullptr;
	size_t m_size = 0;
};