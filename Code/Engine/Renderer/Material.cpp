#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

Material* Material::CreateMaterial(Renderer& rendererToUse, const std::string& materialXmlPath)
{
	XmlDocument xmldoc;
	if (xmldoc.LoadFile(materialXmlPath.c_str()) == XmlResult::XML_SUCCESS) {
		XmlElement* rootElementPtr = xmldoc.FirstChildElement("Material");
		if (rootElementPtr) {
			std::string materialName = ParseXmlAttribute(*rootElementPtr, "name", "Invalid");
			std::string shaderPath = ParseXmlAttribute(*rootElementPtr, "shader", "Invalid");
			std::string diffuseTexturePath = ParseXmlAttribute(*rootElementPtr, "diffuseTexture", "Invalid");
			std::string normalTexturePath = ParseXmlAttribute(*rootElementPtr, "normalTexture", "Invalid");
			std::string specGlossEmitTexturePath = ParseXmlAttribute(*rootElementPtr, "specGlossEmitTexture", "Invalid");
			Rgba8 color = ParseXmlAttribute(*rootElementPtr, "color", Rgba8::WHITE);

			Material* newMaterial = new Material(
				materialName,
				*rendererToUse.CreateOrGetShader(shaderPath.c_str()),
				rendererToUse.CreateOrGetTextureFromFile(diffuseTexturePath.c_str()),
				rendererToUse.CreateOrGetTextureFromFile(normalTexturePath.c_str()),
				rendererToUse.CreateOrGetTextureFromFile(specGlossEmitTexturePath.c_str()),
				color
			);
			return newMaterial;
		}
		else {
			ERROR_AND_DIE("File doesn't have tag called 'Material'");
		}
	}
	else {
		ERROR_AND_DIE(Stringf("Could not parse path %s", materialXmlPath.c_str()));
	}
}

Material::Material(const std::string& name, Shader& shader, Texture* diffuse, Texture* normal, Texture* specGlossEmit, const Rgba8& color) :
	m_name(name), m_shader(&shader), m_diffuseTexture(diffuse), m_normalTexture(normal), m_specGlossEmitTexture(specGlossEmit), m_color(color)
{
}

Shader* Material::GetShader() const
{
	return m_shader;
}

Texture* Material::GetDiffuseTexture() const
{
	return m_diffuseTexture;
}

Texture* Material::GetNormalTexture() const
{
	return m_normalTexture;
}

Texture* Material::GetSpecGlossEmitTexture() const
{
	return m_specGlossEmitTexture;
}

Rgba8 Material::GetColor() const
{
	return m_color;
}
