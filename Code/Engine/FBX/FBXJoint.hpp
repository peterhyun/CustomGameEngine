#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/Quaternion.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/IKSolver/IKSocket.hpp"
#include "ThirdParty/fbxsdk/fbxsdk.h"
#include <vector>
#include <string>

class Shader;
class Renderer;
class VertexBuffer;
class FBXModel;
class FBXPose;

class FBXJoint {
public:
	FBXJoint();
	~FBXJoint();
	FBXJoint(const FBXJoint& copyFrom) = delete;

	void SetName(const std::string& name);
	void AddChildJoints(FBXJoint& childBone);
	FBXJoint* GetParentJoint() const;
	void SetGPUData(Renderer& renderer);
	void SetGlobalBindPose(const FbxAMatrix& globalBindPose);
	void SetGlobalBindPoseInverse(const FbxAMatrix& globalBindPoseInverse);
	void SetStencilRefForThisJoint(unsigned int stencilRef);
	unsigned int GetStencilRefForThisJoint() const;

	/*
	void AddLocalKeyFrame(const FbxAMatrix& key);
	void AddLocalKeyFrame(const Mat44& key);
	void AddLocalTranslationKeyFrame(const FbxVector4& key);
	void AddLocalRotationKeyFrame(const FbxQuaternion& key);
	void AddLocalScalingKeyFrame(const FbxVector4& key);
	*/

	void SetPoseIfThisIsRoot(const FBXPose& pose);

	//From gizmo control
	void AddLocalXRotation(float degrees);
	void AddLocalYRotation(float degrees);
	void AddLocalZRotation(float degrees);
	std::string GetName() const;

	//VERY IMPORTANT! (Must be called at the beginning of each frame)
	/*
	void RecursivelyUpdateGlobalTransformAnimForThisFrame(unsigned int keyframeIndexToPlay, float lerpAlpha, const Mat44& parentTransform);
	*/
	void RecursivelyUpdateGlobalTransformBindPoseForThisFrame(const Mat44& parentTransform);
	void Render(Renderer& renderer) const;

	IKSocket& GetRefToIKSocket();

	Mat44 GetGlobalTransformForThisFrame() const;
	Mat44 GetSkinningMatrixForThisFrame() const;
	
	Mat44 GetGlobalBindPose() const;
	Mat44 GetGlobalBindPoseInverse() const;
	/*
	Mat44 GetLocalKeyFrame(unsigned int index) const;
	unsigned int   GetNumLocalKeyFrames() const;
	void  ClearLocalKeyFrames();
	*/

	void SetOriginalLocalTranslate(const FbxVector4& originalLocalTranslate);
	Vec3 GetOriginalLocalTranslate() const;
	void SetLocalDeltaTranslate(const Vec3& localDeltaTranslate);
	void ResetLocalDeltaTranslate();
	/*
	Vec3 GetLocalDeltaTranslate() const;
	void SetLocalDeltaTranslate(const Vec3& localDeltaTransform);
	*/

	Quaternion GetOriginalLocalRotate() const;
	void SetOriginalLocalRotate(const FbxQuaternion& originalLocalRotate);
	Quaternion GetLocalDeltaRotate() const;
	void SetLocalDeltaRotate(const Quaternion& localDeltaTransform);
	Quaternion GetTotalLocalRotate() const;
	void ResetLocalDeltaRotate();

	void SetIsRoot(bool isRoot);
	void SetIsEndJoint(bool isEndJoint);

	int  GetNumChildJoints() const;
	FBXJoint* GetChildJointByIdx(unsigned int idx) const;
	void ClearChildJoints();

	void SetFBXModel(FBXModel& model);
	FBXModel* GetFBXModel() const;

	bool IsRotationModifiedByGizmo() const;

	float GetConeSphereRadius() const;
	void SetConeSphereRadius(float newConeSphereRadius);

	bool IsRoot() const;

	void GetDOFAxisSettings(bool& isXAxisDOF, bool& isYAxisDOF, bool& isZAxisDOF) const;
	void SetDOFAxisSettings(bool isXAxisDOF, bool isYAxisDOF, bool isZAxisDOF);
	int GetNumDOFs() const;

	//void GetAxisIKWeights(float& xAxisWeight, float& yAxisWeight, float& zAxisWeight) const;
	//void SetAxisIKWeights(float xAxisWeight, float yAxisWeight, float zAxisWeight);
	float GetIKSolverWeight() const;
	void SetIKSolverWeight(float ikSolverWeight);
	FloatRange GetValidIKWeightRange() const;

	FBXJoint* CreateCopy() const;

	bool IsTranslationModifiedByGizmo() const;

private:
	Mat44 GetLocalBindPoseTransform() const;

private:
	FBXModel* m_model = nullptr;
	Shader* m_shader = nullptr;
	std::string m_name;

	//Render Data
	std::vector<VertexBuffer*> m_vbos;
	std::vector<std::vector<Vertex_PCU>> m_vertsForEachVBO;

	FBXJoint* m_parentJoint = nullptr;
	std::vector<FBXJoint*> m_childJoints;

	/*
	std::vector<Mat44> m_localKeyFrames;
	std::vector<Vec4> m_localTranslationKeyFrames;
	std::vector<Quaternion> m_localRotationKeyFrames;
	std::vector<Vec4> m_localScalingKeyFrames;
	*/
	//std::vector<Mat44> m_globalKeyFrames;
	
	//Update this in every loop
	Mat44 m_globalTransformForThisFrame;

	Mat44 m_globalBindPose;
	Mat44 m_globalBindPoseInverse;

	Vec3 m_originalLocalTranslate;
	Vec3 m_localDeltaTranslateFromTranslatorGizmo;

	Quaternion m_originalLocalRotate;
	Quaternion m_localDeltaRotateFromRotatorGizmo;

	bool m_isRoot = false;
	bool m_isEndJoint = false;

	static const unsigned int SLOT_CBO_FBXJOINTGLOBALANIMTRANSFORM = 4;
	static const unsigned int SLOT_CBO_FBXJOINTGLOBALBINDPOSE = 5;
	static const unsigned int SLOT_CBO_FBXJOINTGLOBALANIMSIGNAL = 6;
	unsigned int m_stencilRefForThisJoint = 0;

	bool m_isRotationModified = false;
	bool m_isTranslationModified = false;

	float m_coneSphereRadius = 0.7f;

	IKSocket m_ikSocket;

	bool m_isXAxisDOF = true;
	bool m_isYAxisDOF = true;
	bool m_isZAxisDOF = true;

	float m_ikSolverWeight = 1.0f;
	/*
	float m_yAxisIKWeight = 1.0f;
	float m_zAxisIKWeight = 1.0f;
	*/

	const FloatRange m_validIKWeightRange = FloatRange(0.5f, 1.5f);
};