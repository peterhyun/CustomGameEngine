#pragma once
#include <string>

struct ShaderConfig
{
	std::string m_name;
	std::string m_vertexEntryPoint = "VertexMain";
	std::string m_pixelEntryPoint = "PixelMain";
};

class Shader
{
	friend class Renderer;

public:
	Shader(const Shader& copy) = delete;
	~Shader();
	const std::string& GetName() const;

private:
	Shader(const ShaderConfig& config);

private:
	ShaderConfig m_config;
	struct ID3D11VertexShader* m_vertexShader = nullptr;
	struct ID3D11PixelShader* m_pixelShader = nullptr;
	struct ID3D11InputLayout* m_inputLayout = nullptr;
};