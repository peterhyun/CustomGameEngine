#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <vector>

struct GPUMeshConfig {
	GPUMeshConfig(Renderer& renderer) : m_renderer(renderer) {};
	Renderer& m_renderer;
};

template<typename VertexType>
class GPUMesh {
public:
	GPUMesh(const GPUMeshConfig& config, const std::vector<VertexType>& vertices, const std::vector<unsigned int>& indices, bool isTriangleList = true);
	GPUMesh(const GPUMeshConfig& config, const std::vector<VertexType>& vertices, bool isTriangleList = true);
	~GPUMesh();
	void UpdateVerticesData(const std::vector<VertexType>& vertices);
	void Render() const;

private:
	GPUMeshConfig m_config;
	VertexBuffer* m_vbo = nullptr;
	IndexBuffer* m_ibo = nullptr;

	/*
	//For debugging
	VertexBuffer* m_normalsVBO = nullptr;
	std::vector<Vertex_PCU> m_normalDebugArrowVerts;
	*/

	std::vector<VertexType> m_vertices;
	std::vector<unsigned int> m_indices;

};

template<typename VertexType>
GPUMesh<VertexType>::GPUMesh(const GPUMeshConfig& config, const std::vector<VertexType>& vertices, const std::vector<unsigned int>& indices, bool isTriangleList) : m_config(config), m_vertices(vertices), m_indices(indices)
{
	m_vbo = config.m_renderer.CreateVertexBuffer(vertices.size() * sizeof(VertexType), sizeof(VertexType), Stringf("GPUMesh::m_vbo of %d vertices", (int)vertices.size()), isTriangleList);
	m_ibo = config.m_renderer.CreateIndexBuffer(indices.size() * sizeof(unsigned int));

	config.m_renderer.CopyCPUToGPU(vertices.data(), vertices.size() * sizeof(VertexType), sizeof(VertexType), m_vbo);
	config.m_renderer.CopyCPUToGPU(indices.data(), indices.size() * sizeof(unsigned int), m_ibo);
}

template<typename VertexType>
inline GPUMesh<VertexType>::GPUMesh(const GPUMeshConfig& config, const std::vector<VertexType>& vertices, bool isTriangleList) : m_config(config), m_vertices(vertices)
{
	m_vbo = config.m_renderer.CreateVertexBuffer(vertices.size() * sizeof(VertexType), sizeof(VertexType), Stringf("GPUMesh::m_vbo of %d vertices", (int)vertices.size()), isTriangleList);
	config.m_renderer.CopyCPUToGPU(vertices.data(), vertices.size() * sizeof(VertexType), sizeof(VertexType), m_vbo);
}

template<typename VertexType>
GPUMesh<VertexType>::~GPUMesh()
{
	//delete m_normalsVBO;

	delete m_vbo;
	delete m_ibo;
}

template<typename VertexType>
inline void GPUMesh<VertexType>::UpdateVerticesData(const std::vector<VertexType>& vertices)
{
	m_config.m_renderer.CopyCPUToGPU(vertices.data(), vertices.size() * sizeof(VertexType), sizeof(VertexType), m_vbo);
}

template<typename VertexType>
void GPUMesh<VertexType>::Render() const
{
	if (m_ibo) {
		m_config.m_renderer.DrawVertexAndIndexBuffer(m_vbo, m_ibo, (int)m_indices.size());
	}
	else {
		m_config.m_renderer.DrawVertexBuffer(m_vbo, (int)m_vertices.size());
	}
	/*
	m_config.m_renderer.BindShader(nullptr);
	m_config.m_renderer.DrawVertexBuffer(m_normalsVBO, (int)m_normalDebugArrowVerts.size());
	*/
}