#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StopWatch.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

enum class DebugEntityType {
	INVALID = -1,
	WIREFRAME,
	BILLBOARD_OBJECT3D,
	NONBILLBOARD_OBJECT3D,
	BILLBOARD_TEXT3D,
	NONBILLBOARD_TEXT3D,
	MESSAGE,
	SCREENTEXT,
	NUM
};

class DebugEntity {
public:
	DebugEntity(DebugEntityType type, DebugRenderMode debugRenderMode, float lifespan, const Rgba8& startColor, const Rgba8& endColor, const Vec3& position = Vec3(), const EulerAngles& orientation = EulerAngles());	//For now, position and orientation is for texts only
	~DebugEntity();
	void Render(Renderer* renderer, const Camera& camera, int messageArrayIndex = -1) const;

public:
	std::vector<Vertex_PCU> m_verts;
	VertexBuffer* m_vbo = nullptr;
	DebugEntityType m_type = DebugEntityType::INVALID;
	DebugRenderMode m_debugRenderMode = DebugRenderMode::INVALID;
	Stopwatch m_stopwatch;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	Rgba8 m_currentColor = Rgba8::WHITE;
	Vec3 m_position;
	EulerAngles m_orientation;
};

class DebugRenderManager {
public:
	DebugRenderManager(const DebugRenderConfig& config) : m_config(config) {};
	~DebugRenderManager() {};
	void Startup();
	void Shutdown();
	void AddEntityToManager(DebugEntity& entity);
	void RemoveEntityFromAppropriateSubArray(DebugEntity& entity);
	void RenderWorld(const Camera& camera);
	void RenderScreen(const Camera& camera);

private:
	void AddEntityToAllDebugEntitiesArray(DebugEntity& entity);
	void RemoveEntityFromArray(DebugEntity& entity, std::vector<DebugEntity*>& entityArray);
	void RemoveEntityFromCompactArray(DebugEntity& entity, std::vector<DebugEntity*>& compactEntityArray);

public:
	DebugRenderConfig m_config;
	std::vector<DebugEntity*> m_allDebugEntities;
	std::vector<DebugEntity*> m_debugEntities3D;
	std::vector<DebugEntity*> m_debugEntitiesMessages;
	std::vector<DebugEntity*> m_debugEntitiesScreenTexts;
	float m_messageHeight = 20.0f;

private:
	std::mutex m_mutex;
};

DebugRenderManager* g_theDebugRender = nullptr;
BitmapFont* g_theDebugBitmapFont = nullptr;

void DebugRenderSystemStartup(const DebugRenderConfig& config)
{
	if (config.m_renderer == nullptr) {
		ERROR_AND_DIE("DebugRenderConfig.m_renderer is nullptr!");
	}
	g_theDebugBitmapFont = config.m_renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	g_theDebugRender = new DebugRenderManager(config);
	g_theDebugRender->Startup();
	g_theEventSystem->SubscribeEventCallbackFunction("debugrenderclear", Command_DebugRenderClear);
	g_theEventSystem->SubscribeEventCallbackFunction("debugrendertoggle", Command_DebugRenderToggle);
}

void DebugRenderSystemShutdown()
{
	g_theDebugRender->Shutdown();
	delete g_theDebugRender;
	g_theDebugRender = nullptr;
}

void DebugRenderSetVisible()
{
	g_theDebugRender->m_config.m_startHidden = false;
}

void DebugRenderSetHidden()
{
	g_theDebugRender->m_config.m_startHidden = true;
}

void DebugRenderClear()
{
	for (int i = 0; i < g_theDebugRender->m_allDebugEntities.size(); i++) {
		if (g_theDebugRender->m_allDebugEntities[i]) {
			g_theDebugRender->RemoveEntityFromAppropriateSubArray(*g_theDebugRender->m_allDebugEntities[i]);
			delete g_theDebugRender->m_allDebugEntities[i];
			g_theDebugRender->m_allDebugEntities[i] = nullptr;
		}
	}
	g_theDebugRender->m_allDebugEntities.clear();
}

void DebugRenderBeginFrame()
{
	//Update your color
	for (int i = 0; i < g_theDebugRender->m_allDebugEntities.size(); i++) {
		if (g_theDebugRender->m_allDebugEntities[i] && (g_theDebugRender->m_allDebugEntities[i]->m_stopwatch.GetDuration() > 0.0f)) {
			Rgba8 startColor = g_theDebugRender->m_allDebugEntities[i]->m_startColor;
			Rgba8 endColor = g_theDebugRender->m_allDebugEntities[i]->m_endColor;
			float elapsedFraction = g_theDebugRender->m_allDebugEntities[i]->m_stopwatch.GetElapsedFraction();
			g_theDebugRender->m_allDebugEntities[i]->m_currentColor = Rgba8(
				(uchar)Interpolate((float)startColor.r, (float)endColor.r, elapsedFraction),
				(uchar)Interpolate((float)startColor.g, (float)endColor.g, elapsedFraction),
				(uchar)Interpolate((float)startColor.b, (float)endColor.b, elapsedFraction)
			);
		}
	}
}

void DebugRenderWorld(const Camera& camera)
{
	if (g_theDebugRender->m_config.m_startHidden) {
		return;
	}
	g_theDebugRender->RenderWorld(camera);
}

void DebugRenderScreen(const Camera& camera)
{
	if (g_theDebugRender->m_config.m_startHidden) {
		return;
	}
	g_theDebugRender->RenderScreen(camera);
}

void DebugRenderEndFrame()
{
	//Check if your time is done and delete it
	for (int i = 0; i < g_theDebugRender->m_allDebugEntities.size() ; i++) {
		if (g_theDebugRender->m_allDebugEntities[i] && (g_theDebugRender->m_allDebugEntities[i]->m_stopwatch.GetDuration() >= 0.0f) && g_theDebugRender->m_allDebugEntities[i]->m_stopwatch.HasDurationElapsed()) {
			g_theDebugRender->RemoveEntityFromAppropriateSubArray(*g_theDebugRender->m_allDebugEntities[i]);
			delete g_theDebugRender->m_allDebugEntities[i];
			g_theDebugRender->m_allDebugEntities[i] = nullptr;
		}
	}
}

void DebugAddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugEntity* entity = new DebugEntity(DebugEntityType::BILLBOARD_OBJECT3D, mode, duration, startColor, endColor);
	AddVertsForUVSphereZ3D(entity->m_verts, pos, radius, 16, 16);
	entity->m_vbo = g_theDebugRender->m_config.m_renderer->CreateVertexBuffer(entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	g_theDebugRender->m_config.m_renderer->CopyCPUToGPU(entity->m_verts.data(), entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), entity->m_vbo);
	g_theDebugRender->AddEntityToManager(*entity);
}

void DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugEntity* entity = new DebugEntity(DebugEntityType::BILLBOARD_OBJECT3D, mode, duration, startColor, endColor);
	AddVertsForCylinder3D(entity->m_verts, start, end, radius, 16);
	entity->m_vbo = g_theDebugRender->m_config.m_renderer->CreateVertexBuffer(entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	g_theDebugRender->m_config.m_renderer->CopyCPUToGPU(entity->m_verts.data(), entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), entity->m_vbo);
	g_theDebugRender->AddEntityToManager(*entity);
}

void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugEntity* entity = new DebugEntity(DebugEntityType::WIREFRAME, mode, duration, startColor, endColor);
	AddVertsForCylinder3D(entity->m_verts, base, top, radius, 16);
	entity->m_vbo = g_theDebugRender->m_config.m_renderer->CreateVertexBuffer(entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	g_theDebugRender->m_config.m_renderer->CopyCPUToGPU(entity->m_verts.data(), entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), entity->m_vbo);
	g_theDebugRender->AddEntityToManager(*entity);
}

void DebugAddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugEntity* entity = new DebugEntity(DebugEntityType::WIREFRAME, mode, duration, startColor, endColor);
	AddVertsForUVSphereZ3D(entity->m_verts, center, radius, 16, 16);
	entity->m_vbo = g_theDebugRender->m_config.m_renderer->CreateVertexBuffer(entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	g_theDebugRender->m_config.m_renderer->CopyCPUToGPU(entity->m_verts.data(), entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), entity->m_vbo);
	g_theDebugRender->AddEntityToManager(*entity);
}

void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugEntity* entity = new DebugEntity(DebugEntityType::NONBILLBOARD_OBJECT3D, mode, duration, startColor, endColor);
	AddVertsForArrow3D(entity->m_verts, start, end, radius, 16);
	entity->m_vbo = g_theDebugRender->m_config.m_renderer->CreateVertexBuffer(entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	g_theDebugRender->m_config.m_renderer->CopyCPUToGPU(entity->m_verts.data(), entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), entity->m_vbo);
	g_theDebugRender->AddEntityToManager(*entity);
}

void DebugAddWorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugEntity* entity = new DebugEntity(DebugEntityType::NONBILLBOARD_TEXT3D, mode, duration, startColor, endColor);
	g_theDebugBitmapFont->AddVertsForTextAtOriginXForward3D(entity->m_verts, Vec2(0.0f, 0.0f), textHeight, text, Rgba8::WHITE, 1.0f, alignment);
	TransformVertexArray3D(entity->m_verts, transform);
	entity->m_vbo = g_theDebugRender->m_config.m_renderer->CreateVertexBuffer(entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	g_theDebugRender->m_config.m_renderer->CopyCPUToGPU(entity->m_verts.data(), entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), entity->m_vbo);
	g_theDebugRender->AddEntityToManager(*entity);
}

void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	DebugEntity* entity = new DebugEntity(DebugEntityType::BILLBOARD_TEXT3D, mode, duration, startColor, endColor, origin);
	g_theDebugBitmapFont->AddVertsForTextAtOriginXForward3D(entity->m_verts, Vec2(0.0f, 0.0f), textHeight, text, Rgba8::WHITE, 1.0f, alignment);
	entity->m_vbo = g_theDebugRender->m_config.m_renderer->CreateVertexBuffer(entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	g_theDebugRender->m_config.m_renderer->CopyCPUToGPU(entity->m_verts.data(), entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), entity->m_vbo);
	g_theDebugRender->AddEntityToManager(*entity);
}

void DebugAddWorldWireBox(const AABB3& box, float thickness, const Rgba8& startColor, const Rgba8& endColor, float duration, DebugRenderMode mode)
{
	DebugEntity* entity = new DebugEntity(DebugEntityType::WIREFRAME, mode, duration, startColor, endColor);
	AddVertsForWireframeAABB3D(entity->m_verts, box, thickness);
	entity->m_vbo = g_theDebugRender->m_config.m_renderer->CreateVertexBuffer(entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	g_theDebugRender->m_config.m_renderer->CopyCPUToGPU(entity->m_verts.data(), entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), entity->m_vbo);
	g_theDebugRender->AddEntityToManager(*entity);
}

void DebugAddScreenText(const std::string& text, const Vec2& position, float size, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor)
{
	DebugEntity* entity = new DebugEntity(DebugEntityType::SCREENTEXT, DebugRenderMode::ALWAYS, duration, startColor, endColor, Vec3(position, 0.0f));
	g_theDebugBitmapFont->AddVertsForTextInBox2D(entity->m_verts, AABB2::ZERO_TO_ONE, size, text, Rgba8::WHITE, 1.0f, alignment, TextBoxMode::OVERRUN);
	entity->m_vbo = g_theDebugRender->m_config.m_renderer->CreateVertexBuffer(entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	g_theDebugRender->m_config.m_renderer->CopyCPUToGPU(entity->m_verts.data(), entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), entity->m_vbo);
	g_theDebugRender->AddEntityToManager(*entity);
}

//Messages are a special subset of screen text that should be displayed top to bottom in the order they were added
void DebugAddMessage(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor)
{
	DebugEntity* entity = new DebugEntity(DebugEntityType::MESSAGE, DebugRenderMode::ALWAYS, duration, startColor, endColor);
	g_theDebugBitmapFont->AddVertsForTextInBox2D(entity->m_verts, AABB2::ZERO_TO_ONE, g_theDebugRender->m_messageHeight, text, Rgba8::WHITE, 1.0f, Vec2(0.0f, 0.0f), TextBoxMode::OVERRUN);
	entity->m_vbo = g_theDebugRender->m_config.m_renderer->CreateVertexBuffer(entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	g_theDebugRender->m_config.m_renderer->CopyCPUToGPU(entity->m_verts.data(), entity->m_verts.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), entity->m_vbo);
	g_theDebugRender->AddEntityToManager(*entity);
}

bool Command_DebugRenderClear(EventArgs& args)
{
	UNUSED(args);
	DebugRenderClear();
	return true;
}

bool Command_DebugRenderToggle(EventArgs& args)
{
	UNUSED(args);
	g_theDebugRender->m_config.m_startHidden = !g_theDebugRender->m_config.m_startHidden;
	return true;
}

void DebugRenderManager::Startup()
{
}

void DebugRenderManager::Shutdown()
{
	for (int i = 0; i < g_theDebugRender->m_allDebugEntities.size() ; i++) {
		if (g_theDebugRender->m_allDebugEntities[i]) {
			delete g_theDebugRender->m_allDebugEntities[i];
			g_theDebugRender->m_allDebugEntities[i] = nullptr;
		}
	}
}

void DebugRenderManager::AddEntityToManager(DebugEntity& entity)
{
	m_mutex.lock();
	//m_allDebugEntities.push_back(&entity);
	AddEntityToAllDebugEntitiesArray(entity);
  	switch (entity.m_type) {
	case DebugEntityType::MESSAGE: {
		int insertIndex = 0;
		bool isFiniteDuration = entity.m_stopwatch.GetDuration() >= 0.0f;
		if (isFiniteDuration) {	//Just push back
			m_debugEntitiesMessages.push_back(&entity);
		}
		else {	//Insert before the finite duration messages
			for (insertIndex = 0; insertIndex < m_debugEntitiesMessages.size(); insertIndex++) {
				if (m_debugEntitiesMessages[insertIndex]->m_stopwatch.GetDuration() >= 0.0f) {
					m_debugEntitiesMessages.insert(m_debugEntitiesMessages.begin() + insertIndex, &entity);
					m_mutex.unlock();
					return;
				}
			}
			m_debugEntitiesMessages.push_back(&entity);	//Push back into array there are no finite duration messages
		}
		break;
	}
	case DebugEntityType::SCREENTEXT:
		m_debugEntitiesScreenTexts.push_back(&entity);
		break;
	case DebugEntityType::WIREFRAME:
	case DebugEntityType::BILLBOARD_OBJECT3D:
	case DebugEntityType::NONBILLBOARD_OBJECT3D:
	case DebugEntityType::BILLBOARD_TEXT3D:
	case DebugEntityType::NONBILLBOARD_TEXT3D:
		m_debugEntities3D.push_back(&entity);
		break;
	default:
		m_mutex.unlock();
		ERROR_AND_DIE(Stringf("INVALID debug entity type: %d", (int)entity.m_type));
	}
	m_mutex.unlock();
}

void DebugRenderManager::RemoveEntityFromAppropriateSubArray(DebugEntity& entity) {
	m_mutex.lock();
	if (entity.m_type == DebugEntityType::MESSAGE) {
		g_theDebugRender->RemoveEntityFromCompactArray(entity, g_theDebugRender->m_debugEntitiesMessages);
	}
	else if (entity.m_type == DebugEntityType::SCREENTEXT) {
		g_theDebugRender->RemoveEntityFromCompactArray(entity, g_theDebugRender->m_debugEntitiesScreenTexts);
	}
	else {
		g_theDebugRender->RemoveEntityFromArray(entity, g_theDebugRender->m_debugEntities3D);
	}
	m_mutex.unlock();
}

void DebugRenderManager::RenderWorld(const Camera& camera)
{
	m_mutex.lock();
	m_config.m_renderer->BeginCamera(camera);
	for (int i = 0; i < m_debugEntities3D.size(); i++) {
		if (m_debugEntities3D[i]) {
			m_debugEntities3D[i]->Render(m_config.m_renderer, camera);
		}
	}
	m_config.m_renderer->EndCamera(camera);
	m_mutex.unlock();
}

void DebugRenderManager::RenderScreen(const Camera& camera)
{
	m_mutex.lock();
	m_config.m_renderer->BeginCamera(camera);
	for (int i = 0; i < m_debugEntitiesMessages.size(); i++) {
		if (m_debugEntitiesMessages[i]) {
			m_debugEntitiesMessages[i]->Render(m_config.m_renderer, camera, i);
		}
	}
	for (int i = 0; i < m_debugEntitiesScreenTexts.size(); i++) {
		if (m_debugEntitiesScreenTexts[i]) {
			m_debugEntitiesScreenTexts[i]->Render(m_config.m_renderer, camera);
		}
	}
	m_config.m_renderer->EndCamera(camera);
	m_mutex.unlock();
}

void DebugRenderManager::AddEntityToAllDebugEntitiesArray(DebugEntity& entity)
{
	for (int i = 0; i < m_allDebugEntities.size(); i++) {
		if (m_allDebugEntities[i] == nullptr) {
			m_allDebugEntities[i] = &entity;
			return;
		}
	}
	m_allDebugEntities.push_back(&entity);
}

void DebugRenderManager::RemoveEntityFromArray(DebugEntity& entity, std::vector<DebugEntity*>& entityArray) {
	for (int i = 0; i < entityArray.size(); i++) {
		if (&entity == entityArray[i]) {
			entityArray[i] = nullptr;
			break;
		}
	}
}

void DebugRenderManager::RemoveEntityFromCompactArray(DebugEntity& entity, std::vector<DebugEntity*>& compactEntityArray)
{
	for (int i = 0; i < compactEntityArray.size() ; i++) {
		if (&entity == compactEntityArray[i]) {
			compactEntityArray[i] = nullptr;
			compactEntityArray.erase(compactEntityArray.begin() + i);
		}
	}
}

DebugEntity::DebugEntity(DebugEntityType type, DebugRenderMode debugRenderMode, float lifespan, const Rgba8& startColor, const Rgba8& endColor, const Vec3& position, const EulerAngles& orientation)
	: m_type(type), m_debugRenderMode(debugRenderMode), m_stopwatch(lifespan), m_startColor(startColor), m_endColor(endColor), m_position(position), m_orientation(orientation)
{
	m_currentColor = startColor;
	m_stopwatch.Start();
}

DebugEntity::~DebugEntity()
{
	delete m_vbo;
}

void DebugEntity::Render(Renderer* renderer, const Camera& camera, int messageArrayIndex) const
{
	renderer->BindShader(nullptr);
	Mat44 modelMatrix;
	switch (m_type) {
	case DebugEntityType::WIREFRAME:
		renderer->BindTexture(nullptr);
		renderer->SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_BACK);
		break;
	case DebugEntityType::BILLBOARD_OBJECT3D:
		renderer->BindTexture(nullptr);
		renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		break;
	case DebugEntityType::NONBILLBOARD_OBJECT3D:
		renderer->BindTexture(nullptr);
		renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		break;
	case DebugEntityType::BILLBOARD_TEXT3D: {
		renderer->BindTexture(&g_theDebugBitmapFont->GetTexture());
		renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		Mat44 cameraMatrix = camera.GetOrientation().GetAsMatrix_XFwd_YLeft_ZUp();
		cameraMatrix.SetTranslation3D(camera.GetPosition());
		modelMatrix = GetBillboardMatrix(BillboardType::FULL_CAMERA_OPPOSING, cameraMatrix, m_position);
		break;
	}
	case DebugEntityType::NONBILLBOARD_TEXT3D:
		renderer->BindTexture(&g_theDebugBitmapFont->GetTexture());
		renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);	//I don't wanna cull back of Text3D
		break;
	case DebugEntityType::SCREENTEXT:
		renderer->BindTexture(&g_theDebugBitmapFont->GetTexture());
		renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		modelMatrix.SetTranslation2D(Vec2(m_position));
		break;
	case DebugEntityType::MESSAGE:
		renderer->BindTexture(&g_theDebugBitmapFont->GetTexture());
		renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		modelMatrix.SetTranslation2D(Vec2(0.0f, camera.GetCameraAABB2().GetDimensions().y - g_theDebugRender->m_messageHeight * (messageArrayIndex + 1)));
		break;
	default:
		ERROR_AND_DIE(Stringf("INVALID DebugEntityType: %d", (int)m_type));
	}

	switch (m_debugRenderMode) {
	case DebugRenderMode::ALWAYS:
		renderer->SetBlendMode(BlendMode::ALPHA);
		renderer->SetDepthStencilMode(DepthStencilMode::DEPTH_DISABLED_STENCIL_DISABLED);
		renderer->SetModelConstants(modelMatrix, m_currentColor);
		renderer->DrawVertexBuffer(m_vbo, (int)m_verts.size());
		break;
	case DebugRenderMode::USE_DEPTH:
		renderer->SetBlendMode(BlendMode::ALPHA);
		renderer->SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_DISABLED);
		renderer->SetModelConstants(modelMatrix, m_currentColor);
		renderer->DrawVertexBuffer(m_vbo, (int)m_verts.size());
		break;
	case DebugRenderMode::X_RAY:
		renderer->SetBlendMode(BlendMode::ALPHA);
		renderer->SetDepthStencilMode(DepthStencilMode::DEPTH_DISABLED_STENCIL_DISABLED);
		renderer->SetModelConstants(modelMatrix, m_currentColor.GetDarkenedColor(0.7f, true));
		renderer->DrawVertexBuffer(m_vbo, (int)m_verts.size());
		renderer->SetBlendMode(BlendMode::OPAQUE);
		renderer->SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_DISABLED);
		renderer->SetModelConstants(modelMatrix, m_currentColor);
		renderer->DrawVertexBuffer(m_vbo, (int)m_verts.size());
		break;
	default:
		ERROR_AND_DIE("DebugRenderMode Invalid!");
	}
}