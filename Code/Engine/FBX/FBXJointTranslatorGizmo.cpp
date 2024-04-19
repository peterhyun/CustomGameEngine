#include "Engine/Fbx/FBXJointTranslatorGizmo.hpp"
#include "Engine/Fbx/FBXJointGizmosManager.hpp"
#include "Engine/Fbx/FBxJoint.hpp"
#include "Engine/Fbx/FBXModel.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/LineSegment3.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Window/Window.hpp"
#include <limits>

#define NOMINMAX

//Including directx 11 header files and such
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

#if defined(OPAQUE)
#undef OPAQUE
#endif

FBXJointTranslatorGizmo::FBXJointTranslatorGizmo(FBXJointGizmosManager& manager, FBXModel& model, Renderer& renderer)
	: m_manager(manager), m_model(model), m_renderer(renderer), m_virtualPlane(Vec3(0.0f, 0.0f, 1.0f), Vec3()), m_axisLineOnVirtualPlane(Vec3(1.0f, 0.0f, 0.0f), Vec3())
{
}

FBXJointTranslatorGizmo::~FBXJointTranslatorGizmo()
{
	delete m_vboX;
	m_vboX = nullptr;

	delete m_vboY;
	m_vboY = nullptr;

	delete m_vboZ;
	m_vboZ = nullptr;

	delete m_vboXSelected;
	m_vboXSelected = nullptr;

	delete m_vboYSelected;
	m_vboYSelected = nullptr;

	delete m_vboZSelected;
	m_vboZSelected = nullptr;

	delete m_vboSphere;
	m_vboSphere = nullptr;

	delete m_vboSphereSelected;
	m_vboSphereSelected = nullptr;

	DX_SAFE_RELEASE(m_depthStencilTexture);
	DX_SAFE_RELEASE(m_depthStencilView);
}

void FBXJointTranslatorGizmo::Startup()
{
	unsigned int availableStencilValue = m_model.GetMaxJointStencilValue() + 1;
	m_stencilValX = availableStencilValue;
	m_stencilValY = availableStencilValue + 1;
	m_stencilValZ = availableStencilValue + 2;
	m_stencilValSphere = availableStencilValue + 3;
	SetGPUData();
}

void FBXJointTranslatorGizmo::Update(const Camera& worldCamera)
{
	/*
	FBXJoint* selectedJoint = m_manager.GetSelectedJoint();
	if (selectedJoint) {
		UpdateJointClientPosVariable(worldCamera);
	}
	*/
	UpdateGlobalTransform(worldCamera);
	UpdateFromKeyboard(worldCamera);
	UpdateFromMouseMovement(worldCamera);
}

void FBXJointTranslatorGizmo::Render() const
{
	FBXJoint* selectedJoint = m_manager.GetSelectedJoint();
	if (selectedJoint == nullptr)
		return;

	Mat44 globalTransformForThisJointForThisFrame(m_xAxisVec, m_yAxisVec, m_zAxisVec, m_xyzAxisPos);
	if (m_manager.IsIKSocketMode()) {
		IKSocket& socket = selectedJoint->GetRefToIKSocket();
		socket.Render(m_renderer);
	}

	m_renderer.SetModelConstants(globalTransformForThisJointForThisFrame);
	m_renderer.SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	if (m_state == FBXJointTranslatorGizmoState::XPressed) {
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_DISABLED_STENCIL_ENABLED, 0);
		m_renderer.DrawVertexBuffer(m_vboXSelected, (int)m_verticesXSelected.size());
	}
	else if (m_state == FBXJointTranslatorGizmoState::YPressed) {
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_DISABLED_STENCIL_ENABLED, 0);
		m_renderer.DrawVertexBuffer(m_vboYSelected, (int)m_verticesYSelected.size());
	}
	else if (m_state == FBXJointTranslatorGizmoState::ZPressed) {
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_DISABLED_STENCIL_ENABLED, 0);
		m_renderer.DrawVertexBuffer(m_vboZSelected, (int)m_verticesZSelected.size());
	}
	else if (m_state == FBXJointTranslatorGizmoState::CenterPressed) {
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_DISABLED_STENCIL_ENABLED, 0);
		m_renderer.DrawVertexBuffer(m_vboXSelected, (int)m_verticesXSelected.size());
		m_renderer.DrawVertexBuffer(m_vboYSelected, (int)m_verticesYSelected.size());
		m_renderer.DrawVertexBuffer(m_vboZSelected, (int)m_verticesZSelected.size());
		m_renderer.DrawVertexBuffer(m_vboSphereSelected, (int)m_verticesSphereSelected.size());
	}
	else {
		m_renderer.SetDepthStencilView(m_depthStencilView);
		m_renderer.ClearDepthStencilView();

		m_renderer.SetBlendMode(BlendMode::ALPHA);
		m_renderer.SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_DISABLED);

		m_renderer.SetBlendMode(BlendMode::OPAQUE);
		m_renderer.SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_ENABLED, m_stencilValX);
		m_renderer.DrawVertexBuffer(m_vboX, (int)m_verticesX.size());
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_ENABLED, m_stencilValY);
		m_renderer.DrawVertexBuffer(m_vboY, (int)m_verticesY.size());
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_ENABLED, m_stencilValZ);
		m_renderer.DrawVertexBuffer(m_vboZ, (int)m_verticesZ.size());
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_DISABLED_STENCIL_ENABLED, m_stencilValSphere);
		m_renderer.DrawVertexBuffer(m_vboSphere, (int)m_verticesSphere.size());

		m_renderer.SetDepthStencilView(nullptr);	//Have to set the view back to the renderer default depth stencil view
	}
}

void FBXJointTranslatorGizmo::SetGPUData()
{
	m_renderer.CreateDepthStencilTextureAndView(m_depthStencilTexture, m_depthStencilView);
	/*
	AddVertsForRing3D(m_verticesX, 24, Vec3(0.0f, 0.0f, 0.0f), 4.0f, 4.5f, Vec3(1.0f, 0.0f, 0.0f), Rgba8::RED);
	AddVertsForRing3D(m_verticesY, 24, Vec3(0.0f, 0.0f, 0.0f), 4.0f, 4.5f, Vec3(0.0f, 1.0f, 0.0f), Rgba8::GREEN);
	AddVertsForRing3D(m_verticesZ, 24, Vec3(0.0f, 0.0f, 0.0f), 4.0f, 4.5f, Vec3(0.0f, 0.0f, 1.0f), Rgba8::BLUE);
	*/
	AddVertsForSphere3D(m_verticesSphere, Vec3(0.0f, 0.0f, 0.0f), m_sphereRadius, Rgba8::WHITE);
	m_vboSphere = m_renderer.CreateVertexBuffer(m_verticesSphere.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboSphere in FBXJointTranslatorGizmo");
	m_renderer.CopyCPUToGPU(m_verticesSphere.data(), m_verticesSphere.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboSphere);

	AddVertsForSphere3D(m_verticesSphereSelected, Vec3(0.0f, 0.0f, 0.0f), m_sphereRadius, Rgba8::YELLOW);
	m_vboSphereSelected = m_renderer.CreateVertexBuffer(m_verticesSphereSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboSphereSelected in FBXJointTranslatorGizmo");
	m_renderer.CopyCPUToGPU(m_verticesSphereSelected.data(), m_verticesSphereSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboSphereSelected);

	AddVertsForArrow3D(m_verticesX, Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f) * m_arrowLength, m_arrowRadius, m_numSlicesArrow, Rgba8::RED);
	AddVertsForArrow3D(m_verticesY, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) * m_arrowLength, m_arrowRadius, m_numSlicesArrow, Rgba8::GREEN);
	AddVertsForArrow3D(m_verticesZ, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f) * m_arrowLength, m_arrowRadius, m_numSlicesArrow, Rgba8::BLUE);

	m_vboX = m_renderer.CreateVertexBuffer(m_verticesX.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboX in FBXJointTranslatorGizmo");
	m_vboY = m_renderer.CreateVertexBuffer(m_verticesY.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboY in FBXJointTranslatorGizmo");
	m_vboZ = m_renderer.CreateVertexBuffer(m_verticesZ.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboZ in FBXJointTranslatorGizmo");

	m_renderer.CopyCPUToGPU(m_verticesX.data(), m_verticesX.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboX);
	m_renderer.CopyCPUToGPU(m_verticesY.data(), m_verticesY.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboY);
	m_renderer.CopyCPUToGPU(m_verticesZ.data(), m_verticesZ.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboZ);

	AddVertsForArrow3D(m_verticesXSelected, Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f) * m_arrowLength, m_arrowRadius, m_numSlicesArrow, Rgba8::YELLOW);
	AddVertsForArrow3D(m_verticesYSelected, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f) * m_arrowLength, m_arrowRadius, m_numSlicesArrow, Rgba8::YELLOW);
	AddVertsForArrow3D(m_verticesZSelected, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f) * m_arrowLength, m_arrowRadius, m_numSlicesArrow, Rgba8::YELLOW);

	m_vboXSelected = m_renderer.CreateVertexBuffer(m_verticesXSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboXSelected in FBXJointTranslatorGizmo");
	m_vboYSelected = m_renderer.CreateVertexBuffer(m_verticesYSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboYSelected in FBXJointTranslatorGizmo");
	m_vboZSelected = m_renderer.CreateVertexBuffer(m_verticesZSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboZSelected in FBXJointTranslatorGizmo");
	
	m_renderer.CopyCPUToGPU(m_verticesXSelected.data(), m_verticesXSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboXSelected);
	m_renderer.CopyCPUToGPU(m_verticesYSelected.data(), m_verticesYSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboYSelected);
	m_renderer.CopyCPUToGPU(m_verticesZSelected.data(), m_verticesZSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboZSelected);
}

void FBXJointTranslatorGizmo::UpdateFromKeyboard(const Camera& worldCamera)
{
	m_isLMBBeingUsed = false;
	if (g_theInput->IsCursorHidden())
		return;

	if (m_model.GetJointVisibility() == false) {
		return;
	}

	FBXJoint* selectedJoint = m_manager.GetSelectedJoint();
	if (selectedJoint == nullptr)
		return;

	//Calculating the virtual plane
	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		IntVec2 cursorClientPos = g_theInput->GetCursorClientPos();
		uint8_t selectedStencilValueFromRotatorGizmoTexture = m_renderer.GetStencilValueOfClientPos(cursorClientPos, m_depthStencilTexture);

		IKSocket& ikSocket = selectedJoint->GetRefToIKSocket();
		if (selectedStencilValueFromRotatorGizmoTexture == m_stencilValX) {
			if (m_manager.IsIKSocketMode() && ikSocket.IsMovingWithJoints()) {
				ikSocket.SetIsMovingWithJoints(false, nullptr);
			}
			m_state = FBXJointTranslatorGizmoState::XPressed;
			m_cursorPosPreviousFrame = g_theInput->GetCursorClientPos();
			m_isLMBBeingUsed = true;

			int numFirstPlane, numSecondPlane;
			if (DoesSpawnedPlaneCoverAllCornersOfWindow(m_xAxisVec, m_yAxisVec, m_xyzAxisPos, worldCamera, numFirstPlane)) {
				m_virtualPlane = GetPlaneFromTwoVec3sAndPoint(m_xAxisVec, m_yAxisVec, m_xyzAxisPos);
			}
			else if (DoesSpawnedPlaneCoverAllCornersOfWindow(m_zAxisVec, m_xAxisVec, m_xyzAxisPos, worldCamera, numSecondPlane)) {
				m_virtualPlane = GetPlaneFromTwoVec3sAndPoint(m_zAxisVec, m_xAxisVec, m_xyzAxisPos);
			}
			else {
				if (numFirstPlane > numSecondPlane) {
					m_virtualPlane = GetPlaneFromTwoVec3sAndPoint(m_xAxisVec, m_yAxisVec, m_xyzAxisPos);
				}
				else {
					m_virtualPlane = GetPlaneFromTwoVec3sAndPoint(m_zAxisVec, m_xAxisVec, m_xyzAxisPos);
				}
			}
			m_axisLineOnVirtualPlane = InfiniteLine3(m_xAxisVec, m_xyzAxisPos);
		}
		else if (selectedStencilValueFromRotatorGizmoTexture == m_stencilValY) {
			if (m_manager.IsIKSocketMode() && ikSocket.IsMovingWithJoints()) {
				ikSocket.SetIsMovingWithJoints(false, nullptr);
			}
			m_state = FBXJointTranslatorGizmoState::YPressed;
			m_cursorPosPreviousFrame = g_theInput->GetCursorClientPos();
			m_isLMBBeingUsed = true;

			int numFirstPlane, numSecondPlane;
			if (DoesSpawnedPlaneCoverAllCornersOfWindow(m_yAxisVec, m_zAxisVec, m_xyzAxisPos, worldCamera, numFirstPlane)) {
				m_virtualPlane = GetPlaneFromTwoVec3sAndPoint(m_yAxisVec, m_zAxisVec, m_xyzAxisPos);
			}
			else if (DoesSpawnedPlaneCoverAllCornersOfWindow(m_xAxisVec, m_yAxisVec, m_xyzAxisPos, worldCamera, numSecondPlane)) {
				m_virtualPlane = GetPlaneFromTwoVec3sAndPoint(m_xAxisVec, m_yAxisVec, m_xyzAxisPos);
			}
			else {
				if (numFirstPlane > numSecondPlane) {
					m_virtualPlane = GetPlaneFromTwoVec3sAndPoint(m_yAxisVec, m_zAxisVec, m_xyzAxisPos);
				}
				else {
					m_virtualPlane = GetPlaneFromTwoVec3sAndPoint(m_xAxisVec, m_yAxisVec, m_xyzAxisPos);
				}
			}
			m_axisLineOnVirtualPlane = InfiniteLine3(m_yAxisVec, m_xyzAxisPos);
		}
		else if (selectedStencilValueFromRotatorGizmoTexture == m_stencilValZ) {
			if (m_manager.IsIKSocketMode() && ikSocket.IsMovingWithJoints()) {
				ikSocket.SetIsMovingWithJoints(false, nullptr);
			}
			m_state = FBXJointTranslatorGizmoState::ZPressed;
			m_cursorPosPreviousFrame = g_theInput->GetCursorClientPos();
			m_isLMBBeingUsed = true;

			int numFirstPlane, numSecondPlane;
			if (DoesSpawnedPlaneCoverAllCornersOfWindow(m_zAxisVec, m_xAxisVec, m_xyzAxisPos, worldCamera, numFirstPlane)) {
				m_virtualPlane = GetPlaneFromTwoVec3sAndPoint(m_zAxisVec, m_xAxisVec, m_xyzAxisPos);
			}
			else if (DoesSpawnedPlaneCoverAllCornersOfWindow(m_yAxisVec, m_zAxisVec, m_xyzAxisPos, worldCamera, numSecondPlane)) {
				m_virtualPlane = GetPlaneFromTwoVec3sAndPoint(m_yAxisVec, m_zAxisVec, m_xyzAxisPos);
			}
			else {
				if (numFirstPlane > numSecondPlane) {
					m_virtualPlane = GetPlaneFromTwoVec3sAndPoint(m_zAxisVec, m_xAxisVec, m_xyzAxisPos);
				}
				else {
					m_virtualPlane = GetPlaneFromTwoVec3sAndPoint(m_yAxisVec, m_zAxisVec, m_xyzAxisPos);
				}
			}
			m_axisLineOnVirtualPlane = InfiniteLine3(m_zAxisVec, m_xyzAxisPos);
		}
		else if (selectedStencilValueFromRotatorGizmoTexture == m_stencilValSphere) {
			if (m_manager.IsIKSocketMode() && ikSocket.IsMovingWithJoints()) {
				ikSocket.SetIsMovingWithJoints(false, nullptr);
			}
			m_state = FBXJointTranslatorGizmoState::CenterPressed;
			m_cursorPosPreviousFrame = g_theInput->GetCursorClientPos();
			m_isLMBBeingUsed = true;

			Vec3 viewRayWorldSpace = GetCursorRayDirectionWorldSpace(g_theInput->GetNormalizedCursorPos(), worldCamera);
			m_virtualPlane = Plane3D(viewRayWorldSpace, m_xyzAxisPos);
		}

		if (m_state != FBXJointTranslatorGizmoState::NONE) {
			if (m_state != FBXJointTranslatorGizmoState::CenterPressed) {
				Vec3 worldRay = GetCursorRayDirectionWorldSpace(g_theInput->GetNormalizedCursorPos(), worldCamera);
				float maxFloat = std::numeric_limits<float>::max();
				RaycastResult3D result = RaycastVsPlane3D(worldCamera.GetPosition(), worldRay, maxFloat, m_virtualPlane);
				GUARANTEE_OR_DIE(result.m_didImpact, "Need to find a different plane!");
				m_offsetFromProjectedInitialClickToAxisPos = m_xyzAxisPos - GetProjectedPointOntoInfiniteLine3D(result.m_impactPos, m_axisLineOnVirtualPlane);
			}
			else {
				Vec3 worldRay = GetCursorRayDirectionWorldSpace(g_theInput->GetNormalizedCursorPos(), worldCamera);
				float maxFloat = std::numeric_limits<float>::max();
				RaycastResult3D result = RaycastVsPlane3D(worldCamera.GetPosition(), worldRay, maxFloat, m_virtualPlane);
				GUARANTEE_OR_DIE(result.m_didImpact, "Need to find a different plane!");
				m_offsetFromInitialClickToCenterPos = m_xyzAxisPos - result.m_impactPos;
			}
			//m_pointOnVirtualPlaneProjectedOntoAxisOnInitialClick = GetProjectedPointOntoInfiniteLine3D(result.m_impactPos, m_axisLineOnVirtualPlane);
			//DebugAddWorldPoint(m_pointOnVirtualPlaneProjectedOntoAxisOnInitialClick, 1.0f, 2.f, Rgba8::CYAN, Rgba8::CYAN, DebugRenderMode::ALWAYS);
		}
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_LMB)) {
		if (m_state == FBXJointTranslatorGizmoState::XPressed || m_state == FBXJointTranslatorGizmoState::YPressed || m_state == FBXJointTranslatorGizmoState::ZPressed || m_state == FBXJointTranslatorGizmoState::CenterPressed) {
			m_state = FBXJointTranslatorGizmoState::NONE;
		}
	}
}

void FBXJointTranslatorGizmo::UpdateFromMouseMovement(const Camera& worldCamera)
{
	FBXJoint* selectedJoint = m_manager.GetSelectedJoint();
	if (selectedJoint == nullptr)
		return;

	if (m_state != FBXJointTranslatorGizmoState::NONE) {
		IntVec2 currentClientPos = g_theInput->GetCursorClientPos();
		IntVec2 deltaClientPos = currentClientPos - m_cursorPosPreviousFrame;
		deltaClientPos.y *= -1;
		if (GetAbs(deltaClientPos.x) < m_mouseMoveThreshold && GetAbs(deltaClientPos.y) < m_mouseMoveThreshold) {
			return;
		}
		else {
			m_cursorPosPreviousFrame = currentClientPos;
		}
		Vec3 worldRay = GetCursorRayDirectionWorldSpace(g_theInput->GetNormalizedCursorPos(), worldCamera);
		float maxFloat = std::numeric_limits<float>::max();
		RaycastResult3D result = RaycastVsPlane3D(worldCamera.GetPosition(), worldRay, maxFloat, m_virtualPlane);
		Vec3 newCenterPos;
		if (m_state != FBXJointTranslatorGizmoState::CenterPressed) {
			if (result.m_didImpact) {
				m_latestProjectionPoint = GetProjectedPointOntoInfiniteLine3D(result.m_impactPos, m_axisLineOnVirtualPlane);
			}
			newCenterPos = m_latestProjectionPoint + m_offsetFromProjectedInitialClickToAxisPos;
			
		}
		else {
			GUARANTEE_OR_DIE(result.m_didImpact, "Need to find a different plane!");
			newCenterPos = result.m_impactPos + m_offsetFromInitialClickToCenterPos;
		}
		if (m_manager.IsIKSocketMode()) {
			IKSocket& ikSocket = selectedJoint->GetRefToIKSocket();
			ikSocket.SetGlobalPos(newCenterPos);
		}
		else {
			selectedJoint->SetLocalDeltaTranslate(newCenterPos - selectedJoint->GetOriginalLocalTranslate()); //Has a bug while trying to change it mid-animation
			m_model.SetDDMNeedsRecalculation();
		}
	}
}

/*
void FBXJointTranslatorGizmo::UpdateJointClientPosVariable(const Camera& worldCamera)
{
}
*/

bool FBXJointTranslatorGizmo::DoesSpawnedPlaneCoverAllCornersOfWindow(const Vec3& fixedAxis, const Vec3& candidateAxis, const Vec3& gizmoPos, const Camera& worldCamera, int& numCoveredCorners)
{
	const float maxFloat = std::numeric_limits<float>::max();

	numCoveredCorners = 0;

	Plane3D candidatePlane = GetPlaneFromTwoVec3sAndPoint(fixedAxis, candidateAxis, gizmoPos);
	Vec3 leftBottomWorldRay = GetCursorRayDirectionWorldSpace(Vec2(0.0f, 0.0f), worldCamera);
	RaycastResult3D leftBottomResult = RaycastVsPlane3D(worldCamera.GetPosition(), leftBottomWorldRay, maxFloat, candidatePlane);
	if (leftBottomResult.m_didImpact)
		numCoveredCorners++;

	Vec3 leftTopWorldRay = GetCursorRayDirectionWorldSpace(Vec2(0.0f, 1.0f), worldCamera);
	RaycastResult3D leftTopResult = RaycastVsPlane3D(worldCamera.GetPosition(), leftTopWorldRay, maxFloat, candidatePlane);
	if (leftTopResult.m_didImpact)
		numCoveredCorners++;

	Vec3 rightBottomWorldRay = GetCursorRayDirectionWorldSpace(Vec2(1.0f, 0.0f), worldCamera);
	RaycastResult3D rightBottomResult = RaycastVsPlane3D(worldCamera.GetPosition(), rightBottomWorldRay, maxFloat, candidatePlane);
	if (rightBottomResult.m_didImpact)
		numCoveredCorners++;

	Vec3 rightTopWorldRay = GetCursorRayDirectionWorldSpace(Vec2(1.0f, 1.0f), worldCamera);
	RaycastResult3D rightTopResult = RaycastVsPlane3D(worldCamera.GetPosition(), rightTopWorldRay, maxFloat, candidatePlane);
	if (rightTopResult.m_didImpact)
		numCoveredCorners++;

	return leftBottomResult.m_didImpact && leftTopResult.m_didImpact && rightBottomResult.m_didImpact && rightTopResult.m_didImpact;
}

void FBXJointTranslatorGizmo::UpdateGlobalTransform(const Camera& worldCamera)
{
	FBXJoint* selectedJoint = m_manager.GetSelectedJoint();
	if (selectedJoint == nullptr)
		return;
	IKSocket& ikSocket = selectedJoint->GetRefToIKSocket();

	Mat44 globalTransform;
	if (m_manager.IsIKSocketMode()) {
		globalTransform = ikSocket.GetGlobalTransform();
	}
	else {
		globalTransform = selectedJoint->GetGlobalTransformForThisFrame();
	}
	m_xyzAxisPos = globalTransform.GetTranslation3D();

	if (m_state != FBXJointTranslatorGizmoState::NONE) {
		return;
	}

	Vec3 worldCameraPos = worldCamera.GetPosition();
	if (m_isAlignedToGlobalAxis) {
		m_xAxisVec = GetAxisHeadingToCamera(Vec3(1.0f, 0.0f, 0.0f), m_xyzAxisPos, worldCameraPos);
		m_yAxisVec = GetAxisHeadingToCamera(Vec3(0.0f, 1.0f, 0.0f), m_xyzAxisPos, worldCameraPos);
		m_zAxisVec = GetAxisHeadingToCamera(Vec3(0.0f, 0.0f, 1.0f), m_xyzAxisPos, worldCameraPos);
	}
	else {
		m_xAxisVec = GetAxisHeadingToCamera(globalTransform.GetIBasis3D(), m_xyzAxisPos, worldCameraPos);
		m_yAxisVec = GetAxisHeadingToCamera(globalTransform.GetJBasis3D(), m_xyzAxisPos, worldCameraPos);
		m_zAxisVec = GetAxisHeadingToCamera(globalTransform.GetKBasis3D(), m_xyzAxisPos, worldCameraPos);
	}
}

/*
void FBXJointTranslatorGizmo::SetIsAlignedToGlobalAxis(bool isAlignedToGlobalAxis)
{
	m_isAlignedToGlobalAxis = isAlignedToGlobalAxis;
}
*/

void FBXJointTranslatorGizmo::ToggleIsAlignedToGlobalAxis()
{
	m_isAlignedToGlobalAxis = !m_isAlignedToGlobalAxis;
}

Vec3 FBXJointTranslatorGizmo::GetAxisHeadingToCamera(const Vec3& possibleAxisDir, const Vec3& axisPos, const Vec3& playerPos)
{
	Vec3 fromAxisToPlayer = playerPos - axisPos;
	if (DotProduct3D(possibleAxisDir, fromAxisToPlayer) >= 0.0f) {
		return possibleAxisDir;
	}
	else {
		return -possibleAxisDir;
	}
}