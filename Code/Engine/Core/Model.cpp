#include "Engine/Core/Model.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Core/CPUMesh.hpp"
#include "Engine/Core/GPUMesh.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Material.hpp"

Model::Model(const ModelConfig& config) : m_config(config)
{
	//m_debugShader = config.m_renderer.CreateOrGetShader("Data/Shaders/DefaultNoTexture");
}

Model::~Model()
{
	delete m_gpuDebugMesh;
	m_gpuDebugMesh = nullptr;
	delete m_gpuMesh;
	m_gpuMesh = nullptr;
	delete m_cpuMesh;
	m_cpuMesh = nullptr;
}

bool Model::ParseXMLFile(const std::string& xmlFilePath)
{
	XmlDocument xmldoc;
	if (xmldoc.LoadFile(xmlFilePath.c_str()) == XmlResult::XML_SUCCESS) {
		XmlElement* rootElementPtr = xmldoc.FirstChildElement("Model");
		//Loop through ActorDefinition
		if (rootElementPtr) {
			std::string modelName = ParseXmlAttribute(*rootElementPtr, "name", "Invalid");
			std::string modelPath = ParseXmlAttribute(*rootElementPtr, "path", "Invalid");
			std::string shaderPath = ParseXmlAttribute(*rootElementPtr, "shader", "Invalid");
			std::string materialPath = ParseXmlAttribute(*rootElementPtr, "material", "Invalid");

			XmlElement* transformElementPtr = rootElementPtr->FirstChildElement("Transform");
			Vec3 i(1.0f, 0.0f, 0.0f);
			Vec3 j(0.0f, 1.0f, 0.0f);
			Vec3 k(0.0f, 0.0f, 1.0f);
			Vec3 t(0.0f, 0.0f, 0.0f);
			float scale = 1.0f;
			if (transformElementPtr) {
				i = ParseXmlAttribute(*transformElementPtr, "x", Vec3(1.0f, 0.0f, 0.0f));
				j = ParseXmlAttribute(*transformElementPtr, "y", Vec3(0.0f, 1.0f, 0.0f));
				k = ParseXmlAttribute(*transformElementPtr, "z", Vec3(0.0f, 0.0f, 1.0f));
				t = ParseXmlAttribute(*transformElementPtr, "t", Vec3(0.0f, 0.0f, 0.0f));
				scale = ParseXmlAttribute(*transformElementPtr, "scale", 1.0f);
			}

			std::vector<Vertex_PCUTBN> vertices;
			std::vector<unsigned int> indices;
			OBJLoaderMetaData objMetaData;
			OBJLoader::ParseFile(modelPath, Mat44(Vec4(i * scale, 0.0f), Vec4(j * scale, 0.0f), Vec4(k * scale, 0.0f), Vec4(t, 1.0f)), vertices, indices, &objMetaData);
			CalculateTangents(vertices, indices);

			DebuggerPrintf("------------------------------\n");
			DebuggerPrintf(Stringf("Loaded .obj file %s\n", modelPath.c_str()).c_str());
			DebuggerPrintf(Stringf("[file data] positions: %d, uvs: %d, normals: %d, faces: %d, triangles: %d\n", objMetaData.m_numPositions, objMetaData.m_numUVs, objMetaData.m_numNormals, objMetaData.m_numFaces, objMetaData.m_numTriangles).c_str());
			DebuggerPrintf(Stringf("[loaded mesh] vertices: %d, indices: %d\n", objMetaData.m_numVertices, objMetaData.m_numIndices).c_str());
			DebuggerPrintf(Stringf("[total parse and load seconds]: %f\n", objMetaData.m_totalParseAndLoadTime).c_str());
			DebuggerPrintf("------------------------------");

			if (m_cpuMesh) {
				delete m_cpuMesh;
				m_cpuMesh = nullptr;
			}
			if (m_gpuMesh) {
				delete m_gpuMesh;
				m_gpuMesh = nullptr;
			}
			m_cpuMesh = new CPUMesh(vertices, indices);
			m_gpuMesh = new GPUMesh<Vertex_PCUTBN>(GPUMeshConfig(m_config.m_renderer), vertices, indices);
			if (shaderPath != "Invalid") {
				m_shader = m_config.m_renderer.CreateOrGetShader(shaderPath.c_str());
			}
			else if (materialPath != "Invalid") {
				m_material = Material::CreateMaterial(m_config.m_renderer, materialPath.c_str());
			}
			else {
				ERROR_AND_DIE("shader and material can't both be invalid");
			}

			//Create the debug mesh
			std::vector<Vertex_PCU> debugVerts;
			float lineLength = 0.3f;
			for (int vertexIdx = 0; vertexIdx < vertices.size(); vertexIdx++) {
				debugVerts.emplace_back(vertices[vertexIdx].m_position, Rgba8::RED);
				debugVerts.emplace_back(vertices[vertexIdx].m_position + vertices[vertexIdx].m_tangent * lineLength, Rgba8::RED);
				debugVerts.emplace_back(vertices[vertexIdx].m_position, Rgba8::GREEN);
				debugVerts.emplace_back(vertices[vertexIdx].m_position + vertices[vertexIdx].m_binormal * lineLength, Rgba8::GREEN);
				debugVerts.emplace_back(vertices[vertexIdx].m_position, Rgba8::BLUE);
				debugVerts.emplace_back(vertices[vertexIdx].m_position + vertices[vertexIdx].m_normal * lineLength, Rgba8::BLUE);
			}
			m_gpuDebugMesh = new GPUMesh<Vertex_PCU>(GPUMeshConfig(m_config.m_renderer), debugVerts, false);

			return true;
		}
	}
	return false;
}

void Model::InitializeManually(const std::vector<Vertex_PCUTBN>& vertices, const std::vector<unsigned int>& indices, Shader* shader, Material* material)
{
	GUARANTEE_OR_DIE(m_cpuMesh == nullptr, "This model is already intialized");
	GUARANTEE_OR_DIE(m_gpuMesh == nullptr, "This model is already initialized");
	GUARANTEE_OR_DIE(m_gpuDebugMesh == nullptr, "This model is already initialized");
	GUARANTEE_OR_DIE(shader != nullptr || material != nullptr, "Either shader or material should not be a nullptr!");
	m_cpuMesh = new CPUMesh(vertices, indices);
	m_gpuMesh = new GPUMesh<Vertex_PCUTBN>(GPUMeshConfig(m_config.m_renderer), vertices, indices);
	//Create the debug mesh
	std::vector<Vertex_PCU> debugVerts;
	float lineLength = 0.3f;
	for (int vertexIdx = 0; vertexIdx < vertices.size(); vertexIdx++) {
		debugVerts.emplace_back(vertices[vertexIdx].m_position, Rgba8::RED);
		debugVerts.emplace_back(vertices[vertexIdx].m_position + vertices[vertexIdx].m_tangent * lineLength, Rgba8::RED);
		debugVerts.emplace_back(vertices[vertexIdx].m_position, Rgba8::GREEN);
		debugVerts.emplace_back(vertices[vertexIdx].m_position + vertices[vertexIdx].m_binormal * lineLength, Rgba8::GREEN);
		debugVerts.emplace_back(vertices[vertexIdx].m_position, Rgba8::BLUE);
		debugVerts.emplace_back(vertices[vertexIdx].m_position + vertices[vertexIdx].m_normal * lineLength, Rgba8::BLUE);
	}
	m_gpuDebugMesh = new GPUMesh<Vertex_PCU>(GPUMeshConfig(m_config.m_renderer), debugVerts, false);
	m_shader = shader;
	m_material = material;
}

void Model::Render(const Mat44& modelMat, bool renderDebugMesh) const
{
	m_config.m_renderer.SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	m_config.m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_DISABLED);
	m_config.m_renderer.SetBlendMode(BlendMode::OPAQUE);
	m_config.m_renderer.SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	if (m_material == nullptr) {
		m_config.m_renderer.BindShader(m_shader);
	}
	else {
		m_config.m_renderer.BindShader(m_material->GetShader());
		m_config.m_renderer.BindTextures(m_material->GetDiffuseTexture(), m_material->GetNormalTexture(), m_material->GetSpecGlossEmitTexture());
		m_config.m_renderer.SetModelConstants(modelMat, m_material->GetColor());
	}
	if (m_gpuMesh)
		m_gpuMesh->Render();

	if (m_gpuDebugMesh && renderDebugMesh) {
		//m_config.m_renderer.BindShader(m_debugShader);
		m_config.m_renderer.BindShader(nullptr);
		m_config.m_renderer.BindTexture(nullptr);
		m_gpuDebugMesh->Render();
	}
}
