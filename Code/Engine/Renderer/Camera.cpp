#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void Camera::SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight, float near, float far) {
	m_cameraAABB2.m_mins = bottomLeft;
	m_cameraAABB2.m_maxs = topRight;
	m_preCameraShakeAABB2.m_mins = bottomLeft;
	m_preCameraShakeAABB2.m_maxs = topRight;
	m_orthographicNear = near;
	m_orthographicFar = far;
}

void Camera::SetOrthoView(const AABB2& cameraBounds, float near, float far)
{
	m_cameraAABB2 = cameraBounds;
	m_preCameraShakeAABB2 = cameraBounds;
	m_orthographicNear = near;
	m_orthographicFar = far;
}

void Camera::SetPerspectiveView(float aspect, float fov, float near, float far)
{
	m_perspectiveAspect = aspect;
	m_perspectiveFOV = fov;
	m_perspectiveNear = near;
	m_perspectiveFar = far;
}

void Camera::SetCameraMode(CameraMode mode)
{
	m_mode = mode;
}

void Camera::SetViewportAABB2(const AABB2& viewportAABB2)
{
	m_viewportAABB2 = viewportAABB2;
}

Vec2 Camera::GetOrthoBottomLeft() const {
	return m_cameraAABB2.m_mins;
}
Vec2 Camera::GetOrthoTopRight() const {
	return m_cameraAABB2.m_maxs;
}

AABB2 Camera::GetCameraAABB2() const
{
	return m_cameraAABB2;
}

void Camera::Translate2D(const Vec2& translation2D)
{
	m_cameraAABB2.m_mins = m_preCameraShakeAABB2.m_mins + translation2D;
	m_cameraAABB2.m_maxs = m_preCameraShakeAABB2.m_maxs + translation2D;
}

Mat44 Camera::GetOrthographicMatrix() const
{
	return Mat44::CreateOrthoProjection(m_cameraAABB2.m_mins.x, m_cameraAABB2.m_maxs.x, m_cameraAABB2.m_mins.y, m_cameraAABB2.m_maxs.y, m_orthographicNear, m_orthographicFar);
}

Mat44 Camera::GetPerspectiveMatrix() const
{
	return Mat44::CreatePerspectiveProjection(m_perspectiveFOV, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar);
}

Mat44 Camera::GetProjectionMatrix() const
{
	if (m_mode == CameraMode::ORTHO) {
		Mat44 projectionMatrix = GetOrthographicMatrix();
		projectionMatrix.Append(GetRenderMatrix());
		return projectionMatrix;
	}
	else if (m_mode == CameraMode::PERSPECTIVE) {
		Mat44 projectionMatrix = GetPerspectiveMatrix();
		projectionMatrix.Append(GetRenderMatrix());
		return projectionMatrix;
	}
	else {
		ERROR_AND_DIE("Camera mode is not set correctly");
	}
}

void Camera::SetRenderBasis(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis)
{
	m_renderIBasis = iBasis;
	m_renderJBasis = jBasis;
	m_renderKBasis = kBasis;
}

Mat44 Camera::GetRenderMatrix() const
{
	Mat44 renderMatrix;
	renderMatrix.SetIJK3D(m_renderIBasis, m_renderJBasis, m_renderKBasis);
	return renderMatrix;
}

void Camera::SetTransform(const Vec3& position, const EulerAngles& orientation)
{
	m_position = position;
	m_orientation = orientation;
}

//return R^-1 * T^-1
Mat44 Camera::GetViewMatrix() const
{
	Mat44 viewMatrix = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	viewMatrix.Transpose();
	viewMatrix.AppendTranslation3D(-m_position);
	return viewMatrix;
}

Vec3 Camera::GetPosition() const
{
	return m_position;
}

EulerAngles Camera::GetOrientation() const
{
	return m_orientation;
}

void Camera::SetOrientation(const EulerAngles& orientation)
{
	m_orientation = orientation;
}

CameraMode Camera::GetCameraMode() const
{
	return m_mode;
}

void Camera::GoBackToOriginalFrame() {
	m_cameraAABB2 = m_preCameraShakeAABB2;
}

void Camera::UpdatePreCameraShakeAABB2()
{
	m_preCameraShakeAABB2 = m_cameraAABB2;
}

AABB2 Camera::GetViewportAABB2() const
{
	return m_viewportAABB2;
}

float Camera::GetPerspectiveAspect() const
{
	return m_perspectiveAspect;
}

float Camera::GetPerspectiveFOV() const
{
	return m_perspectiveFOV;
}

float Camera::GetPerspectiveNear() const
{
	return m_perspectiveNear;
}

float Camera::GetPerspectiveFar() const
{
	return m_perspectiveFar;
}
