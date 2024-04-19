#include "Engine/Fbx/FBXJoint.hpp"
#include "Engine/Fbx/FBXUtils.hpp"
#include "Engine/Fbx/FBXModel.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include <queue>

FBXJoint::FBXJoint() : m_ikSocket(*this)
{
}

FBXJoint::~FBXJoint()
{
	for (int i = 0; i < m_vbos.size(); i++) {
		if (m_vbos[i]) {
			delete m_vbos[i];
			m_vbos[i] = nullptr;
		}
	}
}

void FBXJoint::SetName(const std::string& name)
{
	m_name = name;
}

void FBXJoint::AddChildJoints(FBXJoint& childBone)
{
	m_childJoints.push_back(&childBone);
	childBone.m_parentJoint = this;
}

FBXJoint* FBXJoint::GetParentJoint() const
{
	return m_parentJoint;
}

void FBXJoint::SetGPUData(Renderer& renderer)
{
	for (int i = 0; i < m_childJoints.size(); i++) {
		if (m_childJoints[i] == nullptr)
			ERROR_AND_DIE("m_childJoints should never be null");
		Mat44 childJointLocalTransform = m_childJoints[i]->GetLocalBindPoseTransform();
		Vec3 childJointStartPos = childJointLocalTransform.TransformPosition3D(Vec3(0.0f, 0.0f, 0.0f));
		m_vertsForEachVBO.push_back(std::vector<Vertex_PCU>());
		//I'm not using childJointLclRotation, childJointLclScale...
		AddVertsForSphere3D(m_vertsForEachVBO[i], Vec3(0.0f, 0.0f, 0.0f), m_coneSphereRadius);
		AddVertsForCone3D(m_vertsForEachVBO[i], Vec3(0.0f, 0.0f, 0.0f), childJointStartPos, m_coneSphereRadius);
		//AddVertsForCylinder3D(m_vertsForEachVBO[i], Vec3(0.0f, 0.0f, 0.0f), childJointLclTranslation, 1.0f, 16);
		VertexBuffer* newVBO = renderer.CreateVertexBuffer(m_vertsForEachVBO[i].size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), Stringf("VBO made in VBXJoint %s to child FBXJoint %s", m_name.c_str(), m_childJoints[i]->GetName().c_str()));
		renderer.CopyCPUToGPU(m_vertsForEachVBO[i].data(), m_vertsForEachVBO[i].size() * sizeof(Vertex_PCU), sizeof(Vertex_PCU), newVBO);
		m_vbos.push_back(newVBO);
	}

	m_ikSocket.SetGPUData(renderer);
}

void FBXJoint::SetGlobalBindPose(const FbxAMatrix& globalBindPose)
{
	m_globalBindPose = ConvertFbxAMatrixToMat44(globalBindPose);
}

void FBXJoint::SetGlobalBindPoseInverse(const FbxAMatrix& globalBindPoseInverse)
{
	m_globalBindPoseInverse = ConvertFbxAMatrixToMat44(globalBindPoseInverse);
}

void FBXJoint::SetStencilRefForThisJoint(unsigned int stencilRef)
{
	m_stencilRefForThisJoint = stencilRef;
}

unsigned int FBXJoint::GetStencilRefForThisJoint() const
{
	return m_stencilRefForThisJoint;
}

/*
void FBXJoint::AddLocalKeyFrame(const FbxAMatrix& key)
{
	Mat44 keyGH = ConvertFbxAMatrixToMat44(key);
	m_localKeyFrames.push_back(keyGH);
}

void FBXJoint::AddLocalKeyFrame(const Mat44& key)
{
	m_localKeyFrames.push_back(key);
}

void FBXJoint::AddLocalTranslationKeyFrame(const FbxVector4& key)
{
	m_localTranslationKeyFrames.emplace_back((float)key.mData[0], (float)key.mData[1], (float)key.mData[2], (float)key.mData[3]);
}

void FBXJoint::AddLocalRotationKeyFrame(const FbxQuaternion& key)
{
	//My Quaternion class uses a w, x, y, z ordered constructor but key is x, y, z, w ordered
	m_localRotationKeyFrames.emplace_back((float)key.mData[3], (float)key.mData[0], (float)key.mData[1], (float)key.mData[2]);
}

void FBXJoint::AddLocalScalingKeyFrame(const FbxVector4& key)
{
	m_localScalingKeyFrames.emplace_back((float)key.mData[0], (float)key.mData[1], (float)key.mData[2], (float)key.mData[3]);
}
*/

void FBXJoint::SetPoseIfThisIsRoot(const FBXPose& pose)
{
	if (m_isRoot == false) {
		ERROR_RECOVERABLE("FBXJoint::SetPoseIfThisIsRoot() called for a non-root!");
		return;
	}
	
	GUARANTEE_OR_DIE(pose.GetRootJoint() == this, "Pose applied to a different skeleton");

	struct JointAndIdx {
		FBXJoint* m_joint = nullptr;
		Mat44 m_parentTransform;
	};

	std::queue<JointAndIdx> pairsToProcess;
	pairsToProcess.push({this, Mat44()});
	while (!pairsToProcess.empty()) {
		JointAndIdx pair = pairsToProcess.front();
		pairsToProcess.pop();

		const FBXModel* model = pair.m_joint->GetFBXModel();
		GUARANTEE_OR_DIE(model != nullptr, "joints should not have a nullptr model");
		int jointIndex = model->GetIndexOfJoint(*pair.m_joint);
		GUARANTEE_OR_DIE(jointIndex != -1, "joint doesn't exist in model");

		const Mat44 S = Mat44::CreateNonUniformScale3D(Vec3(pose.m_localScalings[jointIndex]));
		const Mat44 R = pose.m_localQuats[jointIndex].GetRotationMatrix();
		Mat44 T = Mat44::CreateTranslation3D(Vec3(pose.m_localLocs[jointIndex]));

		FBXJoint* jointToProcess = pair.m_joint;
		if (jointToProcess->m_isRoot && jointToProcess->m_isTranslationModified) {
			T.Append(Mat44::CreateTranslation3D(jointToProcess->m_localDeltaTranslateFromTranslatorGizmo));
		}

		Mat44 finalLocalKeyframe = T;
		finalLocalKeyframe.Append(R);
		finalLocalKeyframe.Append(S);

		jointToProcess->m_globalTransformForThisFrame = pair.m_parentTransform;
		jointToProcess->m_globalTransformForThisFrame.Append(finalLocalKeyframe);
		if (jointToProcess->m_isRotationModified) {
			Mat44 rotationMat = jointToProcess->m_localDeltaRotateFromRotatorGizmo.GetRotationMatrix();
			jointToProcess->m_globalTransformForThisFrame.Append(rotationMat);
		}

		for (FBXJoint* childJoint : jointToProcess->m_childJoints) {
			pairsToProcess.push({ childJoint, jointToProcess->m_globalTransformForThisFrame });
		}
	}
}

void FBXJoint::AddLocalXRotation(float degrees)
{
	Quaternion xRot = Quaternion::CreateFromAxisAndDegrees(degrees, Vec3(1.0f, 0.0f, 0.0f));
	m_localDeltaRotateFromRotatorGizmo = m_localDeltaRotateFromRotatorGizmo * xRot;
	m_isRotationModified = true;
}

void FBXJoint::AddLocalYRotation(float degrees)
{
	Quaternion yRot = Quaternion::CreateFromAxisAndDegrees(degrees, Vec3(0.0f, 1.0f, 0.0f));
	m_localDeltaRotateFromRotatorGizmo = m_localDeltaRotateFromRotatorGizmo * yRot;
	m_isRotationModified = true;
}

void FBXJoint::AddLocalZRotation(float degrees)
{
	Quaternion zRot = Quaternion::CreateFromAxisAndDegrees(degrees, Vec3(0.0f, 0.0f, 1.0f));
	m_localDeltaRotateFromRotatorGizmo = m_localDeltaRotateFromRotatorGizmo * zRot;
	m_isRotationModified = true;
}

std::string FBXJoint::GetName() const
{
	return m_name;
}

Mat44 FBXJoint::GetGlobalBindPose() const
{
	return m_globalBindPose;
}

Mat44 FBXJoint::GetGlobalBindPoseInverse() const
{
	return m_globalBindPoseInverse;
}

/*
Mat44 FBXJoint::GetLocalKeyFrame(unsigned int index) const
{
	if (index >= (unsigned int)m_localKeyFrames.size()) {
		return Mat44();
	}
	return m_localKeyFrames[index];
}

unsigned int FBXJoint::GetNumLocalKeyFrames() const
{
	return (unsigned int)m_localKeyFrames.size();
}

void FBXJoint::ClearLocalKeyFrames()
{
	m_localKeyFrames.clear();
}
*/

void FBXJoint::SetOriginalLocalTranslate(const FbxVector4& originalLocalTranslate)
{
	GUARANTEE_OR_DIE((float)originalLocalTranslate.mData[3] == 0.0f, "Making sure there's not loss of data!");
	m_originalLocalTranslate = Vec3((float)originalLocalTranslate.mData[0], (float)originalLocalTranslate.mData[1], (float)originalLocalTranslate.mData[2]);
}

Vec3 FBXJoint::GetOriginalLocalTranslate() const
{
	return m_originalLocalTranslate;
}

void FBXJoint::SetLocalDeltaTranslate(const Vec3& localDeltaTranslate)
{
	GUARANTEE_OR_DIE(m_isRoot == true, "HAS TO BE ROOT!");
	m_localDeltaTranslateFromTranslatorGizmo = localDeltaTranslate;
	m_isTranslationModified = true;
}

void FBXJoint::ResetLocalDeltaTranslate()
{
	m_model->SetDDMNeedsRecalculation();
	m_localDeltaTranslateFromTranslatorGizmo = Vec3();
	m_isTranslationModified = false;
}

/*
Vec3 FBXJoint::GetLocalDeltaTranslate() const
{
	return m_localDeltaTranslateFromTranslatorGizmo;
}

void FBXJoint::ResetLocalDeltaTranslate()
{
	m_model->SetDDMNeedsRecalculation();
	m_localDeltaTranslateFromTranslatorGizmo = Vec3();
	m_isTranslationModified = false;
}
*/

Quaternion FBXJoint::GetOriginalLocalRotate() const
{
	return m_originalLocalRotate;
}

void FBXJoint::SetOriginalLocalRotate(const FbxQuaternion& originalLocalRotate)
{
	m_originalLocalRotate = Quaternion((float)originalLocalRotate.mData[3], (float)originalLocalRotate.mData[0], (float)originalLocalRotate.mData[1], (float)originalLocalRotate.mData[2]);
}

Quaternion FBXJoint::GetLocalDeltaRotate() const
{
	return m_localDeltaRotateFromRotatorGizmo;
}

void FBXJoint::SetLocalDeltaRotate(const Quaternion& localDeltaRotate)
{
	m_localDeltaRotateFromRotatorGizmo = localDeltaRotate;
	m_isRotationModified = true;
}

Quaternion FBXJoint::GetTotalLocalRotate() const
{
	Quaternion totalLocalRotate = m_originalLocalRotate;
	if (m_isRotationModified) {
		totalLocalRotate = totalLocalRotate * m_localDeltaRotateFromRotatorGizmo;
	}
	return totalLocalRotate;
}

void FBXJoint::ResetLocalDeltaRotate()
{
	m_model->SetDDMNeedsRecalculation();
	m_localDeltaRotateFromRotatorGizmo = Quaternion();
	m_isRotationModified = false;
}

Mat44 FBXJoint::GetLocalBindPoseTransform() const
{
	Mat44 localTransformToReturn = Mat44::CreateTranslation3D(m_originalLocalTranslate);
	if (m_isTranslationModified)
		localTransformToReturn.Append(Mat44::CreateTranslation3D(m_localDeltaTranslateFromTranslatorGizmo));

	localTransformToReturn.Append(m_originalLocalRotate.GetRotationMatrix());
	if(m_isRotationModified)
		localTransformToReturn.Append(m_localDeltaRotateFromRotatorGizmo.GetRotationMatrix());

	return localTransformToReturn;
}

void FBXJoint::SetIsRoot(bool isRoot)
{
	m_isRoot = isRoot;
}

void FBXJoint::SetIsEndJoint(bool isEndJoint)
{
	m_isEndJoint = isEndJoint;
}

/*
void FBXJoint::RecursivelyUpdateGlobalTransformAnimForThisFrame(unsigned int keyframeIndex, float lerpAlpha, const Mat44& m_parentTransform)
{
	if (m_localKeyFrames.size() == 0)	//end effector: Don't need to go further
		return;
	if (keyframeIndex >= m_localKeyFrames.size()) {
		ERROR_AND_DIE("This can't be happening");
	}

	Mat44 finalLocalKeyFrame;
	if (keyframeIndex < m_localKeyFrames.size() - 1) {
		Vec4 keyFrame0Trans = m_localTranslationKeyFrames[keyframeIndex];
		Vec4 keyFrame1Trans = m_localTranslationKeyFrames[keyframeIndex + 1];
		Vec4 lerpedTrans = Lerp(keyFrame0Trans, keyFrame1Trans, lerpAlpha);

		Quaternion keyFrame0Rot = m_localRotationKeyFrames[keyframeIndex];
		Quaternion keyFrame1Rot = m_localRotationKeyFrames[keyframeIndex + 1];
		Quaternion slerpedQuat = Quaternion::Slerp(keyFrame0Rot, keyFrame1Rot, lerpAlpha);
		
		Vec4 keyFrame0Scale = m_localScalingKeyFrames[keyframeIndex];
		Vec4 keyFrame1Scale = m_localScalingKeyFrames[keyframeIndex + 1];
		Vec4 lerpedScale = Lerp(keyFrame0Scale, keyFrame1Scale, lerpAlpha);

		if (m_isRoot && m_model->IsRootMotionXYFixed()) {
			lerpedTrans.x = 0.0f;
			lerpedTrans.y = 0.0f;
		}
		Mat44 T = Mat44::CreateTranslation3D(Vec3(lerpedTrans.x, lerpedTrans.y, lerpedTrans.z));
		Mat44 R = slerpedQuat.GetRotationMatrix();
		Mat44 S = Mat44::CreateNonUniformScale3D(Vec3(lerpedScale.x, lerpedScale.y, lerpedScale.z));

		
		//Mat44 noTranslationLocalTransform = m_originalLocalTransform;
		//noTranslationLocalTransform.RemoveTranslation3D();
		//finalLocalKeyFrame = T;
		//finalLocalKeyFrame.Append(noTranslationLocalTransform);
		
		finalLocalKeyFrame = T;
		finalLocalKeyFrame.Append(R);
		finalLocalKeyFrame.Append(S);
		//finalLocalKeyFrame.Append(noTranslationLocalTransform);
	}
	else {	//Literally the last keyframe
		finalLocalKeyFrame = m_localKeyFrames[keyframeIndex];
		if (m_isRoot && m_model->IsRootMotionXYFixed()) {
			Vec3 currentTrans = finalLocalKeyFrame.GetTranslation3D();
			finalLocalKeyFrame.SetTranslation3D(Vec3(0.0f, 0.0f, currentTrans.z));
		}
	}

	m_globalTransformForThisFrame = m_parentTransform;
	m_globalTransformForThisFrame.Append(finalLocalKeyFrame);
	if(m_isRotationModified)
		m_globalTransformForThisFrame.Append(m_localDeltaTransformFromRotatorGizmo);

	for (int i = 0; i < m_childJoints.size(); i++) {
		if (m_childJoints[i]) {
			m_childJoints[i]->RecursivelyUpdateGlobalTransformAnimForThisFrame(keyframeIndex, lerpAlpha, m_globalTransformForThisFrame);
		}
	}
}
*/

void FBXJoint::RecursivelyUpdateGlobalTransformBindPoseForThisFrame(const Mat44& parentTransform)
{
	m_globalTransformForThisFrame = parentTransform;
	Mat44 localTransform = GetLocalBindPoseTransform();
	m_globalTransformForThisFrame.Append(localTransform);

	for (int i = 0; i < m_childJoints.size(); i++) {
		if (m_childJoints[i]) {
			m_childJoints[i]->RecursivelyUpdateGlobalTransformBindPoseForThisFrame(m_globalTransformForThisFrame);
		}
	}
}

void FBXJoint::Render(Renderer& renderer) const
{
	if (m_childJoints.size() == 0)
		return;

	renderer.SetModelConstants(m_globalTransformForThisFrame);
	renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_DISABLED_STENCIL_ENABLED, m_stencilRefForThisJoint);

	for (int i = 0; i < m_vbos.size(); i++) {
		renderer.DrawVertexBuffer(m_vbos[i], (int)m_vertsForEachVBO[i].size());
	}
}

IKSocket& FBXJoint::GetRefToIKSocket()
{
	return m_ikSocket;
}

Mat44 FBXJoint::GetGlobalTransformForThisFrame() const
{
	return m_globalTransformForThisFrame;
}

Mat44 FBXJoint::GetSkinningMatrixForThisFrame() const
{
	Mat44 skinningMatrix = m_globalTransformForThisFrame;
	skinningMatrix.Append(m_globalBindPoseInverse);
	return skinningMatrix;
}

int FBXJoint::GetNumChildJoints() const
{
	return (int)m_childJoints.size();
}

FBXJoint* FBXJoint::GetChildJointByIdx(unsigned int idx) const
{
	GUARANTEE_OR_DIE(idx < m_childJoints.size(), "idx >= m_childJoints.size() in FBxJoint::GetChildJointByIdx()");
	return m_childJoints[idx];
}

void FBXJoint::ClearChildJoints()
{
	m_childJoints.clear();
}

void FBXJoint::SetFBXModel(FBXModel& model)
{
	m_model = &model;
}

FBXModel* FBXJoint::GetFBXModel() const
{
	return m_model;
}

bool FBXJoint::IsRotationModifiedByGizmo() const
{
	return m_isRotationModified;
}

float FBXJoint::GetConeSphereRadius() const
{
	return m_coneSphereRadius;
}

void FBXJoint::SetConeSphereRadius(float newConeSphereRadius)
{
	m_coneSphereRadius = newConeSphereRadius;
}

bool FBXJoint::IsRoot() const
{
	return m_isRoot;
}

void FBXJoint::GetDOFAxisSettings(bool& isXAxisDOF, bool& isYAxisDOF, bool& isZAxisDOF) const
{
	isXAxisDOF = m_isXAxisDOF;
	isYAxisDOF = m_isYAxisDOF;
	isZAxisDOF = m_isZAxisDOF;
}

void FBXJoint::SetDOFAxisSettings(bool isXAxisDOF, bool isYAxisDOF, bool isZAxisDOF)
{
	m_isXAxisDOF = isXAxisDOF;
	m_isYAxisDOF = isYAxisDOF;
	m_isZAxisDOF = isZAxisDOF;
}

int FBXJoint::GetNumDOFs() const
{
	int count = 0;
	if (m_isXAxisDOF)
		count++;
	if (m_isYAxisDOF)
		count++;
	if(m_isZAxisDOF)
		count++;
	return count;
}

/*
void FBXJoint::GetAxisIKWeights(float& xAxisWeight, float& yAxisWeight, float& zAxisWeight) const
{
	xAxisWeight = m_xAxisIKWeight;
	yAxisWeight = m_yAxisIKWeight;
	zAxisWeight = m_zAxisIKWeight;
}

void FBXJoint::SetAxisIKWeights(float xAxisWeight, float yAxisWeight, float zAxisWeight)
{
	m_xAxisIKWeight = xAxisWeight;
	m_yAxisIKWeight = yAxisWeight;
	m_zAxisIKWeight = zAxisWeight;
}
*/

float FBXJoint::GetIKSolverWeight() const
{
	return m_ikSolverWeight;
}

void FBXJoint::SetIKSolverWeight(float ikSolverWeight)
{
	GUARANTEE_OR_DIE(m_validIKWeightRange.IsOnRange(ikSolverWeight), "Check value of weight");
	m_ikSolverWeight = ikSolverWeight;
}

FloatRange FBXJoint::GetValidIKWeightRange() const
{
	return m_validIKWeightRange;
}

FBXJoint* FBXJoint::CreateCopy() const
{
	FBXJoint* copy = new FBXJoint;
	copy->m_shader = m_shader;
	copy->m_name = m_name;

	copy->m_globalTransformForThisFrame = m_globalTransformForThisFrame;
	copy->m_globalBindPose = m_globalBindPose;
	copy->m_globalBindPoseInverse = m_globalBindPoseInverse;

	copy->m_originalLocalTranslate = m_originalLocalTranslate;
	copy->m_localDeltaTranslateFromTranslatorGizmo = m_localDeltaTranslateFromTranslatorGizmo;

	copy->m_originalLocalRotate = m_originalLocalRotate;
	copy->m_localDeltaRotateFromRotatorGizmo = m_localDeltaRotateFromRotatorGizmo;

	copy->m_isRoot = m_isRoot;
	copy->m_isEndJoint = m_isEndJoint;

	copy->m_stencilRefForThisJoint = m_stencilRefForThisJoint;

	copy->m_isRotationModified = m_isRotationModified;
	copy->m_isTranslationModified = m_isTranslationModified;

	copy->m_coneSphereRadius = m_coneSphereRadius;

	copy->m_isXAxisDOF = m_isXAxisDOF;
	copy->m_isYAxisDOF = m_isYAxisDOF;
	copy->m_isZAxisDOF = m_isZAxisDOF;

	return copy;
}

bool FBXJoint::IsTranslationModifiedByGizmo() const
{
	return m_isTranslationModified;
}
