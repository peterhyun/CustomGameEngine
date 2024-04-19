#include "Engine/Fbx/FBXJointRotatorGizmo.hpp"
#include "Engine/FBx/FBXJointGizmosManager.hpp"
#include "Engine/Fbx/FBxJoint.hpp"
#include "Engine/Fbx/FBXModel.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Window/Window.hpp"

//Including directx 11 header files and such
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

#if defined(OPAQUE)
#undef OPAQUE
#endif

FBXJointRotatorGizmo::FBXJointRotatorGizmo(FBXJointGizmosManager& manager, FBXModel& model, Renderer& renderer) : m_manager(manager), m_model(model), m_renderer(renderer)
{
}

FBXJointRotatorGizmo::~FBXJointRotatorGizmo()
{
	delete m_iboX;
	m_iboX = nullptr;

	delete m_iboY;
	m_iboY = nullptr;

	delete m_iboZ;
	m_iboZ = nullptr;

	delete m_vboX;
	m_vboX = nullptr;

	delete m_vboY;
	m_vboY = nullptr;

	delete m_vboZ;
	m_vboZ = nullptr;

	delete m_vboTrackball;
	m_vboTrackball = nullptr;

	delete m_vboXSelected;
	m_vboXSelected = nullptr;

	delete m_vboYSelected;
	m_vboYSelected = nullptr;

	delete m_vboZSelected;
	m_vboZSelected = nullptr;

	delete m_iboXSelected;
	m_iboXSelected = nullptr;

	delete m_iboYSelected;
	m_iboYSelected = nullptr;

	delete m_iboZSelected;
	m_iboZSelected = nullptr;

	DX_SAFE_RELEASE(m_depthStencilTexture);
	DX_SAFE_RELEASE(m_depthStencilView);
}

void FBXJointRotatorGizmo::Startup()
{
	unsigned int availableStencilValue = m_model.GetMaxJointStencilValue() + 1;
	m_stencilValX = availableStencilValue;
	m_stencilValY = availableStencilValue + 1;
	m_stencilValZ = availableStencilValue + 2;
	SetGPUData();
}

void FBXJointRotatorGizmo::SetGPUData()
{
	m_renderer.CreateDepthStencilTextureAndView(m_depthStencilTexture, m_depthStencilView);
	/*
	AddVertsForRing3D(m_verticesX, 24, Vec3(0.0f, 0.0f, 0.0f), 4.0f, 4.5f, Vec3(1.0f, 0.0f, 0.0f), Rgba8::RED);
	AddVertsForRing3D(m_verticesY, 24, Vec3(0.0f, 0.0f, 0.0f), 4.0f, 4.5f, Vec3(0.0f, 1.0f, 0.0f), Rgba8::GREEN);
	AddVertsForRing3D(m_verticesZ, 24, Vec3(0.0f, 0.0f, 0.0f), 4.0f, 4.5f, Vec3(0.0f, 0.0f, 1.0f), Rgba8::BLUE);
	*/
	AddVertsForTorus3D(m_verticesX, m_indicesX, 30, 40, 0.12f, 4.1f, Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Rgba8::RED);
	AddVertsForTorus3D(m_verticesY, m_indicesY, 30, 40, 0.12f, 4.1f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Rgba8::GREEN);
	AddVertsForTorus3D(m_verticesZ, m_indicesZ, 30, 40, 0.12f, 4.1f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Rgba8::BLUE);
	AddVertsForSphere3D(m_verticesTrackball, Vec3(0.0f, 0.0f, 0.0f), 3.99f, Rgba8(255, 255, 255, 50), AABB2::ZERO_TO_ONE, 24);
	
	m_vboX = m_renderer.CreateVertexBuffer(m_verticesX.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboX in FBXJointRotatorGizmo");
	m_vboY = m_renderer.CreateVertexBuffer(m_verticesY.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboY in FBXJointRotatorGizmo");
	m_vboZ = m_renderer.CreateVertexBuffer(m_verticesZ.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboZ in FBXJointRotatorGizmo");
	m_iboX = m_renderer.CreateIndexBuffer(m_indicesX.size() * sizeof(unsigned int));
	m_iboY = m_renderer.CreateIndexBuffer(m_indicesY.size() * sizeof(unsigned int));
	m_iboZ = m_renderer.CreateIndexBuffer(m_indicesZ.size() * sizeof(unsigned int));

	m_vboTrackball = m_renderer.CreateVertexBuffer(m_verticesTrackball.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboTrackball in FBXJointRotatorGizmo");

	m_renderer.CopyCPUToGPU(m_verticesX.data(), m_verticesX.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboX);
	m_renderer.CopyCPUToGPU(m_verticesY.data(), m_verticesY.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboY);
	m_renderer.CopyCPUToGPU(m_verticesZ.data(), m_verticesZ.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboZ);
	m_renderer.CopyCPUToGPU(m_indicesX.data(), m_indicesX.size() * sizeof(unsigned int), m_iboX);
	m_renderer.CopyCPUToGPU(m_indicesY.data(), m_indicesY.size() * sizeof(unsigned int), m_iboY);
	m_renderer.CopyCPUToGPU(m_indicesZ.data(), m_indicesZ.size() * sizeof(unsigned int), m_iboZ);

	m_renderer.CopyCPUToGPU(m_verticesTrackball.data(), m_verticesTrackball.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboTrackball);

	AddVertsForTorus3D(m_verticesXSelected, m_indicesXSelected, 30, 40, 0.12f, 4.1f, Vec3(0.0f, 0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Rgba8::YELLOW);
	AddVertsForTorus3D(m_verticesYSelected, m_indicesYSelected, 30, 40, 0.12f, 4.1f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Rgba8::YELLOW);
	AddVertsForTorus3D(m_verticesZSelected, m_indicesZSelected, 30, 40, 0.12f, 4.1f, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Rgba8::YELLOW);

	m_vboXSelected = m_renderer.CreateVertexBuffer(m_verticesXSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboXSelected in FBXJointRotatorGizmo");
	m_vboYSelected = m_renderer.CreateVertexBuffer(m_verticesYSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboYSelected in FBXJointRotatorGizmo");
	m_vboZSelected = m_renderer.CreateVertexBuffer(m_verticesZSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "m_vboZSelected in FBXJointRotatorGizmo");
	m_iboXSelected = m_renderer.CreateIndexBuffer(m_indicesXSelected.size() * sizeof(unsigned int));
	m_iboYSelected = m_renderer.CreateIndexBuffer(m_indicesYSelected.size() * sizeof(unsigned int));
	m_iboZSelected = m_renderer.CreateIndexBuffer(m_indicesZSelected.size() * sizeof(unsigned int));

	m_renderer.CopyCPUToGPU(m_verticesXSelected.data(), m_verticesXSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboXSelected);
	m_renderer.CopyCPUToGPU(m_verticesYSelected.data(), m_verticesYSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboYSelected);
	m_renderer.CopyCPUToGPU(m_verticesZSelected.data(), m_verticesZSelected.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vboZSelected);
	m_renderer.CopyCPUToGPU(m_indicesXSelected.data(), m_indicesXSelected.size() * sizeof(unsigned int), m_iboXSelected);
	m_renderer.CopyCPUToGPU(m_indicesYSelected.data(), m_indicesYSelected.size() * sizeof(unsigned int), m_iboYSelected);
	m_renderer.CopyCPUToGPU(m_indicesZSelected.data(), m_indicesZSelected.size() * sizeof(unsigned int), m_iboZSelected);
}

void FBXJointRotatorGizmo::UpdateFromKeyboard()
{
	m_isLMBBeingUsed = false;
	if (g_theInput->IsCursorHidden())
		return;

	if (m_model.GetJointVisibility() == false) {
		return;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LMB)) {
		IntVec2 cursorClientPos = g_theInput->GetCursorClientPos();

		uint8_t selectedStencilValueFromRotatorGizmoTexture = m_renderer.GetStencilValueOfClientPos(cursorClientPos, m_depthStencilTexture);

		FBXJoint* selectedJoint = m_manager.GetSelectedJoint();
		if (selectedJoint == nullptr)
			return;

		if (m_manager.IsIKSocketMode()) {
			IKSocket& socket = selectedJoint->GetRefToIKSocket();
			if (selectedStencilValueFromRotatorGizmoTexture == m_stencilValX) {
				if (socket.IsMovingWithJoints()) {
					socket.SetIsMovingWithJoints(false, nullptr);
				}
				m_state = FBXJointRotatorGizmoState::XPressed;
				m_cursorPosTrackingStarted = g_theInput->GetCursorClientPos();
				m_cursorPosPreviousFrame = m_cursorPosTrackingStarted;
				m_chosenJointIKSocketInitialLocalDeltaRotation = socket.GetLocalDeltaRotate();
				m_isLMBBeingUsed = true;
			}
			else if (selectedStencilValueFromRotatorGizmoTexture == m_stencilValY) {
				if (socket.IsMovingWithJoints()) {
					socket.SetIsMovingWithJoints(false, nullptr);
				}
				m_state = FBXJointRotatorGizmoState::YPressed;
				m_cursorPosTrackingStarted = g_theInput->GetCursorClientPos();
				m_cursorPosPreviousFrame = m_cursorPosTrackingStarted;
				m_chosenJointIKSocketInitialLocalDeltaRotation = socket.GetLocalDeltaRotate();
				m_isLMBBeingUsed = true;
			}
			else if (selectedStencilValueFromRotatorGizmoTexture == m_stencilValZ) {
				if (socket.IsMovingWithJoints()) {
					socket.SetIsMovingWithJoints(false, nullptr);
				}
				m_state = FBXJointRotatorGizmoState::ZPressed;
				m_cursorPosTrackingStarted = g_theInput->GetCursorClientPos();
				m_cursorPosPreviousFrame = m_cursorPosTrackingStarted;
				m_chosenJointIKSocketInitialLocalDeltaRotation = socket.GetLocalDeltaRotate();
				m_isLMBBeingUsed = true;
			}
		}
		else {
			if (selectedStencilValueFromRotatorGizmoTexture == m_stencilValX) {
				m_state = FBXJointRotatorGizmoState::XPressed;
				m_cursorPosTrackingStarted = g_theInput->GetCursorClientPos();
				m_cursorPosPreviousFrame = m_cursorPosTrackingStarted;
				m_chosenJointInitialLocalDeltaRotation = selectedJoint->GetLocalDeltaRotate();
				m_isLMBBeingUsed = true;
			}
			else if (selectedStencilValueFromRotatorGizmoTexture == m_stencilValY) {
				m_state = FBXJointRotatorGizmoState::YPressed;
				m_cursorPosTrackingStarted = g_theInput->GetCursorClientPos();
				m_cursorPosPreviousFrame = m_cursorPosTrackingStarted;
				m_chosenJointInitialLocalDeltaRotation = selectedJoint->GetLocalDeltaRotate();
				m_isLMBBeingUsed = true;
			}
			else if (selectedStencilValueFromRotatorGizmoTexture == m_stencilValZ) {
				m_state = FBXJointRotatorGizmoState::ZPressed;
				m_cursorPosTrackingStarted = g_theInput->GetCursorClientPos();
				m_cursorPosPreviousFrame = m_cursorPosTrackingStarted;
				m_chosenJointInitialLocalDeltaRotation = selectedJoint->GetLocalDeltaRotate();
				m_isLMBBeingUsed = true;
			}
		}
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_LMB)) {
		if (m_state == FBXJointRotatorGizmoState::XPressed || m_state == FBXJointRotatorGizmoState::YPressed || m_state == FBXJointRotatorGizmoState::ZPressed) {
			m_state = FBXJointRotatorGizmoState::NONE;
		}
	}
}

void FBXJointRotatorGizmo::UpdateFromMouseMovement(const Camera& worldCamera)
{
	FBXJoint* selectedJoint = m_manager.GetSelectedJoint();
	if (selectedJoint == nullptr)
		return;

	if (m_state == FBXJointRotatorGizmoState::XPressed || m_state == FBXJointRotatorGizmoState::YPressed || m_state == FBXJointRotatorGizmoState::ZPressed) {
		IntVec2 currentClientPos = g_theInput->GetCursorClientPos();
		IntVec2 deltaClientPos = currentClientPos - m_cursorPosPreviousFrame;
		if (GetAbs(deltaClientPos.x) < m_mouseMoveThreshold && GetAbs(deltaClientPos.y) < m_mouseMoveThreshold) {
			return;
		}
		else {
			m_cursorPosPreviousFrame = currentClientPos;
		}

		Vec2 gizmoToCurrentPos;
		Vec2 gizmoToTrackingStartedPos;
		if (m_manager.IsIKSocketMode()) {
			gizmoToCurrentPos = Vec2(currentClientPos - m_chosenJointIKSocketClientPos);
			gizmoToTrackingStartedPos = Vec2(m_cursorPosTrackingStarted - m_chosenJointIKSocketClientPos);
		}
		else {
			gizmoToCurrentPos = Vec2(currentClientPos - m_chosenJointClientPos);
			gizmoToTrackingStartedPos = Vec2(m_cursorPosTrackingStarted - m_chosenJointClientPos);
		}
		Vec3 crossProductResult = CrossProduct3D(Vec3(gizmoToCurrentPos, 0.0f), Vec3(gizmoToTrackingStartedPos, 0.0f));
		
		float angleDegrees = GetAngleDegreesBetweenVectors2D(gizmoToCurrentPos, gizmoToTrackingStartedPos);
		float signOfAngle = GetSign(DotProduct3D(crossProductResult, Vec3(0.0f, 0.0f, 1.0f)));

		if (m_manager.IsIKSocketMode()){
			IKSocket& socket = selectedJoint->GetRefToIKSocket();
			const Mat44 socketTransform = socket.GetGlobalTransform();
			const Vec3 socketXAxis = socketTransform.GetIBasis3D();
			const Vec3 socketYAxis = socketTransform.GetJBasis3D();
			const Vec3 socketZAxis = socketTransform.GetKBasis3D();
			const Vec3 socketWorldLocation = socketTransform.GetTranslation3D();
			const Vec3 gizmoToWorldCamera = worldCamera.GetPosition() - socketWorldLocation;

			if (m_state == FBXJointRotatorGizmoState::XPressed) {
				float signOfAxis = GetSign(DotProduct3D(socketXAxis, gizmoToWorldCamera));
				//DebugAddMessage(Stringf("signAxis: %.1f", signOfAxis), debugTime);
				socket.SetLocalDeltaRotate(m_chosenJointIKSocketInitialLocalDeltaRotation);
				socket.AddLocalXRotation(signOfAxis * signOfAngle * angleDegrees);
				m_model.SetDDMNeedsRecalculation();
			}
			else if (m_state == FBXJointRotatorGizmoState::YPressed) {
				float signOfAxis = GetSign(DotProduct3D(socketYAxis, gizmoToWorldCamera));
				//DebugAddMessage(Stringf("signAxis: %.1f", signOfAxis), debugTime);
				socket.SetLocalDeltaRotate(m_chosenJointIKSocketInitialLocalDeltaRotation);
				socket.AddLocalYRotation(signOfAxis * signOfAngle * angleDegrees);
				m_model.SetDDMNeedsRecalculation();
			}
			else if (m_state == FBXJointRotatorGizmoState::ZPressed) {
				float signOfAxis = GetSign(DotProduct3D(socketZAxis, gizmoToWorldCamera));
				//DebugAddMessage(Stringf("signAxis: %.1f", signOfAxis), debugTime);
				socket.SetLocalDeltaRotate(m_chosenJointIKSocketInitialLocalDeltaRotation);
				socket.AddLocalZRotation(signOfAxis * signOfAngle * angleDegrees);
				m_model.SetDDMNeedsRecalculation();
			}
		}
		else {
			const Mat44 jointTransform = selectedJoint->GetGlobalTransformForThisFrame();
			const Vec3 jointXAxis = jointTransform.GetIBasis3D();
			const Vec3 jointYAxis = jointTransform.GetJBasis3D();
			const Vec3 jointZAxis = jointTransform.GetKBasis3D();
			const Vec3 jointWorldLocation = jointTransform.GetTranslation3D();

			const Vec3 gizmoToWorldCamera = worldCamera.GetPosition() - jointWorldLocation;
			//float debugTime = 2.0f;
			if (m_state == FBXJointRotatorGizmoState::XPressed) {
				float signOfAxis = GetSign(DotProduct3D(jointXAxis, gizmoToWorldCamera));
				//DebugAddMessage(Stringf("signAxis: %.1f", signOfAxis), debugTime);
				selectedJoint->SetLocalDeltaRotate(m_chosenJointInitialLocalDeltaRotation);
				selectedJoint->AddLocalXRotation(signOfAxis * signOfAngle * angleDegrees);
				m_model.SetDDMNeedsRecalculation();
			}
			else if (m_state == FBXJointRotatorGizmoState::YPressed) {
				float signOfAxis = GetSign(DotProduct3D(jointYAxis, gizmoToWorldCamera));
				//DebugAddMessage(Stringf("signAxis: %.1f", signOfAxis), debugTime);
				selectedJoint->SetLocalDeltaRotate(m_chosenJointInitialLocalDeltaRotation);
				selectedJoint->AddLocalYRotation(signOfAxis * signOfAngle * angleDegrees);
				m_model.SetDDMNeedsRecalculation();
			}
			else if (m_state == FBXJointRotatorGizmoState::ZPressed) {
				float signOfAxis = GetSign(DotProduct3D(jointZAxis, gizmoToWorldCamera));
				//DebugAddMessage(Stringf("signAxis: %.1f", signOfAxis), debugTime);
				selectedJoint->SetLocalDeltaRotate(m_chosenJointInitialLocalDeltaRotation);
				selectedJoint->AddLocalZRotation(signOfAxis * signOfAngle * angleDegrees);
				m_model.SetDDMNeedsRecalculation();
			}
		}
	}
}

void FBXJointRotatorGizmo::UpdateJointClientPosVariable(const Camera& worldCamera)
{
	FBXJoint* selectedJoint = m_manager.GetSelectedJoint();
	if (selectedJoint == nullptr) return;

	Mat44 globalTransformForThisJointForThisFrame;
	if (m_manager.IsIKSocketMode()) {
		IKSocket& socket = selectedJoint->GetRefToIKSocket();
		globalTransformForThisJointForThisFrame = socket.GetGlobalTransform();
	}
	else {
		globalTransformForThisJointForThisFrame = selectedJoint->GetGlobalTransformForThisFrame();
	}
	Mat44 viewMatrix = worldCamera.GetViewMatrix();
	Mat44 projectionMatrix = worldCamera.GetProjectionMatrix();

	Vec3 worldPos = globalTransformForThisJointForThisFrame.GetTranslation3D();
	Vec4 viewSpacePos = viewMatrix.TransformHomogeneous3D(Vec4(worldPos, 1.0f));
	Vec4 clipSpacePos = projectionMatrix.TransformHomogeneous3D(viewSpacePos);
	clipSpacePos /= clipSpacePos.w;

	Window* window = Window::GetWindowContext();
	IntVec2 clientDimensions;
	if (window) {
		clientDimensions = window->GetClientDimensions();
	}
	else {
		ERROR_AND_DIE("Window is not initialized");
	}

	if (m_manager.IsIKSocketMode()) {
		m_chosenJointIKSocketClientPos.x = (int)RangeMap(clipSpacePos.x, -1.0f, 1.0f, 0.0f, (float)clientDimensions.x);
		m_chosenJointIKSocketClientPos.y = (int)RangeMap(clipSpacePos.y, -1.0f, 1.0f, (float)clientDimensions.y, 0.0f);
	}
	else {
		m_chosenJointClientPos.x = (int)RangeMap(clipSpacePos.x, -1.0f, 1.0f, 0.0f, (float)clientDimensions.x);
		m_chosenJointClientPos.y = (int)RangeMap(clipSpacePos.y, -1.0f, 1.0f, (float)clientDimensions.y, 0.0f);
	}
}

void FBXJointRotatorGizmo::Update(const Camera& camera)
{
	FBXJoint* selectedJoint = m_manager.GetSelectedJoint();
	if (selectedJoint) {
		UpdateJointClientPosVariable(camera);
	}
	UpdateFromKeyboard();
	UpdateFromMouseMovement(camera);
}

void FBXJointRotatorGizmo::Render() const
{
	FBXJoint* selectedJoint = m_manager.GetSelectedJoint();
	if (selectedJoint == nullptr)
		return;

	Mat44 globalTransformForThisJointForThisFrame;
	if (m_manager.IsIKSocketMode()) {
		IKSocket& socket = selectedJoint->GetRefToIKSocket();
		socket.Render(m_renderer);
		globalTransformForThisJointForThisFrame = socket.GetGlobalTransform();
	}
	else {
		globalTransformForThisJointForThisFrame = selectedJoint->GetGlobalTransformForThisFrame();
	}
	m_renderer.SetModelConstants(globalTransformForThisJointForThisFrame);
	m_renderer.SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
	if (m_state == FBXJointRotatorGizmoState::XPressed) {
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_DISABLED_STENCIL_ENABLED, 0);
		m_renderer.DrawVertexAndIndexBuffer(m_vboXSelected, m_iboXSelected, (int)m_indicesXSelected.size());
	}
	else if (m_state == FBXJointRotatorGizmoState::YPressed){
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_DISABLED_STENCIL_ENABLED, 0);
		m_renderer.DrawVertexAndIndexBuffer(m_vboYSelected, m_iboYSelected, (int)m_indicesYSelected.size());
	}
	else if (m_state == FBXJointRotatorGizmoState::ZPressed) {
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_DISABLED_STENCIL_ENABLED, 0);
		m_renderer.DrawVertexAndIndexBuffer(m_vboZSelected, m_iboZSelected, (int)m_indicesZSelected.size());
	}
	else {
		m_renderer.SetDepthStencilView(m_depthStencilView);
		m_renderer.ClearDepthStencilView();

		m_renderer.SetBlendMode(BlendMode::ALPHA);
		m_renderer.SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_DISABLED);
		m_renderer.DrawVertexBuffer(m_vboTrackball, (int)m_verticesTrackball.size());

		m_renderer.SetBlendMode(BlendMode::OPAQUE);
		m_renderer.SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_ENABLED, m_stencilValX);
		m_renderer.DrawVertexAndIndexBuffer(m_vboX, m_iboX,(int)m_indicesX.size());
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_ENABLED, m_stencilValY);
		m_renderer.DrawVertexAndIndexBuffer(m_vboY, m_iboY, (int)m_indicesY.size());
		m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_ENABLED, m_stencilValZ);
		m_renderer.DrawVertexAndIndexBuffer(m_vboZ, m_iboZ, (int)m_indicesZ.size());

		m_renderer.SetDepthStencilView(nullptr);	//Have to set the view back to the renderer default depth stencil view
	}
}