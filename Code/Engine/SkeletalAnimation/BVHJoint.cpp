#include "Engine/SkeletalAnimation/BVHJoint.hpp"
#include "Engine/SkeletalAnimation/BVHPose.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Quaternion.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/Renderer.hpp"

BVHJoint::BVHJoint()
{
}

BVHJoint::~BVHJoint()
{
	int numVBOs = (int)m_vbos.size();
	for (int i = 0; i < numVBOs; i++) {
		if (m_vbos[i]) {
			delete m_vbos[i];
			m_vbos[i] = nullptr;
		}
	}

	int numChildJoints = (int)m_childJoints.size();
	for (int i = 0; i < numChildJoints; i++) {
		if (m_childJoints[i]) {
			delete m_childJoints[i];
			m_childJoints[i] = nullptr;
		}
	}
}

void BVHJoint::SetIsRoot(bool isRoot)
{
	m_isRoot = isRoot;
}

bool BVHJoint::IsRoot() const
{
	return m_isRoot;
}

void BVHJoint::SetIsEndSite(bool isEndSite)
{
	m_isEndSite = isEndSite;
}

bool BVHJoint::IsEndSite() const
{
	return m_isEndSite;
}

void BVHJoint::SetName(const std::string& name)
{
	m_name = name;
}

std::string BVHJoint::GetName() const
{
	return m_name;
}

void BVHJoint::AddBVHChannel(BVHChannel newChannel)
{
	m_bvhChannels.push_back(newChannel);
}

std::vector<BVHChannel> BVHJoint::GetBVHChannels() const
{
	return m_bvhChannels;
}

int BVHJoint::GetNumBVHChannels() const
{
	return (int)m_bvhChannels.size();
}

void BVHJoint::AddChildJoint(BVHJoint& childBone)
{
	m_childJoints.push_back(&childBone);
}

int BVHJoint::GetNumChildJoints() const
{
	return (int)m_childJoints.size();
}

BVHJoint* BVHJoint::GetChildJointOfIndex(int index) const
{
	if (index < 0 || index >= m_childJoints.size())
		return nullptr;
	return m_childJoints[index];
}

void BVHJoint::SetOffset(float xOffset, float yOffset, float zOffset)
{
	m_xOffset = xOffset;
	m_yOffset = yOffset;
	m_zOffset = zOffset;
}

void BVHJoint::GetOffset(float& out_xOffset, float& out_yOffset, float& out_zOffset) const
{
	out_xOffset = m_xOffset;
	out_yOffset = m_yOffset;
	out_zOffset = m_zOffset;
}

void BVHJoint::RecursivelySetVertexData(Renderer& rendererForVBOs)
{
	if (m_vbos.size() != 0) {	//Don't do anything if the vbos are already set up
		return;
	}

	for (int i = 0; i < m_childJoints.size(); i++) {
		if (m_childJoints[i] == nullptr)
			ERROR_AND_DIE("m_childJoints should never be null");
		float childJointOffset_x = 0.0f, childJointOffset_y = 0.0f, childJointOffset_z = 0.0f;
		if(m_childJoints[i])
			m_childJoints[i]->GetOffset(childJointOffset_x, childJointOffset_y, childJointOffset_z);
		//TODO in the future: Have to fix the x, y, z order ( in BVH z is forward, x is left, and y is up)

		m_vertsForEachVBO.push_back(std::vector<Vertex_PCU>());
		AddVertsForCylinder3D(m_vertsForEachVBO[i], Vec3(0.0f, 0.0f, 0.0f), Vec3(childJointOffset_z, childJointOffset_x, childJointOffset_y), 1.0f, 16);
		VertexBuffer* newVBO = rendererForVBOs.CreateVertexBuffer(m_vertsForEachVBO[i].size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), Stringf("VBO made in BVHJoint %s to child BVHJoint %s", m_name.c_str(), m_childJoints[i]->GetName().c_str()));
		
		rendererForVBOs.CopyCPUToGPU(m_vertsForEachVBO[i].data(), m_vertsForEachVBO[i].size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), newVBO);
		m_vbos.push_back(newVBO);

		m_childJoints[i]->RecursivelySetVertexData(rendererForVBOs);
	}
}

void BVHJoint::RecursivelyRender(Renderer& renderer, const Mat44& parentTransform) const
{
	Mat44 localTransformMatrix = GetLocalTransformMatrix();
	Mat44 globalTransformMatrix = parentTransform;
	globalTransformMatrix.Append(localTransformMatrix);

	renderer.SetModelConstants(globalTransformMatrix);
	for (int i = 0; i < m_vbos.size(); i++) {
		renderer.DrawVertexBuffer(m_vbos[i], (int)m_vertsForEachVBO[i].size());
	}

	for (int i = 0; i < m_childJoints.size(); i++) {
		m_childJoints[i]->RecursivelyRender(renderer, globalTransformMatrix);
	}
}

void BVHJoint::SetPoseIfThisIsRoot(const BVHPose& frameData)
{
	if (!m_isRoot) {
		ERROR_RECOVERABLE("Calling BVHJoint::SetPoseIfThisIsRoot() on a non-root BVHJoint " + m_name);
		return;
	}
	m_translationMatrix = Mat44::CreateTranslation3D(frameData.m_rootPosGH);
	int i = 0;
	RecursivelySetPose(frameData, i);
}

Mat44 BVHJoint::GetLocalTransformMatrix() const
{
	Mat44 transform = m_translationMatrix;
	transform.AppendTranslation3D(Vec3(m_zOffset, m_xOffset, m_yOffset));
	transform.Append(m_rotationMatrix);
	return transform;
}

Mat44 BVHJoint::GetLocalTranslationMatrix() const
{
	return m_translationMatrix;
}

Mat44 BVHJoint::GetLocalRotationMatrix() const
{
	return m_rotationMatrix;
}

/*
void BVHJoint::SetLocalTranslationMatrix(const Mat44& translationMatrix)
{
	m_translationMatrix = translationMatrix;
}
*/

/*
void BVHJoint::SetLocalRotationMatrix(const Mat44& rotationMatrix)
{
	m_rotationMatrix = rotationMatrix;
}
*/

void BVHJoint::RecursivelySetPose(const BVHPose& frameData, int& jointIndex)
{
	if (m_isEndSite) {
		jointIndex--;
		return;
	}
	const Quaternion& jointQuat = frameData.m_jointQuatsGH[jointIndex];
	m_rotationMatrix = jointQuat.GetRotationMatrix();

	for (int i = 0; i < m_childJoints.size(); i++) {
		jointIndex++;
		m_childJoints[i]->RecursivelySetPose(frameData, jointIndex);
	}
}