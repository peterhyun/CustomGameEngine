#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Quaternion.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <vector>
#include <string>

class Shader;
class Renderer;
class VertexBuffer;
class IndexBuffer;
class FBXJoint;
class FBXModel;
class Camera;
class FBXJointGizmosManager;

enum class FBXJointRotatorGizmoState {
	XPressed = 0,
	YPressed,
	ZPressed,
	NONE,
	NUMSTATES
};

class FBXJointRotatorGizmo {
	friend class FBXJointGizmosManager;

private:
	FBXJointRotatorGizmo(FBXJointGizmosManager& manager, FBXModel& model, Renderer& renderer);
	~FBXJointRotatorGizmo();

	void Startup();
	void Update(const Camera& worldCamera);
	void Render() const;

	void SetGPUData();
	void UpdateFromKeyboard();
	void UpdateFromMouseMovement(const Camera& worldCamera);
	void UpdateJointClientPosVariable(const Camera& worldCamera);

private:
	FBXJointGizmosManager& m_manager;
	FBXModel& m_model;

	//Render Data
	VertexBuffer* m_vboTrackball = nullptr;
	std::vector<Vertex_PCU> m_verticesTrackball;

	VertexBuffer* m_vboX = nullptr;
	IndexBuffer* m_iboX = nullptr;
	std::vector<Vertex_PCU> m_verticesX;
	std::vector<unsigned int> m_indicesX;
	unsigned int m_stencilValX = 0;

	VertexBuffer* m_vboY = nullptr;
	IndexBuffer* m_iboY = nullptr;
	std::vector<Vertex_PCU> m_verticesY;
	std::vector<unsigned int> m_indicesY;
	unsigned int m_stencilValY = 0;

	VertexBuffer* m_vboZ = nullptr;
	IndexBuffer* m_iboZ = nullptr;
	std::vector<Vertex_PCU> m_verticesZ;
	std::vector<unsigned int> m_indicesZ;
	unsigned int m_stencilValZ = 0;

	//Render Data
	VertexBuffer* m_vboXSelected = nullptr;
	IndexBuffer* m_iboXSelected = nullptr;
	std::vector<Vertex_PCU> m_verticesXSelected;
	std::vector<unsigned int> m_indicesXSelected;

	VertexBuffer* m_vboYSelected = nullptr;
	IndexBuffer* m_iboYSelected = nullptr;
	std::vector<Vertex_PCU> m_verticesYSelected;
	std::vector<unsigned int> m_indicesYSelected;

	VertexBuffer* m_vboZSelected = nullptr;
	IndexBuffer* m_iboZSelected = nullptr;
	std::vector<Vertex_PCU> m_verticesZSelected;
	std::vector<unsigned int> m_indicesZSelected;

	FBXJointRotatorGizmoState m_state = FBXJointRotatorGizmoState::NONE;
	Renderer& m_renderer;

	IntVec2 m_cursorPosTrackingStarted;
	IntVec2 m_cursorPosPreviousFrame;
	const int m_mouseMoveThreshold = 10;
	Vec3 m_gizmoToWorldCamera;

	struct ID3D11Texture2D* m_depthStencilTexture = nullptr;
	struct ID3D11DepthStencilView* m_depthStencilView = nullptr;

	IntVec2 m_chosenJointClientPos;
	Quaternion m_chosenJointInitialLocalDeltaRotation;

	IntVec2 m_chosenJointIKSocketClientPos;
	Quaternion m_chosenJointIKSocketInitialLocalDeltaRotation;

	bool m_isLMBBeingUsed = false;
};