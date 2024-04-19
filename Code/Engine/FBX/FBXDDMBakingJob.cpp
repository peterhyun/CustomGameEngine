#include "Engine/Fbx/FBXDDMBakingJob.hpp"
#include "Engine/Fbx/FBXModel.hpp"
#include "Engine/Fbx/FBxParser.hpp"
#include "Engine/Fbx/FBXMesh.hpp"
#include "Engine/Fbx/FBXJoint.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

FBXDDMBakingJob::FBXDDMBakingJob(FBXModel& model, FBXParser& parser, const std::string& exportFileName, int numPoses, int numMaxBones, float twistLimit, float pruneThreshold)
	: m_model(model), m_parser(parser), m_exportFileName(exportFileName), m_numPoses(numPoses), m_numMaxBones(numMaxBones), m_twistLimit(twistLimit), m_pruneThreshold(pruneThreshold)
{
}

void FBXDDMBakingJob::Execute()
{
	RandomNumberGenerator rng;
	int numJoints = (int)m_model.GetNumJoints();

	for (int meshIdx = 0; meshIdx < m_model.m_meshes.size(); meshIdx++) {
		GUARANTEE_OR_DIE(m_model.m_meshes[meshIdx] != nullptr, "FBXModel::m_meshes[i] == nullptr");
		m_model.m_meshes[meshIdx]->PrepareDDMBaker(m_numPoses);
	}

	//m_model.m_joints[0]->ResetLocalDeltaTranslate();
	m_model.m_joints[0]->ResetLocalDeltaRotate();
	for (int poseIdx = 0; poseIdx < m_numPoses; poseIdx++) {
		for (int jointIdx = 1; jointIdx < numJoints; jointIdx++) {
			Quaternion randomDeltaRotate = Quaternion::CreateFromAxisAndDegrees(rng.RollRandomFloatInRange(-m_twistLimit, m_twistLimit), Vec3(0.0f, 0.0f, 1.0f));
			randomDeltaRotate = randomDeltaRotate * Quaternion::CreateFromAxisAndDegrees(rng.RollRandomFloatInRange(-m_twistLimit, m_twistLimit), Vec3(1.0f, 0.0f, 0.0f));
			randomDeltaRotate = randomDeltaRotate * Quaternion::CreateFromAxisAndDegrees(rng.RollRandomFloatInRange(-m_twistLimit, m_twistLimit), Vec3(0.0f, 1.0f, 0.0f));
			m_model.m_joints[jointIdx]->SetLocalDeltaRotate(randomDeltaRotate);
		}
		m_model.m_joints[0]->RecursivelyUpdateGlobalTransformBindPoseForThisFrame(Mat44());
		std::vector<Mat44> allJointSkinningMatrices;
		for (int i = 0; i < m_model.m_joints.size(); i++) {
			if (m_model.m_joints[i]) {
				allJointSkinningMatrices.emplace_back(m_model.m_joints[i]->GetSkinningMatrixForThisFrame());
			}
		}

		for (int meshIdx = 0; meshIdx < m_model.m_meshes.size(); meshIdx++) {
			GUARANTEE_OR_DIE(m_model.m_meshes[meshIdx] != nullptr, "FBXModel::m_meshes[i] == nullptr");
			m_model.m_meshes[meshIdx]->FillLHSMatForDDMBaker(allJointSkinningMatrices, poseIdx);
			m_model.m_meshes[meshIdx]->FillRHSMatForDDMBaker(allJointSkinningMatrices, poseIdx);
		}
	}

	for (int meshIdx = 0; meshIdx < m_model.m_meshes.size(); meshIdx++) {
		GUARANTEE_OR_DIE(m_model.m_meshes[meshIdx] != nullptr, "FBXModel::m_meshes[i] == nullptr");
		m_model.m_meshes[meshIdx]->SolveDDMBakerLinearSystems(m_numMaxBones, m_pruneThreshold);
		m_model.m_meshes[meshIdx]->UpdateSceneSkinningData();
	}

	m_parser.ExportParsedFile(m_exportFileName);
}

void FBXDDMBakingJob::OnComplete()
{
}
