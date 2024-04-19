#pragma once
#include "Game/EngineBuildPreferences.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <stdint.h>
#include <vector>
#include <string>

#define DX_SAFE_RELEASE(dxObject)		\
{										\
	if ((dxObject) != nullptr)			\
	{									\
		(dxObject)->Release();			\
		(dxObject) = nullptr;			\
	}									\
}										

struct Vertex_PCU;
struct Vertex_PCUTBN;
struct IntVec2;
class Camera;
class Window;
class Texture;
class Image;
class BitmapFont;
class Shader;
class ComputeShader;
class VertexBuffer;
class ConstantBuffer;
class IndexBuffer;
class FrameBuffer;
class StructuredBuffer;
class ComputeOutputBuffer;
class ShadowMap;

enum class BlendMode
{
	INVALID = -1,
	OPAQUE,
	ALPHA,
	ADDITIVE,
	COUNT
};

enum class SamplerMode
{
	INVALID = -1,
	POINT_CLAMP,
	BILINEAR_WRAP,
	COUNT
};

enum class RasterizerMode
{
	INVALID = -1,
	SOLID_CULL_NONE,
	SOLID_CULL_BACK,
	WIREFRAME_CULL_NONE,
	WIREFRAME_CULL_BACK,
	COUNT
};

enum class DepthStencilMode
{
	INVALID = -1,
	DEPTH_DISABLED_STENCIL_DISABLED,
	DEPTH_ENABLED_STENCIL_DISABLED,
	DEPTH_DISABLED_STENCIL_ENABLED,
	DEPTH_ENABLED_STENCIL_ENABLED,
	COUNT
};

struct RendererConfig
{
	Window* m_window = nullptr;
};

class Renderer {
public:
	Renderer(RendererConfig const& config);
	~Renderer();

	void Startup();
	void BeginFrame(FrameBuffer* frameBufferToDrawTo = nullptr);
	void EndFrame();
	void Shutdown();

	void ClearScreen(const Rgba8& clearColor);
	void BeginCamera(const Camera& camera);
	void EndCamera(const Camera& camera);
	void DrawVertexArray(int numVertices, const Vertex_PCU* vertices);
	void DrawVertexArray(int numVertices, const Vertex_PCUTBN* vertices);
	void DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset = 0);
	void DrawVertexAndIndexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int vertexOffset = 0, int indexOffset = 0);

	RendererConfig const& GetConfig() const { return m_config; }

	Texture* CreateOrGetTextureFromFile(const char * imageFilePath);
	BitmapFont* CreateOrGetBitmapFont(const char* bitmapFontFilePathWithNoExtensions);

	void BindConstantBuffer(ConstantBuffer* cbo, unsigned int slot = 0);
	void BindTexture(const Texture* texture, unsigned int slot = 0);
	void BindTextures(const Texture* texture0, const Texture* texture1, const Texture* texture2);
	void BindFrameBuffer(const FrameBuffer* frameBuffer, unsigned int slot = 0);
	void BindShadowMap(const ShadowMap* frameBuffer, unsigned int slot = 0);
	void BindStructuredBufferToVS(const StructuredBuffer* structuredBuffer, unsigned int slot = 0);
	void BindComputeOutputBufferToVS(const ComputeOutputBuffer* computeOutputBuffer, unsigned int slot = 0);
	
	void SetBlendMode(BlendMode blendMode);
	void SetSamplerMode(SamplerMode samplerMode);
	void SetRasterizerMode(RasterizerMode rasterizerMode);
	void SetDepthStencilMode(DepthStencilMode depthStencilMode, unsigned int stencilRef = 0);

	uint8_t GetStencilValueOfClientPos(const IntVec2& clientPos, struct ID3D11Texture2D* depthStencilTexture = nullptr) const;

	Shader* CreateOrGetShader(char const* shaderPathWithoutExtension);
	void BindShader(Shader* shader);
	void RunComputeShader(ComputeShader& computeShader, const std::vector<StructuredBuffer*>& inputSBArray, const std::vector<unsigned int>& inputSBArrayBindSlots, const std::vector<ConstantBuffer*>& inputCBArray, const std::vector<unsigned int>& inputCBArrayBindSlots, ComputeOutputBuffer& computeOutputBuffer, int numGroupsX, int numGroupsY, int numGroupsZ);

	ComputeShader* CreateOrGetComputeShader(char const* computeShaderPathWithoutExtension);
	ComputeShader* CreateOrGetPrecompiledComputeShader(char const* precompiledCSPath);

	VertexBuffer* CreateVertexBuffer(const size_t size, const unsigned int stride, const std::string& vboDebugName = std::string(""), bool isTriangleList = true);
	IndexBuffer* CreateIndexBuffer(const size_t size);
	ConstantBuffer* CreateConstantBuffer(const size_t size);
	FrameBuffer* CreateFrameBuffer(const char* frameBufferName, const IntVec2& dimensions);
	StructuredBuffer* CreateStructuredBuffer(const size_t size, unsigned int byteStride, unsigned int numElements);
	ComputeOutputBuffer* CreateComputeOutputBuffer(const size_t outputBufferSize, unsigned int outputByteStride, unsigned int numOutputElements);

	void SetModelConstants(const Mat44& modelMatrix = Mat44(), const Rgba8& modelColor = Rgba8::WHITE);
	void SetLightConstants(const Vec3& sunDirection = Vec3(0.0f, 0.0f, -1.0f), float sunIntensity = 1.0f, float ambientIntensity = 1.0f, const Vec3& worldEyePosition = Vec3(0.0f, 0.0f, 0.0f), bool hasNormalTexture = true, bool hasSpecularTexture = true, bool hasGlossTexture = true, float specularIntensity = 0.0f, float specularPower = 0.0f);
	
	void CopyCPUToGPU(const void* data, size_t size, unsigned int stride, VertexBuffer*& vbo);
	void CopyCPUToGPU(const void* data, size_t size, ConstantBuffer*& cbo);
	void CopyCPUToGPU(const void* data, size_t size, IndexBuffer*& ibo);
	void CopyCPUToGPU(const void* data, size_t size, unsigned int byteStride, unsigned int numElements, StructuredBuffer*& sbo);

	void SetDebugName(struct ID3D11DeviceChild* object, char const* name);

	struct ID3D11Device* GetDevice() const;
	struct ID3D11DeviceContext* GetDeviceContext() const;

	//These are for the standard Depth Stencil View, not for shadow mapping
	void CreateDepthStencilTextureAndView(struct ID3D11Texture2D*& depthStencilTexture, struct ID3D11DepthStencilView*& depthStencilView, IntVec2 customDim = IntVec2());
	void SetDepthStencilView(struct ID3D11DepthStencilView* depthStencilView = nullptr);
	void ClearDepthStencilView();

	//void * GetOutputDataFromComputeOutputBuffer(const ComputeOutputBuffer& computeOutputBuffer) const;

	//These are for shadow mapping
	ShadowMap* CreateShadowMap(unsigned int width, unsigned int height);
	void ClearDepthStencilViewOfShadowMap(ShadowMap& shadowMap);
	void SetRenderTargetViewOfShadowMap(ShadowMap* shadowMap);

	void SetViewport(Vec2 viewportLeftTop, Vec2 viewportDim);	//Most leftTop of the window is 0, 0

protected:
	//Called before each draw call
	void SetStatesIfChanged();
	void BindVertexBuffer(VertexBuffer* vbo);
	void BindIndexBuffer(IndexBuffer* vbo);
	bool IsShadowMappingSupported() const;

private:
	std::vector<Texture*> m_loadedTextures;
	std::vector<BitmapFont*> m_loadedFonts;
	RendererConfig m_config;

	struct ID3D11Device* m_device = nullptr;
	struct ID3D11DeviceContext* m_deviceContext = nullptr;
	struct IDXGISwapChain* m_swapChain = nullptr;
	struct ID3D11RenderTargetView* m_renderTargetViewForBackBuffer = nullptr;

	void* m_dxgiDebugModule = nullptr;
	void* m_dxgiDebug = nullptr;

	std::vector<Shader*> m_loadedShaders;
	Shader const* m_currentShader = nullptr;
	Shader* m_defaultShader = nullptr;

	std::vector<ComputeShader*> m_loadedComputeShaders;
	ComputeShader const* m_currentComputeShader = nullptr;

	VertexBuffer* m_immediateVBO = nullptr;
	ConstantBuffer* m_cameraCBO = nullptr;
	ConstantBuffer* m_modelCBO = nullptr;
	ConstantBuffer* m_lightCBO = nullptr;

	const Texture* m_defaultTexture = nullptr;

	struct ID3D11DepthStencilView* m_defaultDSV = nullptr;
	struct ID3D11Texture2D* m_defaultDST = nullptr;
	IntVec2 m_defaultDSTDim;

	struct ID3D11BlendState* m_currentBlendState = nullptr;
	struct ID3D11SamplerState* m_currentSamplerState = nullptr;
	struct ID3D11RasterizerState* m_currentRasterizerState = nullptr;
	struct ID3D11DepthStencilState* m_currentDepthStencilState = nullptr;
	unsigned int m_currentStencilRef = 0;

	BlendMode m_desiredBlendMode = BlendMode::INVALID;
	SamplerMode m_desiredSamplerMode = SamplerMode::INVALID;
	RasterizerMode m_desiredRasterizerMode = RasterizerMode::INVALID;
	DepthStencilMode m_desiredDepthStencilMode = DepthStencilMode::INVALID;
	unsigned int m_desiredStencilRef = 0;

	ID3D11BlendState* m_blendStates[(int)(BlendMode::COUNT)] = {};
	ID3D11SamplerState* m_samplerStates[(int)(SamplerMode::COUNT)] = {};
	ID3D11RasterizerState* m_rasterizerStates[(int)(RasterizerMode::COUNT)] = {};
	ID3D11DepthStencilState* m_depthStencilStates[(int)(DepthStencilMode::COUNT)] = {};

//private member functions
private:
	Texture* CreateTextureFromFile(const char* imageFilePath);
	Texture* CreateTextureFromImage(const Image& image);
	BitmapFont* CreateBitmapFont(const char* bitmapFontFilePathWithNoExtensions);
	Texture* GetTextureForFileName(const char* fileName);
	BitmapFont* GetBitmapFontForFileName(const char* bitmapFontFilePathWithNoExtensions);
	bool CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target);

	Shader* CreateShader(char const* shaderPathWithoutExtension);
	Shader* CreateShader(char const* shaderName, char const* shaderSource);

	ComputeShader* CreateComputeShader(char const* shaderPathWithoutExtension);
	ComputeShader* CreateComputeShader(char const* shaderName, char const* shaderSource);

	//Called at startup. m_*States array will be populated here
	void CreateBlendStates();
	void CreateSamplerStates();
	void CreateRasterizerStates();
	void CreateDepthStencilStates();

private:
	FrameBuffer* m_currentlyBoundFrameBuffer = nullptr;
	ID3D11DepthStencilView* m_currentlyBoundDSV = nullptr;
};