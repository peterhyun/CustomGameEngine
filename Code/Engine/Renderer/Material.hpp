#pragma once
#include "Engine/Core/Rgba8.hpp"
#include <string>

class Shader;
class Texture;
class Renderer;

class Material {
public:
	static Material* CreateMaterial(Renderer& rendererToUse, const std::string& materialXmlPath);
	Shader* GetShader() const;
	Texture* GetDiffuseTexture() const;
	Texture* GetNormalTexture() const;
	Texture* GetSpecGlossEmitTexture() const;
	Rgba8 GetColor() const;

private:
	Material(const std::string& name, Shader& shader, Texture* diffuse, Texture* normal, Texture* specGlossEmit, const Rgba8& color);
	
private:
	std::string m_name;
	Shader* m_shader = nullptr;
	Texture* m_diffuseTexture = nullptr;
	Texture* m_normalTexture = nullptr;
	Texture* m_specGlossEmitTexture = nullptr;
	Rgba8 m_color = Rgba8::WHITE;
};