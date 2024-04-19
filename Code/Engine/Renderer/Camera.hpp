#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/EulerAngles.hpp"

enum class CameraMode
{
	INVALID = -1,
	ORTHO,
	PERSPECTIVE,
	NUM_CAMERAMODE
};

class Camera {
public:
	void SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight, float near = 0.0f, float far = 1.0f);
	void SetOrthoView(const AABB2& cameraBounds, float near = 0.0f, float far = 1.0f);
	void SetPerspectiveView(float aspect, float fov, float near, float far);
	void SetCameraMode(CameraMode mode);
	void SetViewportAABB2(const AABB2& viewportAABB2);

	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;
	AABB2 GetCameraAABB2() const;
	void Translate2D(const Vec2& translation2D);

	Mat44 GetOrthographicMatrix() const;
	Mat44 GetPerspectiveMatrix() const;
	Mat44 GetProjectionMatrix() const;

	void SetRenderBasis(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis);
	Mat44 GetRenderMatrix() const;

	void SetTransform(const Vec3& position, const EulerAngles& orientation);
	Mat44 GetViewMatrix() const;

	Vec3 GetPosition() const;
	EulerAngles GetOrientation() const;
	void SetOrientation(const EulerAngles& orientation);

	CameraMode GetCameraMode() const;

	//For camera shake (assuming orthographic camera view)
	void GoBackToOriginalFrame();
	void UpdatePreCameraShakeAABB2();

	//For viewport setup
	AABB2 GetViewportAABB2() const;

	float GetPerspectiveAspect() const;
	float GetPerspectiveFOV() const;
	float GetPerspectiveNear() const;
	float GetPerspectiveFar() const;

private:
	CameraMode m_mode = CameraMode::ORTHO;

	float m_orthographicNear = 0.0f;
	float m_orthographicFar = 0.0f;

	float m_perspectiveAspect = 0.0f;
	float m_perspectiveFOV = 0.0f;
	float m_perspectiveNear = 0.0f;
	float m_perspectiveFar = 0.0f;

	Vec3 m_renderIBasis = Vec3(1.0f, 0.0f, 0.0f);
	Vec3 m_renderJBasis = Vec3(0.0f, 1.0f, 0.0f);
	Vec3 m_renderKBasis = Vec3(0.0f, 0.0f, 1.0f);

	Vec3 m_position;
	EulerAngles m_orientation;

	AABB2 m_preCameraShakeAABB2;

	//This is for camera shake
	AABB2 m_cameraAABB2;

	AABB2 m_viewportAABB2;
};