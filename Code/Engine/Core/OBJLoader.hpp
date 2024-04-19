#pragma once
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Mat44.hpp"
#include <string>
#include <vector>
#include <sstream>

class OBJLoaderMetaData {
public:
	int m_numPositions = 0;
	int m_numUVs = 0;
	int m_numNormals = 0;
	int m_numFaces = 0;
	int m_numTriangles = 0;

	int m_numVertices = 0;
	int m_numIndices = 0;

	float m_totalParseAndLoadTime = 0.0f;
};

class OBJLoader
{
public:
	OBJLoader() {};
	~OBJLoader() {};
	static void ParseFile(const std::string& filePath, const Mat44& transform, std::vector<Vertex_PCUTBN>& out_vertices, std::vector<unsigned int>& out_indices, OBJLoaderMetaData* out_metaData = nullptr);
};