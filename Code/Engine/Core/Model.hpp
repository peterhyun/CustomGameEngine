#pragma once
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/Mat44.hpp"
#include <vector>

class Renderer;
class CPUMesh;
template<typename Vertex_PCUTBN>
class GPUMesh;
class Material;

struct ModelConfig {
	ModelConfig(Renderer& rendererForModel) : m_renderer(rendererForModel) {};
	Renderer& m_renderer;
};

class Model {
public:
	Model(const ModelConfig& config);
	~Model();
	bool ParseXMLFile(const std::string& xmlFilePath);	/* Returns whether the parsing of the xml file was successful */
	void InitializeManually(const std::vector<Vertex_PCUTBN>& verts, const std::vector<unsigned int>& indices, Shader* shader, Material* material);
	void Render(const Mat44& modelMat = Mat44(), bool renderDebugMesh = false) const;	//Is it allowed for me to call a non-const Renderer function here?
	
private:
	ModelConfig m_config;
	CPUMesh* m_cpuMesh = nullptr;
	GPUMesh<Vertex_PCUTBN>* m_gpuMesh = nullptr;
	//For debugging tangent space vectors
	GPUMesh<Vertex_PCU>* m_gpuDebugMesh = nullptr;

	Shader* m_shader = nullptr;
	Material* m_material = nullptr;
};