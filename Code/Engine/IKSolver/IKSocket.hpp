#pragma once
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Quaternion.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>

class FBXJoint;
class VertexBuffer;
class IndexBuffer;
class Renderer;

class IKSocket {
public:
	IKSocket(FBXJoint& jointToAttachTo, float renderRadius = 1.0f);
	~IKSocket();

	void SetGPUData(Renderer& rendererToUse);

	void Render(Renderer& m_rendererToUse) const;
	Mat44 GetGlobalTransform() const;
	FBXJoint& GetAttachedJoint() const;

	//If isMovingWithJoints == false and newGlobalTransform == nullptr, then the socket will now be fixed at the latest joint global transform
	void SetIsMovingWithJoints(bool isMovingWithJoints, const Mat44* newGlobalTransform = nullptr);
	bool IsMovingWithJoints() const;

	void SetGlobalPos(const Vec3& newGlobalPos);
	void SetGlobalOri(const Mat44& newGlobalOri);

	//Rotator gizmo related functions
	Quaternion GetLocalDeltaRotate() const;
	void SetLocalDeltaRotate(const Quaternion& localDeltaRotate);
	Vec3 GetLocalDeltaTranslate() const;
	void SetLocalDeltaTranslate(const Vec3& localDeltaTranslate);

	void AddLocalXRotation(float degrees);
	void AddLocalYRotation(float degrees);
	void AddLocalZRotation(float degrees);

private:
	FBXJoint& m_jointToAttachTo;

	//Modify this value when you start solving IK. Then we just use global transform at that point
	bool m_isMovingWithJoints = true;
	//If it is modified, it's on its own. Make sure this fill this matrix when modifying. This is because when you start solving IK, this socket has to stay at the same place regardless of what the joints are doing
	Mat44 m_globalTransformBeforeLocalDeltaTransform;

	//Render data
	float m_renderRadius = 1.0f;
	VertexBuffer* m_vbo = nullptr;
	IndexBuffer* m_ibo = nullptr;
	std::vector<Vertex_PCU> m_vertices;
	std::vector<unsigned int> m_indices;

	struct ID3D11Texture2D* m_depthStencilTexture = nullptr;
	struct ID3D11DepthStencilView* m_depthStencilView = nullptr;

	Quaternion m_localDeltaRotationFromRotatorGizmo;
	bool m_isRotationModified = true;

	Vec3 m_localDeltaTranslationFromTranslatorGizmo;
	bool m_isTranslationModified = true;
};