#pragma once
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include <vector>

class CPUMesh {
public:
	CPUMesh(const std::vector<Vertex_PCUTBN>& vertices, const std::vector<unsigned int> indices) : m_vertices(vertices), m_indices(indices) 
	{
	};

private:
	std::vector<Vertex_PCUTBN> m_vertices;
	std::vector<unsigned int> m_indices;
};