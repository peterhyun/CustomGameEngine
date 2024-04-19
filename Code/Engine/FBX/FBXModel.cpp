#include "Engine/Fbx/FBXModel.hpp"
#include "Engine/Fbx/FBXJoint.hpp"
#include "Engine/Fbx/FBXControlPoint.hpp"
#include "Engine/Fbx/FBXMesh.hpp"
#include "Engine/Fbx/FBXDDMModifierCPU.hpp"
#include "Engine/Fbx/FBXDDMModifierGPU.hpp"
#include "Engine/Fbx/FBXDDMBakingJob.hpp"
#include "Engine/Fbx/FBXDDMV0CPUJob.hpp"
#include "Engine/Fbx/FBXDDMV1CPUJob.hpp"
#include "Engine/Fbx/FBXDDMOmegaPrecomputeJob.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/GPUMesh.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"

FBXModel::FBXModel(const FBXModelConfig& config, const std::string& fileName)
	: m_config(config), m_jointGizmosManager(*this, config.m_renderer), m_fileName(fileName), m_ikSolver(nullptr)
{
	//m_animPlayerClock.Pause();
}

void FBXModel::SetMeshesAndJointsFromParser(const std::vector<FBXMesh*>& meshes, const std::vector<FBXJoint*>& joints)
{
	GUARANTEE_OR_DIE(meshes.size() > 0 && joints.size() > 0, "Model should have at least one mesh and joint");

	m_meshes = meshes;
	m_joints = joints;

	for (int i = 0; i < m_joints.size(); i++) {
		if (m_joints[i]) {
			m_joints[i]->SetFBXModel(*this);
		}
	}

	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			m_meshes[i]->SetFBXModel(*this);
		}
	}
}

void FBXModel::SetRigidBinding(bool isRigidBind)
{
	for (int i = 0; i < m_meshes.size(); i++) {
		m_meshes[i]->SetRigidBinding(isRigidBind);
	}

	//Have to precompute again if it's not lbs
	if (m_skinningModifier != FBXModelSkinningModifier::LBS) {
		PrecomputeDDM(
			m_latestPrecomputeConstants.m_isCPUSide,
			m_latestPrecomputeConstants.m_useCotangentLaplacian,
			m_latestPrecomputeConstants.m_numLaplacianIterations,
			m_latestPrecomputeConstants.m_lambda,
			m_latestPrecomputeConstants.m_kappa,
			m_latestPrecomputeConstants.m_alpha
		);
	}

	for (int i = 0; i < m_meshes.size(); i++) {
		m_meshes[i]->GetDDMModifierCPU()->SetNeedsRecalculation();
		m_meshes[i]->GetDDMModifierGPU()->SetNeedsRecalculation();
	}
}

void FBXModel::InstantiateGPUData()
{
	m_blinnPhongAnimShader = m_config.m_renderer.CreateOrGetShader("Data/Shaders/FbxLit");
	m_lightSpaceShader = m_config.m_renderer.CreateOrGetShader("Data/Shaders/LightSpace");
	m_boneAnimShader = m_config.m_renderer.CreateOrGetShader("Data/Shaders/Bone");

	for (int i = 0; i < m_joints.size(); i++) {
		if (m_joints[i]) {
			m_joints[i]->SetGPUData(m_config.m_renderer);
		}
	}

	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			m_meshes[i]->SetGPUData(m_config.m_renderer);
		}
	}

	m_sboForJointGlobalTransforms = m_config.m_renderer.CreateStructuredBuffer(size_t(m_joints.size() * sizeof(Mat44)), unsigned int(sizeof(Mat44)), (unsigned int)m_joints.size());
	m_sboForJointGlobalBindInverses   = m_config.m_renderer.CreateStructuredBuffer(size_t(m_joints.size() * sizeof(Mat44)), unsigned int(sizeof(Mat44)), (unsigned int)m_joints.size());
}

int FBXModel::GetNumControlPoints() const
{
	int numControlPoints = 0;
	for (int i = 0; i < m_meshes.size(); i++) {
		numControlPoints += (int)m_meshes[i]->GetControlPointsMatrixRestPose().rows();
	}
	return numControlPoints;
}

FBXModel::~FBXModel()
{
	if (m_sboForJointGlobalTransforms) {
		delete m_sboForJointGlobalTransforms;
		m_sboForJointGlobalTransforms = nullptr;
	}
	if (m_sboForJointGlobalBindInverses) {
		delete m_sboForJointGlobalBindInverses;
		m_sboForJointGlobalBindInverses = nullptr;
	}

	for (int i = 0; i < m_meshes.size(); i++) {
		delete m_meshes[i];
		m_meshes[i] = nullptr;
	}

	for (int i = 0; i < m_joints.size(); i++) {
		delete m_joints[i];
		m_joints[i] = nullptr;
	}

	delete m_animManager;
}

void FBXModel::Startup()
{
	m_jointGizmosManager.Startup();

	GUARANTEE_OR_DIE(m_meshes.size() > 0, "Did you set up the meshes of FBXModel before calling FBXModel::Startup()?");
	AABB3 completeAABB3 = m_meshes[0]->GetBoundingBox();
	for (size_t i = 1; i < m_meshes.size(); i++) {
		completeAABB3 = MergeAABB3s(completeAABB3, m_meshes[i]->GetBoundingBox());
	}

	float shortestDim = completeAABB3.GetShortestDimensionSize();
	for (size_t i = 0; i < m_joints.size(); i++) {
		float defaultRadius = m_joints[i]->GetConeSphereRadius();
		float newRadius = RangeMapClamped(shortestDim, 0.25f, 10.0f, defaultRadius * 0.1f, defaultRadius);
		m_joints[i]->SetConeSphereRadius( newRadius );
	}

	InstantiateGPUData();
	std::vector<Mat44> jointGlobalBindInverseList = GetJointGlobalBindPoseInverseList();
	m_config.m_renderer.CopyCPUToGPU(jointGlobalBindInverseList.data(), jointGlobalBindInverseList.size() * sizeof(Mat44), unsigned int(sizeof(Mat44)), (unsigned int)jointGlobalBindInverseList.size(), m_sboForJointGlobalBindInverses);
	m_config.m_renderer.BindStructuredBufferToVS(m_sboForJointGlobalBindInverses, SLOT_SBO_JOINTGLOBALBINDPOSEINVERSES);	//This should NOT be called every frame
}

void FBXModel::ToggleAnimationMode()
{
	m_animManager->ToggleActivation();
	
	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			m_meshes[i]->GetDDMModifierCPU()->SetNeedsRecalculation();
			m_meshes[i]->GetDDMModifierGPU()->SetNeedsRecalculation();
		}
		else {
			ERROR_AND_DIE("FBXModel::m_meshes[i] == nullptr");
		}
	}
}

void FBXModel::ToggleAnimPause()
{
	m_animManager->ToggleAnimPause();
}

void FBXModel::Update(const Camera& worldCamera)
{
	//Check if baking is in progress
	if (m_isBakingInProgress) {
		Job* completedBakingJob = g_theJobSystem->GetCompletedJob();
		GUARANTEE_OR_DIE(dynamic_cast<FBXDDMV0CPUJob*>(completedBakingJob) == nullptr, "Make sure no other job is in progress when baking job is happening!");
		GUARANTEE_OR_DIE(dynamic_cast<FBXDDMV1CPUJob*>(completedBakingJob) == nullptr, "Make sure no other job is in progress when baking job is happening!");
		GUARANTEE_OR_DIE(dynamic_cast<FBXDDMOmegaPrecomputeJob*>(completedBakingJob) == nullptr, "Make sure no other job is in progress when baking job is happening!");
		GUARANTEE_OR_DIE(m_skinningModifier == FBXModelSkinningModifier::LBS, "When baking stats, the modifier should change to LBS!");

		FBXDDMBakingJob* realBakingJob = dynamic_cast<FBXDDMBakingJob*>(completedBakingJob);

		if (m_joints.size() > 0 && m_joints[0]) {
			m_joints[0]->RecursivelyUpdateGlobalTransformBindPoseForThisFrame(Mat44());
			UpdateJointGlobalTransformsStructuredBuffer();
		}

		if (realBakingJob) {
			int numJoints = (int)GetNumJoints();
			//Restore joint transformations
			for (int jointIdx = 0; jointIdx < numJoints; jointIdx++) {
				//m_joints[jointIdx]->ResetLocalDeltaTranslate();
				m_joints[jointIdx]->ResetLocalDeltaRotate();
			}
			for (int meshIdx = 0; meshIdx < m_meshes.size(); meshIdx++) {
				m_meshes[meshIdx]->RestoreGPUVerticesToRestPose();
			}

			delete realBakingJob;
			m_isBakingInProgress = false;
			SetSkinningModifierState(m_preBakingSkinningModifier);
		}
		return;
	}

	m_jointGizmosManager.Update(worldCamera);
	if (m_animManager->IsActive()) {
		m_animManager->Update();

		//DebuggerPrintf("m_keyframeIdx0ToPlay: %d\n", m_keyframeIdx0ToPlay);

		if (m_skinningModifier != FBXModelSkinningModifier::LBS) {
			SetDDMNeedsRecalculation();
		}

		if (m_joints.size() > 0 && m_joints[0]) {
			const FBXPose& pose = m_animManager->GetPoseForThisFrame();
			m_joints[0]->SetPoseIfThisIsRoot(pose);
		}
	}
	else {
		if (m_joints.size() > 0 && m_joints[0]) {
			m_joints[0]->RecursivelyUpdateGlobalTransformBindPoseForThisFrame(Mat44());
		}
	}
	if (m_isIKOn) {
		m_ikSolver.Solve();
	}

	switch (m_skinningModifier) {
	case FBXModelSkinningModifier::LBS:
		break;
	case FBXModelSkinningModifier::DDM_CPU_v0:
		ApplyDDMv0_CPU();
		break;
	case FBXModelSkinningModifier::DDM_CPU_v1:
		ApplyDDMv1_CPU();
		break;
	case FBXModelSkinningModifier::DDM_GPU_v0:
		ApplyDDMv0_GPU();
		break;
	case FBXModelSkinningModifier::DDM_GPU_v1:
		ApplyDDMv1_GPU();
		break;
	default:
		ERROR_AND_DIE("Invalid skinning state!");
	}
	UpdateJointGlobalTransformsStructuredBuffer();
}

void FBXModel::Render(bool renderForLightSpace) const
{
	//if (m_skinningModifier == FBXModelSkinningModifier::LBS) {
	m_config.m_renderer.BindStructuredBufferToVS(m_sboForJointGlobalTransforms, SLOT_SBO_JOINTGLOBALANIMTRANSFORMS);	//This has to be called every frame, unlike the global bind inverse
	//}
	m_config.m_renderer.SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	m_config.m_renderer.SetDepthStencilMode(DepthStencilMode::DEPTH_ENABLED_STENCIL_DISABLED);

	if (renderForLightSpace) {
		m_config.m_renderer.BindShader(m_lightSpaceShader);
		RenderAllMeshes();
	}
	else {
		m_config.m_renderer.BindShader(m_blinnPhongAnimShader);
		RenderAllMeshes();
		if (m_shouldRenderJoints) {
			m_config.m_renderer.BindTexture(nullptr);
			m_config.m_renderer.BindShader(m_boneAnimShader);
			RenderAllJoints();
			m_jointGizmosManager.Render();
		}
	}
}

bool FBXModel::IsAnimPlayerClockPaused() const
{
	return m_animManager->m_animClock.IsPaused();
}

bool FBXModel::IsAnimationMode() const
{
	return m_animManager->IsActive();
}

void FBXModel::SetIsAnimationMode(bool isAnimationMode)
{
	if (m_animManager->IsActive() != isAnimationMode) {
		for (int i = 0; i < m_meshes.size(); i++) {
			if (m_meshes[i]) {
				m_meshes[i]->GetDDMModifierCPU()->SetNeedsRecalculation();
			}
			else {
				ERROR_AND_DIE("FBXModel::m_meshes[i] == nullptr");
			}
		}
	}

	if (isAnimationMode && m_animManager->m_animClock.IsPaused()) {
		m_animManager->m_animClock.Unpause();
	}
}

bool FBXModel::IsRootMotionXYFixed() const
{
	return m_isRootMotionXYFixed;
}

void FBXModel::SetIsRootMotionXYFixed(bool isRootMotionXYFixed)
{
	m_isRootMotionXYFixed = isRootMotionXYFixed;
}

bool FBXModel::HasAnimationData() const
{
	return m_animManager->HasAnimationData();
}

unsigned int FBXModel::GetMaxJointStencilValue() const
{
	if (m_joints.back() == nullptr) {
		ERROR_AND_DIE("Your code is fucked up. nullptr in m_joints.");
	}
	return m_joints.back()->GetStencilRefForThisJoint();
}

FBXJoint* FBXModel::GetJointByStencilRef(unsigned int stencilValue)
{
	for (int i = 0; i < m_joints.size(); i++) {
		if (m_joints[i] && m_joints[i]->GetStencilRefForThisJoint() == stencilValue) {
			return m_joints[i];
		}
	}
	return nullptr;
}

int FBXModel::GetNumJoints() const
{
	return (int)m_joints.size();
}

void FBXModel::AddDDMModifer()
{
	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			m_meshes[i]->AddDDMModifier();
		}
	}
}

void FBXModel::PrecomputeDDM(bool isCPUSide, bool useCotangentLaplacian, int numLaplacianIterations, float lambda, float kappa, float alpha)
{
	FBXDDMPrecomputeConstants newPrecomputeConstants = { isCPUSide, useCotangentLaplacian, numLaplacianIterations, lambda, kappa, alpha };
	//If we already did this precomputation before... than just ignore
	bool areAllDDMModifiersPrecomputed = true;

	for (auto mesh : m_meshes) {
		if (isCPUSide) {
			areAllDDMModifiersPrecomputed &= mesh->GetDDMModifierCPU()->IsPrecomputed();
		}
		else {
			areAllDDMModifiersPrecomputed &= mesh->GetDDMModifierGPU()->IsPrecomputed();
		}
		if (areAllDDMModifiersPrecomputed == false)
			break;
	}

	if ((newPrecomputeConstants == m_latestPrecomputeConstants) && areAllDDMModifiersPrecomputed) {
		SetDDMNeedsRecalculation();
		return;
	}

	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			m_meshes[i]->PrecomputeDDM(isCPUSide, useCotangentLaplacian, numLaplacianIterations, lambda, kappa, alpha);
		}
	}
	m_latestPrecomputeConstants = newPrecomputeConstants;
	SetDDMNeedsRecalculation();
}

/*
void FBXModel::ToggleDDM_CPU()
{
	if (m_isDDMPrecomputed == false) {
		ERROR_AND_DIE("You can't toggle DDM_CPU on and off without precomputing!");
	}
	m_isUsingDDMCPU = !m_isUsingDDMCPU;
	if (m_isUsingDDMCPU == false) {
		for (int i = 0; i < m_meshes.size(); i++) {
			m_meshes[i]->RestoreGPUVerticesToRestPose();
		}
	}
}
*/

void FBXModel::ToggleJointVisibility()
{
	m_shouldRenderJoints = !m_shouldRenderJoints;
}

void FBXModel::SetJointVisibility(bool areJointsVisible)
{
	m_shouldRenderJoints = areJointsVisible;
}

void FBXModel::ToggleRigidBinding()
{
	m_shouldUseRigidBinding = !m_shouldUseRigidBinding;

	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			m_meshes[i]->GetDDMModifierCPU()->SetNeedsRecalculation();
		}
		else {
			ERROR_AND_DIE("FBXModel::m_meshes[i] == nullptr");
		}
	}
}

std::string FBXModel::GetFileName() const
{
	return m_fileName;
}

void FBXModel::SetSkinningModifierState(FBXModelSkinningModifier modifier)
{
	if (m_skinningModifier == modifier)
		return;

	m_skinningModifier = modifier;
	for (int i = 0; i < m_meshes.size(); i++) {
		m_meshes[i]->UpdateFBXMeshCBOConstants(m_config.m_renderer);
		if (m_skinningModifier == FBXModelSkinningModifier::LBS) {
			m_meshes[i]->RestoreGPUVerticesToRestPose();
		}
	}
}

FBXModelSkinningModifier FBXModel::GetSkinningModifierState() const
{
	return m_skinningModifier;
}

bool FBXModel::GetJointVisibility() const
{
	return m_shouldRenderJoints;
}

FBXJoint* FBXModel::GetSelectedJoint() const
{
	return m_jointGizmosManager.GetSelectedJoint();
}

/*
unsigned int FBXModel::GetKeyframeIndexToPlayBasedOnElapsedTime() const
{
	float elapsedTime = m_animPlayerClock.GetTotalSeconds();
	int newFrameIndex = 0;
	switch (m_animTimeMode) {
	case FbxTime::EMode::eFrames24:
		newFrameIndex = int(elapsedTime * 24.0f);
		break;
	case FbxTime::EMode::eFrames30:
		newFrameIndex = int(elapsedTime * 30.0f);
		break;
	case FbxTime::EMode::eFrames60:
		newFrameIndex = int(elapsedTime * 60.0f);
		break;
	default:
		ERROR_AND_DIE("Only 24fps, 30fps, 60fps supported so far!");
	}
	return newFrameIndex;
}
*/

unsigned int FBXModel::GetAnimFPS() const
{
	return m_animManager->GetAnimFPS();
}

int FBXModel::GetIndexOfJoint(const FBXJoint& joint) const
{
	for (int i = 0; i < m_joints.size(); i++) {
		if (m_joints[i] == &joint) {
			return i;
		}
	}
	return -1;
}

const FBXJoint& FBXModel::GetRootJoint() const
{
	return *m_joints[0];
}

void FBXModel::SetIsIKOn(bool isIKOn)
{
	m_isIKOn = isIKOn;
}

FBXModel* FBXModel::CreateCopy(const Vec3& offset) const
{
	FBXModel* copiedModel = new FBXModel(m_config, m_fileName);
	//TODO: Deep copiedModel m_meshes and m_joints
	copiedModel->m_joints.reserve(m_joints.size());
	copiedModel->m_meshes.reserve(m_meshes.size());
	for (FBXJoint* joint : m_joints) {
		FBXJoint* copiedJoint = joint->CreateCopy();
		copiedJoint->SetFBXModel(*copiedModel);
		copiedModel->m_joints.push_back(copiedJoint);
	}
	for (FBXMesh* mesh : m_meshes) {
		FBXMesh* copiedMesh = mesh->CreateCopy();
		copiedMesh->SetFBXModel(*copiedModel);
		copiedModel->m_meshes.push_back(copiedMesh);
	}
	//copiedModel->m_animManager = m_animManager->CreateCopy();

	//Set the child-parent relationships of joints for the copiedModel
	for (int jointIdx = 0; jointIdx < m_joints.size(); jointIdx++) {
		for (int childIdx = 0; childIdx < m_joints[jointIdx]->GetNumChildJoints(); childIdx++) {
			unsigned int childJointStencilRef = m_joints[jointIdx]->GetChildJointByIdx(childIdx)->GetStencilRefForThisJoint();
			FBXJoint* copysChildJoint = copiedModel->GetJointByStencilRef(childJointStencilRef);
			GUARANTEE_OR_DIE(copysChildJoint != nullptr, "Did you copiedModel stencil ref?");
			copiedModel->m_joints[jointIdx]->AddChildJoints(*copysChildJoint);
		}
	}

	copiedModel->m_animManager = m_animManager->CreateCopy();
	std::vector<FBXPose>& poses = copiedModel->m_animManager->GetFBXPoseSequence();
	for (FBXPose& pose : poses) {
		pose.SetRootJoint(*copiedModel->m_joints[0]);
	}

	copiedModel->Startup();
	copiedModel->m_joints[0]->SetLocalDeltaTranslate(offset);

	for (FBXMesh* copiedMesh : copiedModel->m_meshes) {
		copiedMesh->RestoreGPUVerticesToRestPose();
	}

	return copiedModel;
}

int FBXModel::GetNumFaces() const
{
	int numFaces = 0;
	for (int i = 0; i < m_meshes.size(); i++) {
		numFaces += (int)m_meshes[i]->GetFacesMatrix().rows();
	}
	return numFaces;
}

void FBXModel::RenderAllMeshes() const
{
	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			m_meshes[i]->Render(m_config.m_renderer);
		}
	}
}

int FBXModel::GetNumVertices() const
{
	int numVertices = 0;
	for (int i = 0; i < m_meshes.size(); i++) {
		numVertices += (int)m_meshes[i]->GetNumVertices();
	}
	return numVertices;
}

void FBXModel::SetDDMNeedsRecalculation()
{
	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			std::shared_ptr<FBXDDMModifierCPU> ddmModifierCPU = m_meshes[i]->GetDDMModifierCPU();
			if (ddmModifierCPU) {
				ddmModifierCPU->SetNeedsRecalculation();
			}
			else {
				ERROR_AND_DIE("Mesh should have FBXDDMModifierCPU!");
			}
			std::shared_ptr<FBXDDMModifierGPU> ddmModifierGPU = m_meshes[i]->GetDDMModifierGPU();
			if (ddmModifierGPU) {
				ddmModifierGPU->SetNeedsRecalculation();
			}
			else {
				ERROR_AND_DIE("Mesh should have FBXDDMModifierCPU!");
			}
		}
		else {
			ERROR_AND_DIE("FBXModel::m_meshes[i] can't be nullptr");
		}
	}
}

void FBXModel::ToggleMeshDebugMode()
{
	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			m_meshes[i]->ToggleMeshDebugMode();
		}
	}
}

void FBXModel::InitiateDDMBaking(FBXParser& fbxParser, const std::string& exportFileName, int numPoses, int numMaxBones, float twistLimit, float pruneThreshold)
{
	GUARANTEE_OR_DIE(m_skinningModifier != FBXModelSkinningModifier::LBS, "m_skinningModifier is LBS");
	GUARANTEE_OR_DIE(numPoses > 0, "numPoses <= 0");
	GUARANTEE_OR_DIE(numMaxBones > 0, "numMaxBones <= 0");
	GUARANTEE_OR_DIE(twistLimit > 0.0f, "twistLimit <= 0.0f");
	GUARANTEE_OR_DIE(pruneThreshold >= 0.0f, "pruneThreshold < 0.0f");

	g_theJobSystem->WaitUntilAllJobsCompleted();

	FBXDDMBakingJob* bakingJob = new FBXDDMBakingJob(*this, fbxParser, exportFileName, numPoses, numMaxBones, twistLimit, pruneThreshold);
	g_theJobSystem->PostNewJob(bakingJob);
	m_isBakingInProgress = true;
	m_animManager->SetIsActive(false);
	m_jointGizmosManager.SetSelectedJoint(nullptr);
	m_preBakingSkinningModifier = m_skinningModifier;
	SetSkinningModifierState(FBXModelSkinningModifier::LBS);
	/*
	RandomNumberGenerator rng;
	int numJoints = (int)m_joints.size();

	for (int meshIdx = 0; meshIdx < m_meshes.size(); meshIdx++) {
		GUARANTEE_OR_DIE(m_meshes[meshIdx] != nullptr, "FBXModel::m_meshes[i] == nullptr");
		m_meshes[meshIdx]->PrepareDDMBaker(numPoses);
	}

	std::vector<Mat44> jointLocalDeltasBeforeBaking;
	jointLocalDeltasBeforeBaking.reserve((size_t)numJoints);
	for (int jointIdx = 0; jointIdx < numJoints; jointIdx++) {
		jointLocalDeltasBeforeBaking.push_back(m_joints[jointIdx]->GetLocalDeltaTransform());
	}

	for (int poseIdx = 0; poseIdx < numPoses; poseIdx++) {
		for (int jointIdx = 0; jointIdx < numJoints; jointIdx++) {
			Mat44 randomDeltaTransform = Mat44::CreateZRotationDegrees(rng.RollRandomFloatInRange(-twistLimit, twistLimit));
			randomDeltaTransform.Append(Mat44::CreateXRotationDegrees(rng.RollRandomFloatInRange(-twistLimit, twistLimit)));
			randomDeltaTransform.Append(Mat44::CreateYRotationDegrees(rng.RollRandomFloatInRange(-twistLimit, twistLimit)));
			m_joints[jointIdx]->SetLocalDeltaTransform(randomDeltaTransform);
		}
		m_joints[0]->RecursivelyUpdateGlobalTransformBindPoseForThisFrame(Mat44());
		std::vector<Mat44> allJointSkinningMatrices;
		for (int i = 0; i < m_joints.size(); i++) {
			if (m_joints[i]) {
				allJointSkinningMatrices.emplace_back(m_joints[i]->GetSkinningMatrixForThisFrame());
			}
		}

		for (int meshIdx = 0; meshIdx < m_meshes.size(); meshIdx++) {
			GUARANTEE_OR_DIE(m_meshes[meshIdx] != nullptr, "FBXModel::m_meshes[i] == nullptr");
			m_meshes[meshIdx]->FillLHSMatForDDMBaker(allJointSkinningMatrices, poseIdx);
			m_meshes[meshIdx]->FillRHSMatForDDMBaker(allJointSkinningMatrices, poseIdx);
		}
	}

	for (int meshIdx = 0; meshIdx < m_meshes.size(); meshIdx++) {
		GUARANTEE_OR_DIE(m_meshes[meshIdx] != nullptr, "FBXModel::m_meshes[i] == nullptr");
		m_meshes[meshIdx]->SolveDDMBakerLinearSystems(numMaxBones, pruneThreshold);
		m_meshes[meshIdx]->UpdateSceneSkinningData();
	}
	*/

	/*
	//restore the deltas
	for (int jointIdx = 0; jointIdx < numJoints; jointIdx++) {
		m_joints[jointIdx]->SetLocalDeltaTransform(jointLocalDeltasBeforeBaking[jointIdx]);
	}
	*/
}

const std::vector<FBXJoint*>& FBXModel::GetJointsArray() const
{
	return m_joints;
}

void FBXModel::RenderAllJoints() const
{
	for (int i = 0; i < m_joints.size(); i++) {
		if (m_joints[i]) {
			m_joints[i]->Render(m_config.m_renderer);
		}
	}
}

void FBXModel::UpdateJointGlobalTransformsStructuredBuffer()
{
	std::vector<Mat44> keys = GetJointGlobalTransformList();
	m_config.m_renderer.CopyCPUToGPU(keys.data(), keys.size() * sizeof(Mat44), unsigned int(sizeof(Mat44)), (unsigned int)keys.size(), m_sboForJointGlobalTransforms);
}

void FBXModel::ApplyDDMv0_CPU()
{
	std::vector<Mat44> allJointSkinningMatrices;
	for (int i = 0; i < m_joints.size(); i++) {
		if (m_joints[i]) {
			allJointSkinningMatrices.emplace_back(m_joints[i]->GetSkinningMatrixForThisFrame());
		}
	}

	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			m_meshes[i]->ApplyDDMv0_CPU(allJointSkinningMatrices);
		}
	}
}

bool FBXModel::IsBakingInProgress() const
{
	return m_isBakingInProgress;
}

/*
void FBXModel::UpdateJointsKeyFrameAnimation(const std::vector<FBXJoint*>& newJoints)
{
	GUARANTEE_OR_DIE(newJoints.size() == m_joints.size(), "newJoints.size() != m_joints.size()");
	for (int jointIdx = 0; jointIdx < newJoints.size(); jointIdx++) {
		if (m_joints[jointIdx] && newJoints[jointIdx]) {
			m_joints[jointIdx]->ClearLocalKeyFrames();
			for (unsigned int keyFrameIdx = 0; keyFrameIdx < newJoints[jointIdx]->GetNumLocalKeyFrames(); keyFrameIdx++) {
				m_joints[jointIdx]->AddLocalKeyFrame(newJoints[jointIdx]->GetLocalKeyFrame(keyFrameIdx));
			}
		}
		else {
			ERROR_AND_DIE("nullptr in joints vector");
		}
	}
}
*/

void FBXModel::SetSelectedJointOfGizmoFromNameIfValid(const std::string& jointName)
{
	FBXJoint* newSelectedJoint = nullptr;
	for (int i = 0; i < m_joints.size(); i++) {
		if (m_joints[i] && m_joints[i]->GetName() == jointName) {
			newSelectedJoint = m_joints[i];
			break;
		}
	}
	if (newSelectedJoint)
		m_jointGizmosManager.SetSelectedJoint(newSelectedJoint);
}

void FBXModel::SetAnimClockTotalSeconds(float newTotalSeconds)
{
	m_animManager->m_animClock.SetTotalSeconds(newTotalSeconds);
}

float FBXModel::GetAnimTimeSpan() const
{
	return m_animManager->GetAnimTimeSpan();
}

Clock& FBXModel::GetRefToAnimPlayerClock()
{
	return m_animManager->m_animClock;
}

void FBXModel::ApplyDDMv1_CPU()
{
	std::vector<Mat44> allJointSkinningMatrices;
	for (int i = 0; i < m_joints.size(); i++) {
		if (m_joints[i]) {
			allJointSkinningMatrices.emplace_back(m_joints[i]->GetSkinningMatrixForThisFrame());
		}
	}

	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			m_meshes[i]->ApplyDDMv1_CPU(allJointSkinningMatrices);
		}
	}
}

void FBXModel::ApplyDDMv0_GPU()
{
	std::vector<Mat44> allJointSkinningMatrices;
	for (int i = 0; i < m_joints.size(); i++) {
		if (m_joints[i]) {
			allJointSkinningMatrices.emplace_back(m_joints[i]->GetSkinningMatrixForThisFrame());
		}
	}

	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			m_meshes[i]->ApplyDDMv0_GPU(allJointSkinningMatrices);
		}
	}
}

void FBXModel::ApplyDDMv1_GPU()
{
	std::vector<Mat44> allJointSkinningMatrices;
	for (int i = 0; i < m_joints.size(); i++) {
		if (m_joints[i]) {
			allJointSkinningMatrices.emplace_back(m_joints[i]->GetSkinningMatrixForThisFrame());
		}
	}

	for (int i = 0; i < m_meshes.size(); i++) {
		if (m_meshes[i]) {
			m_meshes[i]->ApplyDDMv1_GPU(allJointSkinningMatrices);
		}
	}
}

std::vector<Mat44> FBXModel::GetJointGlobalBindPoseInverseList() const
{
	std::vector<Mat44> list;
	for (int i = 0; i < m_joints.size(); i++) {
		if (m_joints[i]) {
			list.emplace_back(m_joints[i]->GetGlobalBindPoseInverse());
		}
		else {
			ERROR_AND_DIE("m_joints should NOT have a nullptr element");
		}
	}
	return list;
}

std::vector<Mat44> FBXModel::GetJointGlobalTransformList() const
{
	std::vector<Mat44> list;
	for (int i = 0; i < m_joints.size(); i++) {
		if (m_joints[i]) {
			list.emplace_back(m_joints[i]->GetGlobalTransformForThisFrame());
		}
		else {
			ERROR_AND_DIE("m_joints should NOT have a nullptr element");
		}
	}
	return list;
}
