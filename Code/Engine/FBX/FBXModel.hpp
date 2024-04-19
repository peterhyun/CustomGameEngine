#pragma once
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Fbx/FBXJointGizmosManager.hpp"
#include "Engine/FBX/FBXAnimManager.hpp"
#include "Engine/IKSolver/JacobianIKSolver.hpp"
#include "ThirdParty/fbxsdk/fbxsdk.h"
#include <vector>
#include <string>

class Renderer;
class FBXJoint;
class Shader;
class StructuredBuffer;
class FBXMesh;
class FBXParser;
class Camera;
class FBXAnimManager;

class FBXModelConfig {
public:
	FBXModelConfig(Renderer& renderer, Clock& parentClock) : m_renderer(renderer), m_parentClock(parentClock) {};
	Renderer& m_renderer;
	Clock& m_parentClock;
};

enum class FBXModelSkinningModifier {
	LBS = 0,
	DDM_CPU_v0,
	DDM_CPU_v1,
	DDM_GPU_v0,
	DDM_GPU_v1
};

class FBXModel {
public:
	friend class FBXParser;
	friend class FBXDDMBakingJob;
	~FBXModel();
	FBXModel(const FBXModel& copyFrom) = delete;

	void ToggleAnimationMode();
	void ToggleAnimPause();
	void Update(const Camera& worldCamera);
	void Render(bool renderForLightSpace) const;
	bool IsAnimPlayerClockPaused() const;
	bool IsAnimationMode() const;
	void SetIsAnimationMode(bool isAnimationMode);
	bool IsRootMotionXYFixed() const;
	void SetIsRootMotionXYFixed(bool isRootMotionXYFixed);
	bool HasAnimationData() const;
	unsigned int GetMaxJointStencilValue() const;
	FBXJoint* GetJointByStencilRef(unsigned int stencilValue);	//Called from FBXJointRotatorGizmo
	int GetNumJoints() const;
	void AddDDMModifer();
	void PrecomputeDDM(bool isCPUSide, bool useCotangentLaplacian, int numLaplacianIterations, float lambda, float kappa, float alpha);
	//void ToggleDDM_CPU();
	void ToggleJointVisibility();
	void SetJointVisibility(bool areJointsVisible);
	void ToggleRigidBinding();
	std::string GetFileName() const;
	//void SetDDM_CPU(bool isDDM_CPUActivated);
	void SetSkinningModifierState(FBXModelSkinningModifier modifier);
	FBXModelSkinningModifier GetSkinningModifierState() const;
	bool GetJointVisibility() const;
	FBXJoint* GetSelectedJoint() const;
	void SetRigidBinding(bool isRigidBound);
	int GetNumControlPoints() const;
	int GetNumFaces() const;
	int GetNumVertices() const;
	void SetDDMNeedsRecalculation();

	void ToggleMeshDebugMode();

	void InitiateDDMBaking(FBXParser& fbxParser, const std::string& exportFileName, int numPoses, int numMaxBoneNums, float twistLimit, float pruneThreshold);

	const std::vector<FBXJoint*>& GetJointsArray() const;

	bool IsBakingInProgress() const;

	//void UpdateJointsKeyFrameAnimation(const std::vector<FBXJoint*>& newJoints);

	void SetSelectedJointOfGizmoFromNameIfValid(const std::string& jointName);

	Clock& GetRefToAnimPlayerClock();
	float GetAnimTimeSpan() const;
	void SetAnimClockTotalSeconds(float newTotalSeconds);

	unsigned int GetAnimFPS() const;

	int GetIndexOfJoint(const FBXJoint& joint) const;

	const FBXJoint& GetRootJoint() const;

	void SetIsIKOn(bool isIKOn);

	FBXModel* CreateCopy(const Vec3& offset) const;

private:
	FBXModel(const FBXModelConfig& config, const std::string& fileName);
	void Startup();
	void SetMeshesAndJointsFromParser(const std::vector<FBXMesh*>& meshes, const std::vector<FBXJoint*>& joints);

	void InstantiateGPUData();
	//unsigned int GetKeyframeIndexToPlayBasedOnElapsedTime() const;
	void RenderAllMeshes() const;
	void RenderAllJoints() const;
	void UpdateJointGlobalTransformsStructuredBuffer();
	void ApplyDDMv0_CPU();
	void ApplyDDMv1_CPU();
	void ApplyDDMv0_GPU();
	void ApplyDDMv1_GPU();

	//Helper functions
	std::vector<Mat44> GetJointGlobalBindPoseInverseList() const;
	std::vector<Mat44> GetJointGlobalTransformList() const;

public:
	Shader* m_blinnPhongAnimShader = nullptr;
	FBXAnimManager* m_animManager = nullptr;
	FBXJointGizmosManager m_jointGizmosManager;
	JacobianIKSolver m_ikSolver;

private:
	const std::string m_fileName;
	FBXModelConfig m_config;
	//Clock m_animPlayerClock;
	std::vector<FBXJoint*> m_joints;
	std::vector<FBXMesh*> m_meshes;

	//bool m_isAnimationMode = false;	//Don't need anymore since I have an animmanager now

	bool m_isRootMotionXYFixed = false;

	//Constants
	static const unsigned int SLOT_SBO_JOINTGLOBALANIMTRANSFORMS = 1;
	static const unsigned int SLOT_SBO_JOINTGLOBALBINDPOSEINVERSES = 2;

	Shader* m_boneAnimShader = nullptr;
	Shader* m_lightSpaceShader = nullptr;

	StructuredBuffer* m_sboForJointGlobalTransforms = nullptr;
	StructuredBuffer* m_sboForJointGlobalBindInverses = nullptr;

	unsigned int m_keyframeIdx0ToPlay = 0;
	//unsigned int m_previousKeyframeIdx0 = (unsigned int)-1;

	//bool m_isUsingDDMCPU = false;
	bool m_shouldRenderJoints = true;
	bool m_shouldUseRigidBinding = false;
	bool m_isUsingRigidBinding = false;

	FBXModelSkinningModifier m_skinningModifier = FBXModelSkinningModifier::LBS;
	FBXModelSkinningModifier m_preBakingSkinningModifier = FBXModelSkinningModifier::LBS;

	bool m_isBakingInProgress = false;

	//For rigid binding and gpu recompute
	struct FBXDDMPrecomputeConstants {
		bool m_isCPUSide = false;
		bool m_useCotangentLaplacian = false;
		int m_numLaplacianIterations = 0;
		float m_lambda = 0.0f;
		float m_kappa = 0.0f;
		float m_alpha = 0.0f;
		bool operator==(const FBXDDMPrecomputeConstants other) const {
			return (m_isCPUSide == other.m_isCPUSide) &&
				(m_useCotangentLaplacian == other.m_useCotangentLaplacian) &&
				(m_numLaplacianIterations == other.m_numLaplacianIterations) &&
				(m_lambda == other.m_lambda) &&
				(m_kappa == other.m_kappa) &&
				(m_alpha == other.m_alpha);
		};
	};
	FBXDDMPrecomputeConstants m_latestPrecomputeConstants;

	bool m_isIKOn = false;
};