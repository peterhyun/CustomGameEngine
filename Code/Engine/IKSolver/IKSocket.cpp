#include "Engine/IKSolver/IKSocket.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/FBX/FBXJoint.hpp"

//Including directx 11 header files and such
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <dxgidebug.h>

#if defined(OPAQUE)
#undef OPAQUE
#endif

IKSocket::IKSocket(FBXJoint& jointToAttachTo, float renderRadius)
	: m_jointToAttachTo(jointToAttachTo), m_renderRadius(renderRadius)
{
}

IKSocket::~IKSocket()
{
	delete m_vbo;
	m_vbo = nullptr;
	
	delete m_ibo;
	m_ibo = nullptr;

	DX_SAFE_RELEASE(m_depthStencilTexture);
	DX_SAFE_RELEASE(m_depthStencilView);
}

void IKSocket::SetGPUData(Renderer& rendererToUse)
{
	GUARANTEE_OR_DIE(m_vbo == nullptr && m_ibo == nullptr, "Calling this IKSocket::SetGPUData() twice!");
	AddVertsForSphere3D(m_vertices, m_indices, Vec3(), m_renderRadius, 16, 8, Rgba8::GREY, AABB2::ZERO_TO_ONE);
	m_vbo = rendererToUse.CreateVertexBuffer(m_vertices.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), "VBO for IKSocket");
	m_ibo = rendererToUse.CreateIndexBuffer(m_indices.size() * sizeof(unsigned int));

	rendererToUse.CopyCPUToGPU(m_vertices.data(), m_vertices.size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), m_vbo);
	rendererToUse.CopyCPUToGPU(m_indices.data(), m_indices.size() * sizeof(unsigned int), m_ibo);
	rendererToUse.CreateDepthStencilTextureAndView(m_depthStencilTexture, m_depthStencilView);
}

void IKSocket::Render(Renderer& rendererToUse) const
{
	GUARANTEE_OR_DIE(m_vbo != nullptr && m_ibo != nullptr, "Didn't call IKSocket::SetGPUData() before IKSocket::Render()!");

	rendererToUse.SetDepthStencilView(m_depthStencilView);
	rendererToUse.ClearDepthStencilView();

	Mat44 globalTransform = GetGlobalTransform();
	rendererToUse.SetModelConstants(globalTransform);
	rendererToUse.SetBlendMode(BlendMode::OPAQUE);
	rendererToUse.SetRasterizerMode(RasterizerMode::WIREFRAME_CULL_NONE);
	rendererToUse.SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_DISABLED);
	rendererToUse.DrawVertexAndIndexBuffer(m_vbo, m_ibo, (int)m_indices.size());

	rendererToUse.SetDepthStencilView(nullptr);
}

Mat44 IKSocket::GetGlobalTransform() const
{
	if (m_isMovingWithJoints) {
		Mat44 globalTransform = m_jointToAttachTo.GetGlobalTransformForThisFrame();
		return globalTransform;
	}
	else {
		Mat44 globalTransform = m_globalTransformBeforeLocalDeltaTransform;
		if (m_isTranslationModified) {
			globalTransform.Append(m_localDeltaRotationFromRotatorGizmo.GetRotationMatrix());
		}
		if (m_isRotationModified) {
			globalTransform.Append(m_localDeltaRotationFromRotatorGizmo.GetRotationMatrix());
		}
		return globalTransform;
	}
}

FBXJoint& IKSocket::GetAttachedJoint() const
{
	return m_jointToAttachTo;
}

void IKSocket::SetIsMovingWithJoints(bool isMovingWithJoints, const Mat44* newGlobalTransform)
{
	m_isMovingWithJoints = isMovingWithJoints;
	if (isMovingWithJoints == false) {
		if (newGlobalTransform == nullptr) {
			m_globalTransformBeforeLocalDeltaTransform = m_jointToAttachTo.GetGlobalTransformForThisFrame();
			m_localDeltaTranslationFromTranslatorGizmo = Vec3();
			m_localDeltaRotationFromRotatorGizmo = Quaternion();
			m_isRotationModified = false;
			m_isTranslationModified = false;
		}
		else {
			m_globalTransformBeforeLocalDeltaTransform = *newGlobalTransform;
			m_localDeltaTranslationFromTranslatorGizmo = Vec3();
			m_localDeltaRotationFromRotatorGizmo = Quaternion();
			m_isRotationModified = false;
			m_isTranslationModified = false;
		}
	}
}

bool IKSocket::IsMovingWithJoints() const
{
	return m_isMovingWithJoints;
}

void IKSocket::SetGlobalPos(const Vec3& newGlobalPos)
{
	m_globalTransformBeforeLocalDeltaTransform.SetTranslation3D(newGlobalPos);
	m_isMovingWithJoints = false;
}

void IKSocket::SetGlobalOri(const Mat44& newGlobalOri)
{
	m_globalTransformBeforeLocalDeltaTransform.CopyRotation(newGlobalOri);
	m_isMovingWithJoints = false;
}

Quaternion IKSocket::GetLocalDeltaRotate() const
{
	return m_localDeltaRotationFromRotatorGizmo;
}

void IKSocket::SetLocalDeltaRotate(const Quaternion& localDeltaRotate)
{
	m_isRotationModified = true;
	m_localDeltaRotationFromRotatorGizmo = localDeltaRotate;
}

Vec3 IKSocket::GetLocalDeltaTranslate() const
{
	return m_localDeltaTranslationFromTranslatorGizmo;
}

void IKSocket::SetLocalDeltaTranslate (const Vec3& localDeltaTranslate)
{
	m_isTranslationModified = true;
	m_localDeltaTranslationFromTranslatorGizmo = localDeltaTranslate;
}

void IKSocket::AddLocalXRotation(float degrees)
{
	m_isRotationModified = true;
	m_localDeltaRotationFromRotatorGizmo = m_localDeltaRotationFromRotatorGizmo * Quaternion::CreateFromAxisAndDegrees(degrees, Vec3(1.0f, 0.0f, 0.0f));
}

void IKSocket::AddLocalYRotation(float degrees)
{
	m_isRotationModified = true;
	m_localDeltaRotationFromRotatorGizmo = m_localDeltaRotationFromRotatorGizmo * Quaternion::CreateFromAxisAndDegrees(degrees, Vec3(0.0f, 1.0f, 0.0f));
}

void IKSocket::AddLocalZRotation(float degrees)
{
	m_isRotationModified = true;
	m_localDeltaRotationFromRotatorGizmo = m_localDeltaRotationFromRotatorGizmo * Quaternion::CreateFromAxisAndDegrees(degrees, Vec3(0.0f, 0.0f, 1.0f));
}
