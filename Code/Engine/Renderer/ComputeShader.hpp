#pragma once
#include <string>

struct ComputeShaderConfig {
	std::string m_name;
};

class ComputeShader {
	friend class Renderer;
public:
	ComputeShader(const ComputeShader& copy) = delete;
	~ComputeShader();
	std::string GetName() const;

private:
	ComputeShader(const ComputeShaderConfig& config);

private:
	ComputeShaderConfig m_config;
	struct ID3D11ComputeShader* m_computeShader = nullptr;
};