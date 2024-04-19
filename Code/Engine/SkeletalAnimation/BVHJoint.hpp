#pragma once
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <string>
#include <vector>

constexpr int MAXCHILDBONES = 3;

//enum for bvh channel data parsing
enum class BVHChannel;
class VertexBuffer;
class Renderer;
class BVHPose;

class BVHJoint {
public:
	BVHJoint();
	~BVHJoint();
	void SetIsRoot(bool isRoot);
	bool IsRoot() const;
	void SetIsEndSite(bool isEndSite);
	bool IsEndSite() const;
	void SetName(const std::string& name);
	std::string GetName() const;
	void AddBVHChannel(BVHChannel newChannel);
	std::vector<BVHChannel> GetBVHChannels() const;
	int GetNumBVHChannels() const;
	void AddChildJoint(BVHJoint& childBone);
	int GetNumChildJoints() const;
	BVHJoint* GetChildJointOfIndex(int index) const;
	void SetOffset(float xOffset, float yOffset, float zOffset);
	void GetOffset(float& out_xOffset, float& out_yOffset, float& out_zOffset) const;
	Mat44 GetLocalTransformMatrix() const;
	Mat44 GetLocalTranslationMatrix() const;
	Mat44 GetLocalRotationMatrix() const;

	//Mainly used for setting the root joint's position and orientation
	//void SetLocalTranslationMatrix(const Mat44& translationMatrix);
	//void SetLocalRotationMatrix(const Mat44& rotationMatrix);

	void RecursivelySetVertexData(Renderer& rendererForVBOs);
	void RecursivelyRender(Renderer& renderer, const Mat44& parentTransform) const;
	void SetPoseIfThisIsRoot(const BVHPose& frameData);

private:
	void RecursivelySetPose(const BVHPose& frameData, int& jointIndex);

private:
	bool m_isRoot = false;
	bool m_isEndSite = false;
	std::string m_name;
	Mat44 m_rotationMatrix;
	Mat44 m_translationMatrix;
	std::vector<BVHJoint*> m_childJoints;

	//for bvh format
	//int m_numBVHChannels = 0;
	float m_xOffset = 0.0f, m_yOffset = 0.0f, m_zOffset = 0.0f;	//THIS Should be translated for intial T
	std::vector<BVHChannel> m_bvhChannels;
	
	//Render data
	std::vector<VertexBuffer*> m_vbos;	//We need one vbo from this to each child
	std::vector<std::vector<Vertex_PCU>> m_vertsForEachVBO;
};