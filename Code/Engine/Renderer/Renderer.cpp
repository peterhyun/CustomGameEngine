#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Window/Window.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/FrameBuffer.hpp"
#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/ComputeOutputBuffer.hpp"
#include "Engine/Renderer/ComputeShader.hpp"
#include "Engine/Renderer/ShadowMap.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "ThirdParty/stb/stb_image.h"

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places
#include <math.h>
#include <cassert>
#include <crtdbg.h>
//Including directx 11 header files and such
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")

#if defined(ENGINE_DEBUG_RENDER)
#include <dxgidebug.h>
#pragma comment( lib, "dxguid.lib")
#endif

#if defined(OPAQUE)
#undef OPAQUE
#endif

struct LightConstants
{
	Vec3 SunDirection;
	float SunIntensity = 0.0f;
	float AmbientIntensity = 0.0f;
	Vec3 WorldEyePosition;
	int HasNormalTexture = 0;
	int HasSpecularTexture = 0;
	int HasGlossTexture = 0;
	float SpecularIntensity = 0.0f;
	float SpecularPower = 0.0f;
	int padding[3];
};
static const int k_lightConstantsSlot = 1;

struct CameraConstants
{
	Mat44 ViewMatrix;
	Mat44 ProjectionMatrix;
};
static const int k_cameraConstantsSlot = 2;

struct ModelConstants
{
	Mat44 ModelMatrix;
	float ModelColor[4];
};
static const int k_modelConstantsSlot = 3;

Renderer::Renderer(RendererConfig const& config)
	:m_config(config)
{
}

Renderer::~Renderer()
{
}

void Renderer::Startup() {
	//DXGIDebug settings set
#if defined(ENGINE_DEBUG_RENDER)
	m_dxgiDebugModule = (void*) ::LoadLibraryA("dxgidebug.dll");
	if (m_dxgiDebugModule == nullptr)
	{
		ERROR_AND_DIE("Could not load dxgidebug.dll");
	}
	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	((GetDebugModuleCB) ::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))(__uuidof(IDXGIDebug), &m_dxgiDebug);

	if (m_dxgiDebug == nullptr)
	{
		ERROR_AND_DIE("Could not load debug module.");
	}
#endif
	
	//Create device, device context and swap chain
	DXGI_SWAP_CHAIN_DESC scd; // = {0} not necessary if you're doing ZeroMemory lol
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	Window* window = Window::GetWindowContext();
	IntVec2 clientDimensions;
	if (window) {
		clientDimensions = window->GetClientDimensions();
		m_defaultDSTDim = clientDimensions;
	}
	else {
		ERROR_AND_DIE("You should call Renderer::Startup() AFTER calling Window::Startup()");
	}
	scd.BufferDesc.Width = clientDimensions.x;
	scd.BufferDesc.Height = clientDimensions.y;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.SampleDesc.Count = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 2;
	scd.OutputWindow = static_cast<HWND>(window->GetHwnd());
	scd.Windowed = true;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	unsigned int deviceFlags = 0;
#if defined(ENGINE_DEBUG_RENDER)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		deviceFlags,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&m_swapChain,
		&m_device,
		NULL,
		&m_deviceContext);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create D3D 11 device and swap chain.");
	}

	//Setting Render Target View
	ID3D11Texture2D* backBuffer;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not get swap chain buffer.");
	}
	hr = m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetViewForBackBuffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create render target view for swap chain buffer.");
	}
	backBuffer->Release();

	//Creating & Binding default shader
	m_defaultShader = CreateShader("Data/Shaders/Default");
	BindShader(m_defaultShader);

	//Create immediate vertex buffer
	m_immediateVBO = CreateVertexBuffer(sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	m_cameraCBO = CreateConstantBuffer(sizeof(float) * 32);
	m_modelCBO = CreateConstantBuffer(sizeof(float) * 20);
	m_lightCBO = CreateConstantBuffer(sizeof(LightConstants));

	//Create default 1x1 white texture
	Image defaultTextureImage(IntVec2(2, 2), Rgba8::WHITE);
	m_defaultTexture = CreateTextureFromImage(defaultTextureImage);
	BindTexture(m_defaultTexture);

	//Create Depth Stencil View
	CreateDepthStencilTextureAndView(m_defaultDST, m_defaultDSV);
	m_currentlyBoundDSV = m_defaultDSV;
	
	CreateBlendStates();
	CreateSamplerStates();
	CreateRasterizerStates();
	CreateDepthStencilStates();

	//Setting BlenderMode to Alpha
	SetBlendMode(BlendMode::ALPHA);

	//Set sampler state
	SetSamplerMode(SamplerMode::POINT_CLAMP);

	//Setting Rasterizer State
	SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);

	//Create Depth Stencil State
	SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_DISABLED);

	//Check if shadow mapping is supported on this device
	if (IsShadowMappingSupported()) {
		DebuggerPrintf("Shadow mapping is supported\n");
	}
	else {
		DebuggerPrintf("Shadow mapping is NOT supported\n");
	}
}

void Renderer::Shutdown()
{
	delete m_defaultTexture;
	m_defaultTexture = nullptr;

	delete m_lightCBO;
	m_lightCBO = nullptr;

	delete m_modelCBO;
	m_modelCBO = nullptr;

	delete m_cameraCBO;
	m_cameraCBO = nullptr;

	delete m_immediateVBO;
	m_immediateVBO = nullptr;

	for (int i = 0; i < m_loadedShaders.size() ; i++) {
		if (m_loadedShaders[i]) {
			delete m_loadedShaders[i];
			m_loadedShaders[i] = nullptr;
		}
	}

	for (int i = 0; i < m_loadedComputeShaders.size(); i++) {
		if (m_loadedComputeShaders[i]) {
			delete m_loadedComputeShaders[i];
			m_loadedComputeShaders[i] = nullptr;
		}
	}

	for (int i = 0; i < m_loadedFonts.size(); i++) {
		if (m_loadedFonts[i]) {
			delete m_loadedFonts[i];
			m_loadedFonts[i] = nullptr;
		}
	}

	for (int i = 0; i < m_loadedTextures.size(); i++) {
		if (m_loadedTextures[i]) {
			delete m_loadedTextures[i];
			m_loadedTextures[i] = nullptr;
		}
	}

	DX_SAFE_RELEASE(m_defaultDST);
	DX_SAFE_RELEASE(m_defaultDSV);
	for (int i = 0; i < (int)DepthStencilMode::COUNT; i++) {
		DX_SAFE_RELEASE(m_depthStencilStates[i]);
	}
	for (int i = 0; i < (int)SamplerMode::COUNT; i++) {
		DX_SAFE_RELEASE(m_samplerStates[i]);
	}
	for (int i = 0; i < (int)BlendMode::COUNT; i++) {
		DX_SAFE_RELEASE(m_blendStates[i]);
	}
	for (int i = 0; i < (int)RasterizerMode::COUNT; i++) {
		DX_SAFE_RELEASE(m_rasterizerStates[i]);
	}
	DX_SAFE_RELEASE(m_renderTargetViewForBackBuffer);
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_deviceContext);
	DX_SAFE_RELEASE(m_device);

#if defined(ENGINE_DEBUG_RENDER)
	((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));

	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;

	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif
}

void Renderer::BeginFrame(FrameBuffer* frameBufferToDrawTo)
{
	if (frameBufferToDrawTo == nullptr) {
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetViewForBackBuffer, m_currentlyBoundDSV);
		m_currentlyBoundFrameBuffer = nullptr;
	}
	else {
		m_deviceContext->OMSetRenderTargets(1, &frameBufferToDrawTo->m_renderTargetViewForFrameBuffer, m_currentlyBoundDSV);
		m_currentlyBoundFrameBuffer = frameBufferToDrawTo;
	}
}

void Renderer::EndFrame()
{
	if (m_currentlyBoundFrameBuffer == nullptr) {
 		HRESULT hr = m_swapChain->Present(0, 0);
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
			ERROR_AND_DIE("Device has been lost, application will now terminate");
		}
		return;
	}
	else {	//if (m_currentlyBoundFrameBuffer != nullptr)
		m_deviceContext->ResolveSubresource(m_currentlyBoundFrameBuffer->m_textureForSRV, 0, m_currentlyBoundFrameBuffer->m_textureForRTV, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
		m_currentlyBoundFrameBuffer = nullptr;
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetViewForBackBuffer, m_currentlyBoundDSV);
	}
}

void Renderer::BeginCamera(const Camera& camera) {
	CameraConstants cameraConsts;
	cameraConsts.ViewMatrix = camera.GetViewMatrix();
	cameraConsts.ProjectionMatrix = camera.GetProjectionMatrix();

	CopyCPUToGPU(&cameraConsts, m_cameraCBO->m_size, m_cameraCBO);
	BindConstantBuffer(m_cameraCBO, k_cameraConstantsSlot);

	//Setting the viewport
	Window* window = Window::GetWindowContext();
	IntVec2 clientDimensions;
	if (window) {
		clientDimensions = window->GetClientDimensions();
	}
	else {
		ERROR_AND_DIE("You should call Renderer::BeginCamera() AFTER calling Window::Startup()");
	}
	if (camera.GetViewportAABB2() == AABB2()) {
		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)clientDimensions.x;
		viewport.Height = (float)clientDimensions.y;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		m_deviceContext->RSSetViewports(1, &viewport);
	}
	else {	//If you don't remember this was for supporting 2p in one window. Like in Doomenstein
		D3D11_VIEWPORT viewport;
		ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
		AABB2 cameraViewportAABB2 = camera.GetViewportAABB2();
		Vec2 viewportTopLeft = cameraViewportAABB2.GetTopLeft();
		Vec2 viewportDimensions = cameraViewportAABB2.GetDimensions();
		viewport.TopLeftX = viewportTopLeft.x;
		viewport.TopLeftY = (float)clientDimensions.y - viewportTopLeft.y;
		viewport.Width  = viewportDimensions.x;
		viewport.Height = viewportDimensions.y;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		m_deviceContext->RSSetViewports(1, &viewport);
	}
}

void Renderer::EndCamera(const Camera& camera) {
	UNUSED(camera);
}

void Renderer::DrawVertexArray(int numVertices, const Vertex_PCU* vertices)
{
	//GUARANTEE_OR_DIE(numVertices > 0, "Empty DrawVertexArray() call");
	CopyCPUToGPU(vertices, numVertices * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_immediateVBO);
	DrawVertexBuffer(m_immediateVBO, numVertices, 0);
}

void Renderer::DrawVertexArray(int numVertices, const Vertex_PCUTBN* vertices)
{
	//GUARANTEE_OR_DIE(numVertices > 0, "Empty DrawVertexArray() call");
	CopyCPUToGPU(vertices, numVertices * sizeof(Vertex_PCUTBN), sizeof(Vertex_PCUTBN), m_immediateVBO);
	DrawVertexBuffer(m_immediateVBO, numVertices, 0);
}

void Renderer::DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset)
{
	//Called before each draw call
	SetStatesIfChanged();
	BindVertexBuffer(vbo);
	m_deviceContext->Draw(vertexCount, vertexOffset);
}

void Renderer::DrawVertexAndIndexBuffer(VertexBuffer* vbo, IndexBuffer* ibo, int indexCount, int vertexOffset, int indexOffset)
{
	SetStatesIfChanged();
	BindVertexBuffer(vbo);
	BindIndexBuffer(ibo);
	m_deviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

void Renderer::ClearScreen(const Rgba8& clearColor) {
	GUARANTEE_OR_DIE(m_currentlyBoundDSV != nullptr, "m_currentlyBoundDSV == nullptr");
	float normalizedClearColor[4];
	clearColor.GetAsFloats(normalizedClearColor);

	if (m_currentlyBoundFrameBuffer == nullptr) {
		m_deviceContext->ClearRenderTargetView(m_renderTargetViewForBackBuffer, normalizedClearColor);
	}
	else {
		m_deviceContext->ClearRenderTargetView(m_currentlyBoundFrameBuffer->m_renderTargetViewForFrameBuffer, normalizedClearColor);
	}

	m_deviceContext->ClearDepthStencilView(m_currentlyBoundDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

Texture* Renderer::CreateOrGetTextureFromFile(char const* imageFilePath)
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureForFileName(imageFilePath);
	if (existingTexture)
		return existingTexture;

	// Never seen this texture before!  Let's load it.
	Texture* newTexture = CreateTextureFromFile(imageFilePath);
	return newTexture;
}

BitmapFont* Renderer::CreateOrGetBitmapFont(const char* bitmapFontFilePathWithNoExtensions)
{
	BitmapFont* existingBitmapFont = GetBitmapFontForFileName(bitmapFontFilePathWithNoExtensions);
	if (existingBitmapFont)
		return existingBitmapFont;

	BitmapFont* newBitmapFont = CreateBitmapFont(bitmapFontFilePathWithNoExtensions);
	return newBitmapFont;
}

Texture* Renderer::CreateTextureFromFile(char const* imageFilePath)
{
	Image textureImage(imageFilePath);
	Texture* newTexture = CreateTextureFromImage(textureImage);
	m_loadedTextures.push_back(newTexture);
	return newTexture;
}

Texture* Renderer::CreateTextureFromImage(const Image& image)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = image.m_dimensions.x;
	textureDesc.Height = image.m_dimensions.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.SampleDesc.Count = 1;

	D3D11_SUBRESOURCE_DATA subresourceData;
	ZeroMemory(&subresourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	subresourceData.pSysMem = image.GetRawData();
	subresourceData.SysMemPitch = image.m_dimensions.x * sizeof(Rgba8);	//Have to be the size of bytes of one row

	Texture* newTexture = new Texture();
	HRESULT hr = m_device->CreateTexture2D(&textureDesc, &subresourceData, &newTexture->m_texture);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Texture2D was not created in the Renderer::CreateTextureFromImage()");
	}
	hr = m_device->CreateShaderResourceView(newTexture->m_texture, NULL, &newTexture->m_shaderResourceView);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("ShaderResourceView was not created in the Renderer::CreateTextureFromImage()");
	}
	newTexture->m_name = image.GetImageFilePath();
	newTexture->m_dimensions = image.GetDimensions();

	//Adding debug info
	SetDebugName(newTexture->m_texture, Stringf("Texture2D of texture name %s", newTexture->m_name.c_str()).c_str());
	SetDebugName(newTexture->m_shaderResourceView, Stringf("ShaderResourceView of texture name %s", newTexture->m_name.c_str()).c_str());

	return newTexture;
}

BitmapFont* Renderer::CreateBitmapFont(const char* bitmapFontFilePathWithNoExtensions)
{
	const char* fileExtension = ".png";
	std::string bitmapFontFilePath(bitmapFontFilePathWithNoExtensions);
	bitmapFontFilePath += fileExtension;
	Texture* newFontTexture = CreateTextureFromFile(bitmapFontFilePath.c_str());
	BitmapFont* newBitmapFont = new BitmapFont(bitmapFontFilePathWithNoExtensions, *newFontTexture);
	m_loadedFonts.push_back(newBitmapFont);
	return newBitmapFont;
}

Texture* Renderer::GetTextureForFileName(const char* fileName)
{
	for (int i = 0; i < m_loadedTextures.size() ; i++) {
		if (m_loadedTextures[i] && m_loadedTextures[i]->GetImageFilePath() == fileName) {
			return m_loadedTextures[i];
		}
	}
	return nullptr;
}

BitmapFont* Renderer::GetBitmapFontForFileName(const char* bitmapFontFilePathWithNoExtensions)
{
	for (int i = 0; i < m_loadedFonts.size(); i++) {
		if (m_loadedFonts[i] && m_loadedFonts[i]->GetFontFilePathWithNoExtension() == bitmapFontFilePathWithNoExtensions) {
			return m_loadedFonts[i];
		}
	}
	return nullptr;
}

void Renderer::BindTexture(const Texture* texture, unsigned int slot)
{
	if (texture == nullptr) {
		m_deviceContext->PSSetShaderResources(slot , 1, &m_defaultTexture->m_shaderResourceView);
		return;
	}
	m_deviceContext->PSSetShaderResources(slot, 1, &texture->m_shaderResourceView);
}

void Renderer::BindTextures(const Texture* texture0, const Texture* texture1, const Texture* texture2)
{
	if (texture0 == nullptr) {
		m_deviceContext->PSSetShaderResources(0, 1, &m_defaultTexture->m_shaderResourceView);
		return;
	}
	m_deviceContext->PSSetShaderResources(0, 1, &texture0->m_shaderResourceView);

	if (texture1 == nullptr) {
		m_deviceContext->PSSetShaderResources(1, 1, &m_defaultTexture->m_shaderResourceView);
		return;
	}
	m_deviceContext->PSSetShaderResources(1, 1, &texture1->m_shaderResourceView);

	if (texture2 == nullptr) {
		m_deviceContext->PSSetShaderResources(2, 1, &m_defaultTexture->m_shaderResourceView);
		return;
	}
	m_deviceContext->PSSetShaderResources(2, 1, &texture2->m_shaderResourceView);
}

void Renderer::BindFrameBuffer(const FrameBuffer* frameBuffer, unsigned int slot)
{
	if (frameBuffer == nullptr) {
		m_deviceContext->PSSetShaderResources(slot, 1, &m_defaultTexture->m_shaderResourceView);
		return;
	}
	m_deviceContext->PSSetShaderResources(slot, 1, &frameBuffer->m_shaderResourceViewForFrameBuffer);
}

void Renderer::BindShadowMap(const ShadowMap* shadowMap, unsigned int slot)
{
	if (shadowMap == nullptr) {
		m_deviceContext->PSSetShaderResources(slot, 1, &m_defaultTexture->m_shaderResourceView);
		return;
	}
	m_deviceContext->PSSetShaderResources(slot, 1, &shadowMap->m_shadowMapSRV);
}

void Renderer::BindStructuredBufferToVS(const StructuredBuffer* structuredBuffer, unsigned int slot)
{
	if (structuredBuffer) {
		m_deviceContext->VSSetShaderResources(slot, 1, &structuredBuffer->m_shaderResourceViewForStructuredBuffer);
	}
	else {
		ID3D11ShaderResourceView* nullSRV = nullptr;
		m_deviceContext->VSSetShaderResources(slot, 1, &nullSRV);
	}
}

void Renderer::BindComputeOutputBufferToVS(const ComputeOutputBuffer* computeOutputBuffer, unsigned int slot)
{
	if (computeOutputBuffer) {
		m_deviceContext->VSSetShaderResources(slot, 1, &computeOutputBuffer->m_outputSRV);
	}
	else {
		ID3D11ShaderResourceView* nullSRV = nullptr;
		m_deviceContext->VSSetShaderResources(slot, 1, &nullSRV);
	}
}

ShadowMap* Renderer::CreateShadowMap(unsigned int width, unsigned int height)
{
	ShadowMap* shadowMap = new ShadowMap(width, height);

	D3D11_TEXTURE2D_DESC shadowMapDesc;
	ZeroMemory(&shadowMapDesc, sizeof(D3D11_TEXTURE2D_DESC));
	shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	shadowMapDesc.MipLevels = 1;
	shadowMapDesc.ArraySize = 1;
	shadowMapDesc.SampleDesc.Count = 1;
	shadowMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	shadowMapDesc.Width = width;
	shadowMapDesc.Height = height;
	HRESULT hr = m_device->CreateTexture2D(&shadowMapDesc, nullptr, &shadowMap->m_shadowMapTexture);	
	GUARANTEE_OR_DIE(SUCCEEDED(hr), "Texture creation failed for Shadow Maps!");

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	hr = m_device->CreateDepthStencilView(
		shadowMap->m_shadowMapTexture,
		&depthStencilViewDesc,
		&shadowMap->m_shadowMapDSV
	);
	GUARANTEE_OR_DIE(SUCCEEDED(hr), "DSV creation failed for Shadow Maps!");

	hr = m_device->CreateShaderResourceView(
		shadowMap->m_shadowMapTexture,
		&shaderResourceViewDesc,
		&shadowMap->m_shadowMapSRV
	);
	GUARANTEE_OR_DIE(SUCCEEDED(hr), "SRV creation failed for Shadow Maps!");

	return shadowMap;
}

void Renderer::ClearDepthStencilViewOfShadowMap(ShadowMap& shadowMap)
{
	m_deviceContext->ClearDepthStencilView(shadowMap.m_shadowMapDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::SetRenderTargetViewOfShadowMap(ShadowMap* shadowMap)
{
	if (shadowMap) {
		m_deviceContext->OMSetRenderTargets(0, nullptr, shadowMap->m_shadowMapDSV);
	}
	else if (m_currentlyBoundFrameBuffer)
		m_deviceContext->OMSetRenderTargets(1, &m_currentlyBoundFrameBuffer->m_renderTargetViewForFrameBuffer, m_currentlyBoundDSV);
	else
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetViewForBackBuffer, m_currentlyBoundDSV);
}

void Renderer::SetViewport(Vec2 viewportLeftTop, Vec2 viewportDim)
{
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
	viewport.TopLeftX = viewportLeftTop.x;
	viewport.TopLeftY = viewportLeftTop.y;
	viewport.Width = (float)viewportDim.x;
	viewport.Height = (float)viewportDim.y;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	m_deviceContext->RSSetViewports(1, &viewport);
}

void Renderer::SetStatesIfChanged()
{
	if (m_blendStates[(int)m_desiredBlendMode] != m_currentBlendState) {
		m_currentBlendState = m_blendStates[(int)m_desiredBlendMode];
		float blendFactorArray[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_deviceContext->OMSetBlendState(m_currentBlendState, blendFactorArray, 0xffffffff);
	}

	if (m_samplerStates[(int)m_desiredSamplerMode] != m_currentSamplerState) {
		m_currentSamplerState = m_samplerStates[(int)m_desiredSamplerMode];
		m_deviceContext->PSSetSamplers(0, 1, &m_currentSamplerState);
	}

	if (m_rasterizerStates[(int)m_desiredRasterizerMode] != m_currentRasterizerState) {
		m_currentRasterizerState = m_rasterizerStates[(int)m_desiredRasterizerMode];
		m_deviceContext->RSSetState(m_currentRasterizerState);
	}

	if (m_depthStencilStates[(int)m_desiredDepthStencilMode] != m_currentDepthStencilState) {
		m_currentDepthStencilState = m_depthStencilStates[(int)m_desiredDepthStencilMode];
		m_deviceContext->OMSetDepthStencilState(m_currentDepthStencilState, m_currentStencilRef);
	}

	if (m_currentStencilRef != m_desiredStencilRef) {
		m_currentStencilRef = m_desiredStencilRef;
		m_deviceContext->OMSetDepthStencilState(m_currentDepthStencilState, m_currentStencilRef);
	}
}

void Renderer::SetBlendMode(BlendMode blendMode)
{
	m_desiredBlendMode = blendMode;
}

void Renderer::SetSamplerMode(SamplerMode samplerMode)
{
	m_desiredSamplerMode = samplerMode;
}

void Renderer::SetRasterizerMode(RasterizerMode rasterizerMode)
{
	m_desiredRasterizerMode = rasterizerMode;
}

void Renderer::SetDepthStencilMode(DepthStencilMode depthStencilMode, unsigned int stencilRef)
{
	m_desiredDepthStencilMode = depthStencilMode;
	m_desiredStencilRef = stencilRef;
}

uint8_t Renderer::GetStencilValueOfClientPos(const IntVec2& clientPos, ID3D11Texture2D* depthStencilTexture) const
{
	D3D11_TEXTURE2D_DESC stagingTextureDesc = {};
	if (depthStencilTexture == nullptr) {
		m_defaultDST->GetDesc(&stagingTextureDesc);
	}
	else {
		depthStencilTexture->GetDesc(&stagingTextureDesc);
	}
	stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingTextureDesc.Usage = D3D11_USAGE_STAGING;

	stagingTextureDesc.BindFlags = 0;

	ID3D11Texture2D* pStagingTexture = nullptr;
	HRESULT hr = m_device->CreateTexture2D(&stagingTextureDesc, nullptr, &pStagingTexture);
	if (FAILED(hr)) {
		ERROR_AND_DIE("Was not able to create staging texture!");
	}

	if (depthStencilTexture == nullptr) {
		m_deviceContext->CopyResource(pStagingTexture, m_defaultDST);
	}
	else {
		m_deviceContext->CopyResource(pStagingTexture, depthStencilTexture);
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = m_deviceContext->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
	if (FAILED(hr)) {
		ERROR_AND_DIE("Was not able to map the staging texture to a D3D11_MAPPED_SUBRESOURCE!");
	}

	uint8_t* stencilData = reinterpret_cast<uint8_t*>(mappedResource.pData);

	int offset = (clientPos.y * mappedResource.RowPitch + clientPos.x * 4) + 3;
	uint8_t stencilValue = stencilData[offset];

	m_deviceContext->Unmap(pStagingTexture, 0);
	pStagingTexture->Release();

	return stencilValue;
}

void Renderer::BindShader(Shader* shader)
{
	if (shader == nullptr) {
		m_deviceContext->VSSetShader(m_defaultShader->m_vertexShader, 0, 0);
		m_deviceContext->PSSetShader(m_defaultShader->m_pixelShader, 0, 0);
		m_deviceContext->IASetInputLayout(m_defaultShader->m_inputLayout);
		m_currentShader = m_defaultShader;
		return;
	}
	if (shader->m_vertexShader)
		m_deviceContext->VSSetShader(shader->m_vertexShader, 0, 0);
	if (shader->m_pixelShader)
		m_deviceContext->PSSetShader(shader->m_pixelShader, 0, 0);
	m_deviceContext->IASetInputLayout(shader->m_inputLayout);
	m_currentShader = shader;
}

void Renderer::RunComputeShader(ComputeShader& computeShader, const std::vector<StructuredBuffer*>& inputSBArray, const std::vector<unsigned int>& inputSBArrayBindSlots, const std::vector<ConstantBuffer*>& inputCBArray, const std::vector<unsigned int>& inputCBArrayBindSlots, ComputeOutputBuffer& computeOutputBuffer, int numGroupsX, int numGroupsY, int numGroupsZ)
{
	if (computeOutputBuffer.m_outputStructuredBuffer == nullptr || computeOutputBuffer.m_outputUAV == nullptr) {
		ERROR_AND_DIE("Need to initialize computeOutputBuffer before calling Renderer::RunComputeShader()!");
	}

	if (inputSBArray.size() != inputSBArrayBindSlots.size()) {
		ERROR_AND_DIE("inputSBArray.size() != inputSBArrayBindSlots.size()");
	}

	if (inputCBArray.size() != inputCBArrayBindSlots.size()) {
		ERROR_AND_DIE("inputCBArray.size() != inputCBArrayBindSlots.size()");
	}

	m_deviceContext->CSSetShader(computeShader.m_computeShader, nullptr, 0);
	for (int i = 0; i < inputSBArray.size(); i++) {
		if (inputSBArray[i]) {
			m_deviceContext->CSSetShaderResources(inputSBArrayBindSlots[i], 1, &inputSBArray[i]->m_shaderResourceViewForStructuredBuffer);
		}
		else {
			ERROR_AND_DIE("inputSBArray should not have a nullptr!");
		}
	}
	for (int i = 0; i < inputCBArray.size(); i++) {
		if (inputCBArray[i]) {
			m_deviceContext->CSSetConstantBuffers(inputCBArrayBindSlots[i], 1, &inputCBArray[i]->m_buffer);
		}
		else {
			ERROR_AND_DIE("inputCBArray should not have a nullptr!");
		}
	}
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &computeOutputBuffer.m_outputUAV, nullptr);

	m_deviceContext->Dispatch(numGroupsX, numGroupsY, numGroupsZ);

	m_deviceContext->CSSetShader(nullptr, nullptr, 0);

	ID3D11UnorderedAccessView* ppUAViewnullptr = nullptr;
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &ppUAViewnullptr, nullptr);

	ID3D11ShaderResourceView* ppSRVnullptr = nullptr;
	m_deviceContext->CSSetShaderResources(0, 1, &ppSRVnullptr);

	ID3D11Buffer* ppCBnullptr = nullptr;
	m_deviceContext->CSSetConstantBuffers(0, 1, &ppCBnullptr);
}

ComputeShader* Renderer::CreateOrGetComputeShader(char const* computeShaderPathWithoutExtension)
{
	for (int i = 0; i < m_loadedComputeShaders.size(); i++) {
		if (m_loadedComputeShaders[i] && m_loadedComputeShaders[i]->GetName() == computeShaderPathWithoutExtension) {
			return m_loadedComputeShaders[i];
		}
	}
	ComputeShader* newComputeShader = CreateComputeShader(computeShaderPathWithoutExtension);
	return newComputeShader;
}

ComputeShader* Renderer::CreateOrGetPrecompiledComputeShader(char const* precompiledCSPathWithoutExtension)
{
	for (int i = 0; i < m_loadedComputeShaders.size(); i++) {
		if (m_loadedComputeShaders[i] && m_loadedComputeShaders[i]->GetName() == precompiledCSPathWithoutExtension) {
			return m_loadedComputeShaders[i];
		}
	}

	ComputeShaderConfig config;
	config.m_name = precompiledCSPathWithoutExtension;

	ComputeShader* newShader = new ComputeShader(config);

	//Read in the byte code from the fxc file
	std::vector<unsigned char> computeShaderByteCode;
	FileReadToBuffer(computeShaderByteCode, std::string(precompiledCSPathWithoutExtension) + ".fxc");

	HRESULT hr = m_device->CreateComputeShader(computeShaderByteCode.data(), computeShaderByteCode.size(), nullptr, &newShader->m_computeShader);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create compute shader successfully!");
	}

	SetDebugName(newShader->m_computeShader, Stringf("Compute Shader of shader name %s", precompiledCSPathWithoutExtension).c_str());

	m_loadedComputeShaders.push_back(newShader);
	return newShader;
}

Shader* Renderer::CreateOrGetShader(char const* shaderPathWithoutExtension)
{
	for (int i = 0; i < m_loadedShaders.size(); i++) {
		if (m_loadedShaders[i] && m_loadedShaders[i]->GetName() == shaderPathWithoutExtension) {
			return m_loadedShaders[i];
		}
	}
	Shader* newShader = CreateShader(shaderPathWithoutExtension);
	return newShader;
}

Shader* Renderer::CreateShader(char const* shaderPathWithoutExtension)
{
	std::string shaderPathWithExtension(shaderPathWithoutExtension);
	shaderPathWithExtension += ".hlsl";
	std::string readFileContent;
	FileReadToString(readFileContent, shaderPathWithExtension);
	return CreateShader(shaderPathWithoutExtension, readFileContent.c_str());
}

Shader* Renderer::CreateShader(char const* shaderName, char const* shaderSource)
{
	ShaderConfig shaderConfig;
	shaderConfig.m_name = shaderName;

	Shader * newShader = new Shader(shaderConfig);
	std::vector<unsigned char> vertexShaderByteCode;
	std::vector<unsigned char> pixelShaderByteCode;
	if (!CompileShaderToByteCode(vertexShaderByteCode, shaderName, shaderSource, "VertexMain", "vs_5_0")) {
		ERROR_RECOVERABLE("Vertex Shader NOT Compiled");
	}
	if (!CompileShaderToByteCode(pixelShaderByteCode, shaderName, shaderSource, "PixelMain", "ps_5_0")) {
		ERROR_RECOVERABLE("Pixel Shader NOT Compiled");
	}
	HRESULT hr = m_device->CreateVertexShader(vertexShaderByteCode.data(), vertexShaderByteCode.size(), NULL, &newShader->m_vertexShader);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create vertex shader successfully!");
	}
	hr = m_device->CreatePixelShader(pixelShaderByteCode.data(), pixelShaderByteCode.size(), NULL, &newShader->m_pixelShader);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create pixel shader successfully!");
	}

	//Set up input layout
	ID3D11InputLayout* inputLayout = nullptr;

	if (std::string(shaderSource).find("NORMAL") == std::string::npos) {
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		hr = m_device->CreateInputLayout(inputElementDesc, 3, vertexShaderByteCode.data(), vertexShaderByteCode.size(), &inputLayout);
		if (!SUCCEEDED(hr)) {
			ERROR_AND_DIE("Input Layout was not created!");
		}
		newShader->m_inputLayout = inputLayout;
	}
	else if (std::string(shaderSource).find("JOINT") == std::string::npos) {
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
 			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		hr = m_device->CreateInputLayout(inputElementDesc, 6, vertexShaderByteCode.data(), vertexShaderByteCode.size(), &inputLayout);
		if (!SUCCEEDED(hr)) {
			ERROR_AND_DIE("Input Layout was not created!");
		}
		newShader->m_inputLayout = inputLayout;
	}
	else {
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"JOINTINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"JOINTINDICES", 1, DXGI_FORMAT_R32G32B32A32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"JOINTWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"JOINTWEIGHTS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"MATERIALIDX", 0, DXGI_FORMAT_R32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};
		hr = m_device->CreateInputLayout(inputElementDesc, 11, vertexShaderByteCode.data(), vertexShaderByteCode.size(), &inputLayout);
		if (!SUCCEEDED(hr)) {
			ERROR_AND_DIE("Input Layout was not created!");
		}
		newShader->m_inputLayout = inputLayout;
	}

	SetDebugName(newShader->m_vertexShader, Stringf("VertexShader of shader name %s and source %s", shaderName, shaderSource).c_str());
	SetDebugName(newShader->m_pixelShader, Stringf("PixelShader of shader name %s and source %s", shaderName, shaderSource).c_str());
	SetDebugName(newShader->m_inputLayout, Stringf("InputLayout of shader name %s and source %s", shaderName, shaderSource).c_str());

	m_loadedShaders.push_back(newShader);
	return newShader;
}

ComputeShader* Renderer::CreateComputeShader(char const* shaderPathWithoutExtension)
{
	std::string shaderPathWithExtension(shaderPathWithoutExtension);
	shaderPathWithExtension += ".hlsl";
	std::string readFileContent;
	FileReadToString(readFileContent, shaderPathWithExtension);
	return CreateComputeShader(shaderPathWithoutExtension, readFileContent.c_str());
}

ComputeShader* Renderer::CreateComputeShader(char const* shaderName, char const* shaderSource)
{
	ComputeShaderConfig config;
	config.m_name = shaderName;

	ComputeShader* newShader = new ComputeShader(config);
	std::vector<unsigned char> computeShaderByteCode;
	if (!CompileShaderToByteCode(computeShaderByteCode, shaderName, shaderSource, "ComputeMain", "cs_5_0")) {
		ERROR_RECOVERABLE("Compute Shader NOT Compiled");
	}
	HRESULT hr = m_device->CreateComputeShader(computeShaderByteCode.data(), computeShaderByteCode.size(), nullptr, &newShader->m_computeShader);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create compute shader successfully!");
	}

	SetDebugName(newShader->m_computeShader, Stringf("Compute Shader of shader name %s and source %s", shaderName, shaderSource).c_str());
	
	m_loadedComputeShaders.push_back(newShader);
	return newShader;
}

VertexBuffer* Renderer::CreateVertexBuffer(const size_t size, const unsigned int stride, const std::string& vboDebugName, bool isTriangleList)
{
	VertexBuffer* vertexBuffer = new VertexBuffer(size, stride, isTriangleList);
	//Create a vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (unsigned int)size;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = m_device->CreateBuffer(&bufferDesc, NULL, &vertexBuffer->m_buffer);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create vertex buffer");
	}

	if (vboDebugName == "")
		SetDebugName(vertexBuffer->m_buffer, Stringf("VertexBuffer of size %d", size).c_str());
	else
		SetDebugName(vertexBuffer->m_buffer, vboDebugName.c_str());

	return vertexBuffer;
}

void Renderer::BindVertexBuffer(VertexBuffer* vbo)
{
	if (vbo == nullptr) {
		ERROR_AND_DIE("Need to specify the vbo when calling Renderer::BindVertexBuffer()");
	}
	const UINT stride = vbo->GetStride();
	const UINT offset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &stride, &offset);

	if (vbo->m_isTriangleList) {
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
	else {
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}
}

ConstantBuffer* Renderer::CreateConstantBuffer(const size_t size)
{
	ConstantBuffer* constantBuffer = new ConstantBuffer(size);
	//Create a vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (unsigned int)size;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = m_device->CreateBuffer(&bufferDesc, NULL, &constantBuffer->m_buffer);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create constant buffer");
	}

	SetDebugName(constantBuffer->m_buffer, Stringf("ConstantBuffer of size %d", size).c_str());

	return constantBuffer;
}

void Renderer::BindConstantBuffer(ConstantBuffer* cbo, unsigned int slot)
{
	if (cbo == nullptr) {
		ERROR_AND_DIE("Need to specify the cbo when calling Renderer::BindConstantBuffer()");
	}
	m_deviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	m_deviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
}

IndexBuffer* Renderer::CreateIndexBuffer(const size_t size)
{
	IndexBuffer* indexBuffer = new IndexBuffer(size);
	//Create an index buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (unsigned int)size;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HRESULT hr = m_device->CreateBuffer(&bufferDesc, NULL, &indexBuffer->m_buffer);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create index buffer");
	}

	SetDebugName(indexBuffer->m_buffer, Stringf("IndexBuffer of size %d", size).c_str());

	return indexBuffer;
}

void Renderer::BindIndexBuffer(IndexBuffer* ibo)
{
	if (ibo == nullptr) {
		ERROR_AND_DIE("Need to specify the ibo when calling Renderer::BindIndexBuffer()");
	}
	const UINT offset = 0;
	m_deviceContext->IASetIndexBuffer(ibo->m_buffer, DXGI_FORMAT_R32_UINT, offset);
}

FrameBuffer* Renderer::CreateFrameBuffer(const char* frameBufferName, const IntVec2& dimensions)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = dimensions.x;
	textureDesc.Height = dimensions.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	FrameBuffer* newFrameBuffer = new FrameBuffer();
	HRESULT hr = m_device->CreateTexture2D(&textureDesc, nullptr, &newFrameBuffer->m_textureForRTV);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Texture2D was not created in the Renderer::CreateFrameBuffer()");
	}
	hr = m_device->CreateTexture2D(&textureDesc, nullptr, &newFrameBuffer->m_textureForSRV);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Texture2D was not created in the Renderer::CreateFrameBuffer()");
	}
	hr = m_device->CreateRenderTargetView(newFrameBuffer->m_textureForRTV, NULL, &newFrameBuffer->m_renderTargetViewForFrameBuffer);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("RenderTargetView was not created in the Renderer::CreateTextureFromImage()");
	}
	hr = m_device->CreateShaderResourceView(newFrameBuffer->m_textureForSRV, NULL, &newFrameBuffer->m_shaderResourceViewForFrameBuffer);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("ShaderResourceView was not created in the Renderer::CreateTextureFromImage()");
	}
	newFrameBuffer->m_name = frameBufferName;
	newFrameBuffer->m_dimensions = dimensions;

	//Adding debug info
	SetDebugName(newFrameBuffer->m_textureForRTV, Stringf("TextureForRTV of FrameBuffer of name %s", newFrameBuffer->m_name.c_str()).c_str());
	SetDebugName(newFrameBuffer->m_textureForSRV, Stringf("TextureForSRV of FrameBuffer of name %s", newFrameBuffer->m_name.c_str()).c_str());
	SetDebugName(newFrameBuffer->m_renderTargetViewForFrameBuffer, Stringf("RenderTargetView of framebuffer name %s", newFrameBuffer->m_name.c_str()).c_str());
	SetDebugName(newFrameBuffer->m_shaderResourceViewForFrameBuffer, Stringf("ShaderTargetView of framebuffer name %s", newFrameBuffer->m_name.c_str()).c_str());

	return newFrameBuffer;
}

StructuredBuffer* Renderer::CreateStructuredBuffer(const size_t size, unsigned int byteStride, unsigned int numElements)
{
	StructuredBuffer* structuredBuffer = new StructuredBuffer(size);
	//Create a structured buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = (unsigned int)size;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = byteStride;

	HRESULT hr = m_device->CreateBuffer(&bufferDesc, NULL, &structuredBuffer->m_structuredBuffer);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create structured buffer");
	}

	SetDebugName(structuredBuffer->m_structuredBuffer, Stringf("StructuredBuffer of size %d", size).c_str());

	//Now create the shader resource view (srv)
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = numElements;

	hr = m_device->CreateShaderResourceView(structuredBuffer->m_structuredBuffer, &srvDesc, &structuredBuffer->m_shaderResourceViewForStructuredBuffer);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create a SRV for a structured buffer");
	}

	SetDebugName(structuredBuffer->m_shaderResourceViewForStructuredBuffer, "SRV for Structured Buffer");

	return structuredBuffer;
}

ComputeOutputBuffer* Renderer::CreateComputeOutputBuffer(const size_t outputBufferSize, unsigned int outputByteStride, unsigned int numOutputElements)
{
	ComputeOutputBuffer* computeOutputBuffer = new ComputeOutputBuffer(outputBufferSize);

	//Create a structured buffer for output
	D3D11_BUFFER_DESC outputBufferDesc;
	ZeroMemory(&outputBufferDesc, sizeof(outputBufferDesc));
	outputBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	outputBufferDesc.ByteWidth = (unsigned int)outputBufferSize;
	outputBufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	outputBufferDesc.StructureByteStride = outputByteStride;

	HRESULT hr = m_device->CreateBuffer(&outputBufferDesc, NULL, &computeOutputBuffer->m_outputStructuredBuffer);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create output structured buffer for compute buffer");
	}

	SetDebugName(computeOutputBuffer->m_outputStructuredBuffer, Stringf("ComputeOutputBuffer::OutputStructuredBuffer of size % d", outputBufferSize).c_str());

	// Create the unordered access view (UAV) for output
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = numOutputElements; // Set the appropriate number of elements

	hr = m_device->CreateUnorderedAccessView(computeOutputBuffer->m_outputStructuredBuffer, &uavDesc, &computeOutputBuffer->m_outputUAV);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create a UAV for the compute output buffer");
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = numOutputElements;

	hr = m_device->CreateShaderResourceView(computeOutputBuffer->m_outputStructuredBuffer, &srvDesc, &computeOutputBuffer->m_outputSRV);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create a SRV for the compute output buffer");
	}
	/*
	D3D11_BUFFER_DESC stagingBufferDesc = {};
	stagingBufferDesc.Usage = D3D11_USAGE_STAGING;
	stagingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingBufferDesc.ByteWidth = (unsigned int)computeOutputBuffer->m_outputBufferByteSize;

	hr = m_device->CreateBuffer(&stagingBufferDesc, nullptr, &computeOutputBuffer->m_stagingBuffer);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not create staging buffer for compute buffer");
	}
	*/

	return computeOutputBuffer;
}

void Renderer::SetModelConstants(const Mat44& modelMatrix, const Rgba8& modelColor)
{
	ModelConstants modelConsts;
	modelConsts.ModelMatrix = modelMatrix;
	modelColor.GetAsFloats(modelConsts.ModelColor);

	CopyCPUToGPU(&modelConsts, m_modelCBO->m_size, m_modelCBO);
	BindConstantBuffer(m_modelCBO, k_modelConstantsSlot);
}

void Renderer::SetLightConstants(const Vec3& sunDirection, float sunIntensity, float ambientIntensity, const Vec3& worldEyePosition, bool hasNormalTexture, bool hasSpecularTexture, bool hasGlossTexture, float specularIntensity, float specularPower)
{
	LightConstants lightConstants;
	lightConstants.SunDirection = sunDirection;
	lightConstants.SunIntensity = sunIntensity;
	lightConstants.AmbientIntensity = ambientIntensity;
	lightConstants.WorldEyePosition = worldEyePosition;
	lightConstants.HasNormalTexture = hasNormalTexture ? 1 : 0;
	lightConstants.HasSpecularTexture = hasSpecularTexture ? 1 : 0;
	lightConstants.HasGlossTexture = hasGlossTexture ? 1 : 0;
	lightConstants.SpecularIntensity = specularIntensity;
	lightConstants.SpecularPower = specularPower;

	CopyCPUToGPU(&lightConstants, m_lightCBO->m_size, m_lightCBO);
	BindConstantBuffer(m_lightCBO, k_lightConstantsSlot);
}

bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target)
{
	//Creating vertex shader
	DWORD flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined(ENGINE_DEBUG_RENDER)
	flags = D3DCOMPILE_DEBUG;
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif
	ID3DBlob* compiledCodeBlob = nullptr;
	ID3DBlob* errorMsgBlob = nullptr;
	HRESULT hr = D3DCompile(source, strlen(source), name, NULL, NULL, entryPoint, target, flags, 0, &compiledCodeBlob, &errorMsgBlob);
	if (!SUCCEEDED(hr))
	{
		DebuggerPrintf("Your code is wrong: ");
		DebuggerPrintf((const char *)errorMsgBlob->GetBufferPointer());
		return false;
	}
	outByteCode.assign((unsigned char*)compiledCodeBlob->GetBufferPointer(), (unsigned char*)compiledCodeBlob->GetBufferPointer() + compiledCodeBlob->GetBufferSize());
	
	DX_SAFE_RELEASE(compiledCodeBlob);
	DX_SAFE_RELEASE(errorMsgBlob);
	return true;
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, unsigned int stride, VertexBuffer*& vbo)
{
	if (vbo == nullptr) {
		ERROR_AND_DIE("Need to specify your vbo when calling Renderer::CopyCPUToGPU()");
	}
	if ((vbo->m_size < size)||(vbo->m_stride != stride)) {
		delete vbo;
		vbo = CreateVertexBuffer(size, stride);
	}
	//Copy vertex buffer data from the CPU to the GPU
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	m_deviceContext->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, data, size);
	m_deviceContext->Unmap(vbo->m_buffer, 0);
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, ConstantBuffer*& cbo)
{
	if (cbo == nullptr) {
		ERROR_AND_DIE("Need to specify your cbo when calling Renderer::CopyCPUToGPU()");
	}
	//Copy constant buffer data from the CPU to the GPU
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	m_deviceContext->Map(cbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, data, size);
	m_deviceContext->Unmap(cbo->m_buffer, 0);
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, IndexBuffer*& ibo)
{
	if (ibo == nullptr) {
		ERROR_AND_DIE("Need to specify your ibo when calling Renderer::CopyCPUToGPU()");
	}
	if (ibo->m_size < size) {
		delete ibo;
		ibo = CreateIndexBuffer(size);
	}
	//Copy vertex buffer data from the CPU to the GPU
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	m_deviceContext->Map(ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, data, size);
	m_deviceContext->Unmap(ibo->m_buffer, 0);
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, unsigned int byteStride, unsigned int numElements, StructuredBuffer*& sbo)
{
	if (sbo == nullptr) {
		ERROR_AND_DIE("Need to specify your sbo when calling Renderer::CopyCPUToGPU()");
	}
	if (sbo->m_size < size) {
		delete sbo;
		sbo = CreateStructuredBuffer(size, byteStride, numElements);
	}

	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	m_deviceContext->Map(sbo->m_structuredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, data, size);
	m_deviceContext->Unmap(sbo->m_structuredBuffer, 0);
}

void Renderer::SetDebugName(ID3D11DeviceChild* object, char const* name)
{
#if defined(ENGINE_DEBUG_RENDER)
	HRESULT hr = object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not set debug name \"%s\".", name));
	}
#else
	UNUSED(object);
	UNUSED(name);
#endif
}

ID3D11Device* Renderer::GetDevice() const
{
	return m_device;
}

ID3D11DeviceContext* Renderer::GetDeviceContext() const
{
	return m_deviceContext;
}

void Renderer::CreateDepthStencilTextureAndView(ID3D11Texture2D*& depthStencilTexture, ID3D11DepthStencilView*& depthStencilView, IntVec2 customDim)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	if (customDim == IntVec2()) {
		textureDesc.Width = m_defaultDSTDim.x;
		textureDesc.Height = m_defaultDSTDim.y;
	}
	else {
		GUARANTEE_OR_DIE(customDim.x > 0 && customDim.y > 0, "CustomDim has to be positive!");
		textureDesc.Width = customDim.x;
		textureDesc.Height = customDim.y;
	}
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.SampleDesc.Count = 1;
	HRESULT hr = m_device->CreateTexture2D(&textureDesc, nullptr, &depthStencilTexture);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Texture for depth/stencil testing was not created!");
	}
	hr = m_device->CreateDepthStencilView(depthStencilTexture, nullptr, &depthStencilView);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Depth/Stencil View was not created!");
	}
}

void Renderer::ClearDepthStencilView()
{
	GUARANTEE_OR_DIE(m_currentlyBoundDSV != nullptr, "m_currentlyBoundDSV should NOT be nullptr");
	m_deviceContext->ClearDepthStencilView(m_currentlyBoundDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::SetDepthStencilView(ID3D11DepthStencilView* depthStencilView)
{
	if (depthStencilView == nullptr) {
		if (m_currentlyBoundFrameBuffer == nullptr) {
			m_deviceContext->OMSetRenderTargets(1, &m_renderTargetViewForBackBuffer, m_defaultDSV);
		}
		else {
			m_deviceContext->OMSetRenderTargets(1, &m_currentlyBoundFrameBuffer->m_renderTargetViewForFrameBuffer, m_defaultDSV);
		}
		m_currentlyBoundDSV = m_defaultDSV;
	}
	else {
		if (m_currentlyBoundFrameBuffer == nullptr) {
			m_deviceContext->OMSetRenderTargets(1, &m_renderTargetViewForBackBuffer, depthStencilView);
		}
		else {
			m_deviceContext->OMSetRenderTargets(1, &m_currentlyBoundFrameBuffer->m_renderTargetViewForFrameBuffer, depthStencilView);
		}
		m_currentlyBoundDSV = depthStencilView;
	}
}

bool Renderer::IsShadowMappingSupported() const
{
	GUARANTEE_OR_DIE(m_device != nullptr, "m_device is a nullptr!");
	D3D11_FEATURE_DATA_D3D9_SHADOW_SUPPORT isD3D9ShadowSupported;
	ZeroMemory(&isD3D9ShadowSupported, sizeof(isD3D9ShadowSupported));
	m_device->CheckFeatureSupport(
		D3D11_FEATURE_D3D9_SHADOW_SUPPORT,
		&isD3D9ShadowSupported,
		sizeof(D3D11_FEATURE_D3D9_SHADOW_SUPPORT)
	);
	return isD3D9ShadowSupported.SupportsDepthAsTextureWithLessEqualComparisonFilter;
}

/*
void* Renderer::GetOutputDataFromComputeOutputBuffer(const ComputeOutputBuffer& computeOutputBuffer) const
{
	m_deviceContext->CopyResource(computeOutputBuffer.m_stagingBuffer, computeOutputBuffer.m_outputStructuredBuffer);

	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	HRESULT hr = m_deviceContext->Map(computeOutputBuffer.m_stagingBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Could not map staging buffer to mappedResource!");
	}

	size_t dataSize = computeOutputBuffer.m_outputBufferByteSize;
	void* copiedData = new char[dataSize];
	memcpy(copiedData, mappedResource.pData, dataSize);

	m_deviceContext->Unmap(computeOutputBuffer.m_stagingBuffer, 0);

	return copiedData;
}
*/

void Renderer::CreateBlendStates()
{
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	HRESULT hr = m_device->CreateBlendState(&blendDesc, &m_blendStates[(int)BlendMode::OPAQUE]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("BlendState for BlendMode::OPAQUE couldn't be created!");
	}

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	hr = m_device->CreateBlendState(&blendDesc, &m_blendStates[(int)BlendMode::ALPHA]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("BlendState for BlendMode::ALPHA couldn't be created!");
	}
	
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	hr = m_device->CreateBlendState(&blendDesc, &m_blendStates[(int)BlendMode::ADDITIVE]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("BlendState for BlendMode::ADDITIVE couldn't be created!");
	}

	SetDebugName(m_blendStates[(int)BlendMode::OPAQUE], "BlendState Opaque");
	SetDebugName(m_blendStates[(int)BlendMode::ALPHA], "BlendState Alpha");
	SetDebugName(m_blendStates[(int)BlendMode::ADDITIVE], "BlendState Additive");
}

void Renderer::CreateSamplerStates()
{
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0.f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = 0;

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	HRESULT hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerStates[(int)SamplerMode::POINT_CLAMP]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Sampler State for POINT_CLAMP was not created!");
	}

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerStates[(int)SamplerMode::BILINEAR_WRAP]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Sampler State for BILINEAR_WRAP was not created!");
	}

	SetDebugName(m_samplerStates[(int)SamplerMode::POINT_CLAMP], "SamplerState PointClamp");
	SetDebugName(m_samplerStates[(int)SamplerMode::BILINEAR_WRAP], "SamplerState BilinearWrap");
}

void Renderer::CreateRasterizerStates()
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.AntialiasedLineEnable = true;

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	HRESULT hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_BACK]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Rasterizer State for SOLID_CULL_BACK was not created!");
	}

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;	
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Rasterizer State for SOLID_CULL_NONE was not created!");
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;	
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_BACK]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Rasterizer State for WIREFRAME_CULL_BACK was not created!");
	}

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_NONE]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Rasterizer State for WIREFRAME_CULL_NONE was not created!");
	}

	SetDebugName(m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_BACK], "RasterizerState SolidCullBack");
	SetDebugName(m_rasterizerStates[(int)RasterizerMode::SOLID_CULL_NONE], "RasterizerState SolidCullNone");
	SetDebugName(m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_BACK], "RasterizerState WireframeCullBack");
	SetDebugName(m_rasterizerStates[(int)RasterizerMode::WIREFRAME_CULL_NONE], "RasterizerState WireframeCullNone");
}

void Renderer::CreateDepthStencilStates()
{
	D3D11_DEPTH_STENCIL_DESC stencilDesc;
	ZeroMemory(&stencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	//Depth disabled, stencil disabled
	stencilDesc.DepthEnable = false;	//It was true before, but I changed it to false for my thesis...
	stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	stencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	stencilDesc.StencilEnable = false;
	stencilDesc.StencilReadMask = 0xFF;
	stencilDesc.StencilWriteMask = 0xFF;
	stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	stencilDesc.BackFace = stencilDesc.FrontFace;

	HRESULT hr = m_device->CreateDepthStencilState(&stencilDesc, &m_depthStencilStates[(int)DepthStencilMode::DEPTH_DISABLED_STENCIL_DISABLED]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Depth Stencil State for DEPTH_DISABLED_STENCIL_DISABLED was not created!");
	}

	//Depth Disabled, Stencil Enabled
	stencilDesc.StencilEnable = true;
	stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // Adjust this as needed
	stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; // Adjust this as needed
	stencilDesc.BackFace = stencilDesc.FrontFace;

	hr = m_device->CreateDepthStencilState(&stencilDesc, &m_depthStencilStates[(int)DepthStencilMode::DEPTH_DISABLED_STENCIL_ENABLED]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Depth Stencil State for DEPTH_DISABLED_STENCIL_ENABLED was not created!");
	}

	// Depth Enabled, Stencil Disabled
	stencilDesc.DepthEnable = true;
	stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	stencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	stencilDesc.StencilEnable = false;

	hr = m_device->CreateDepthStencilState(&stencilDesc, &m_depthStencilStates[(int)DepthStencilMode::DEPTH_ENABLED_STENCIL_DISABLED]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Depth Stencil State for DEPTH_ENABLED_STENCIL_DISABLED was not created!");
	}

	// Depth Enabled, Stencil Enabled
	stencilDesc.StencilEnable = true;
	stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; // Adjust this as needed
	stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; // Adjust this as needed
	stencilDesc.BackFace = stencilDesc.FrontFace;

	hr = m_device->CreateDepthStencilState(&stencilDesc, &m_depthStencilStates[(int)DepthStencilMode::DEPTH_ENABLED_STENCIL_ENABLED]);
	if (!SUCCEEDED(hr)) {
		ERROR_AND_DIE("Depth Stencil State for DEPTH_ENABLED_STENCIL_ENABLED was not created!");
	}

	SetDebugName(m_depthStencilStates[(int)DepthStencilMode::DEPTH_DISABLED_STENCIL_DISABLED], "DepthStencilState Depth Disabled Stencil Disabled");
	SetDebugName(m_depthStencilStates[(int)DepthStencilMode::DEPTH_ENABLED_STENCIL_DISABLED], "DepthStencilState Depth Enabled Stencil Disabled");
	SetDebugName(m_depthStencilStates[(int)DepthStencilMode::DEPTH_DISABLED_STENCIL_ENABLED], "DepthStencilState Depth Disabled Stencil Enabled");
	SetDebugName(m_depthStencilStates[(int)DepthStencilMode::DEPTH_ENABLED_STENCIL_ENABLED], "DepthStencilState Depth Enabled Stencil Enabled");
}