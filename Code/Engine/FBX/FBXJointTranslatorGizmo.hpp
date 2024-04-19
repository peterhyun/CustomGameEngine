#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Plane3D.hpp"
#include "Engine/Math/InfiniteLine3.hpp"
#include <vector>
#include <string>

class Shader;
class Renderer;
class VertexBuffer;
class FBXJoint;
class FBXModel;
class Camera;
class FBXJointGizmosManager;

enum class FBXJointTranslatorGizmoState {
	XPressed = 0,
	YPressed,
	ZPressed,
	CenterPressed,
	NONE,
	NUMSTATES
};

class FBXJointTranslatorGizmo {
	friend class FBXJointGizmosManager;

private:
	FBXJointTranslatorGizmo(FBXJointGizmosManager& manager, FBXModel& model, Renderer& renderer);
	~FBXJointTranslatorGizmo();

	void Startup();
	void Update(const Camera& worldCamera);
	void Render() const;

	void SetGPUData();
	void UpdateFromKeyboard(const Camera& worldCamera);
	void UpdateFromMouseMovement(const Camera& worldCamera);
	//void UpdateJointClientPosVariable(const Camera& worldCamera);

	bool DoesSpawnedPlaneCoverAllCornersOfWindow(const Vec3& fixedAxis, const Vec3& candidateAxis, const Vec3& gizmoPos, const Camera& worldCamera, int& numCoveredCorners);
	void UpdateGlobalTransform(const Camera& worldCamera);
	//void SetIsAlignedToGlobalAxis(bool isAlignedToGlobalAxis);
	void ToggleIsAlignedToGlobalAxis();
	Vec3 GetAxisHeadingToCamera(const Vec3& possibleAxisDir, const Vec3& axisPos, const Vec3& playerPos);

private:
	Mat44 m_localDeltaTransform;
	bool m_isLMBBeingUsed = false;

	FBXJointGizmosManager& m_manager;
	FBXModel& m_model;

	FBXJointTranslatorGizmoState m_state = FBXJointTranslatorGizmoState::NONE;
	Renderer& m_renderer;

	//Needed if you want to implement snapping. But not now I guess
	const int m_mouseMoveThreshold = 0;
	IntVec2 m_cursorPosPreviousFrame;

	//Variables that are filled with data when LMB clicked
	Plane3D m_virtualPlane;
	InfiniteLine3 m_axisLineOnVirtualPlane;
	//Vec3 m_pointOnVirtualPlaneProjectedOntoAxisOnInitialClick;
	Vec3 m_offsetFromProjectedInitialClickToAxisPos;
	Vec3 m_offsetFromInitialClickToCenterPos;

	float m_sphereRadius = 0.5f;
	const float m_arrowRadius = 0.2f;

	//Render Data
	VertexBuffer* m_vboX = nullptr;
	std::vector<Vertex_PCU> m_verticesX;
	unsigned int m_stencilValX = 0;
	VertexBuffer* m_vboY = nullptr;
	std::vector<Vertex_PCU> m_verticesY;
	unsigned int m_stencilValY = 0;
	VertexBuffer* m_vboZ = nullptr;
	std::vector<Vertex_PCU> m_verticesZ;
	unsigned int m_stencilValZ = 0;
	const float m_arrowLength = 10.0f;
	const unsigned int m_numSlicesArrow = 32;
	VertexBuffer* m_vboXSelected = nullptr;
	std::vector<Vertex_PCU> m_verticesXSelected;
	VertexBuffer* m_vboYSelected = nullptr;
	std::vector<Vertex_PCU> m_verticesYSelected;
	VertexBuffer* m_vboZSelected = nullptr;
	std::vector<Vertex_PCU> m_verticesZSelected;
	struct ID3D11Texture2D* m_depthStencilTexture = nullptr;
	struct ID3D11DepthStencilView* m_depthStencilView = nullptr;

	VertexBuffer* m_vboSphere = nullptr;
	std::vector<Vertex_PCU> m_verticesSphere;
	unsigned int m_stencilValSphere = 0;
	VertexBuffer* m_vboSphereSelected = nullptr;
	std::vector<Vertex_PCU> m_verticesSphereSelected;

	bool m_isAlignedToGlobalAxis = false;

	Vec3 m_xAxisVec;
	Vec3 m_yAxisVec;
	Vec3 m_zAxisVec;
	Vec3 m_xyzAxisPos;

	Vec3 m_latestProjectionPoint;
};