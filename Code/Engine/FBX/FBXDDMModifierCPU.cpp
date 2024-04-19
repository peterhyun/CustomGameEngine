#include "Engine/Fbx/FBXDDMModifierCPU.hpp"
#include "Engine/Fbx/FBXMesh.hpp"
#include "Engine/Fbx/FBXDDMOmegaPrecomputeJob.hpp"
#include "Engine/Fbx/FBXDDMV0CPUJob.hpp"
#include "Engine/Fbx/FBXDDMV1CPUJob.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "ThirdParty/igl/cotmatrix.h"
#include "ThirdParty/igl/adjacency_matrix.h"
#include "ThirdParty/igl/sum.h"
#include "ThirdParty/igl/direct_delta_mush.h"
#include <Eigen/SparseCholesky>
#include <Eigen/SVD>

FBXDDMModifierCPU::FBXDDMModifierCPU(FBXMesh& mesh) : FBXDDMModifier(mesh)
{
}

FBXDDMModifierCPU::~FBXDDMModifierCPU()
{
}

Eigen::MatrixX3f FBXDDMModifierCPU::GetVariantv0Deform(const std::vector<Mat44>& allJointTransforms, bool& recalculatedThisFrame)
{
	if (m_needsRecalculation == false) {
		recalculatedThisFrame = false;
		return m_deformedControlPoints;
	}

	if (m_isPrecomputed == false) {
		ERROR_AND_DIE("Have to precompute first!");
	}
	if (allJointTransforms.size() != static_cast<size_t>(m_numJoints)) {
		ERROR_AND_DIE("allJointTransforms.size() != m_numJoints!");
	}

	//DebugAddMessage(Stringf("NumQueuedJobs: %d, NumClaimedJobs: %d, NumCompletedJobs: %d", g_theJobSystem->GetNumQueuedJobs(), g_theJobSystem->GetNumClaimedJobs(), g_theJobSystem->GetNumCompletedJobs()), 1.0f);
	
	std::vector<Eigen::Matrix<double, 4, 4>> allJointTransformsEigen;
	for (int i = 0; i < allJointTransforms.size(); i++) {
		allJointTransformsEigen.push_back(ConvertMat44ToEigen(allJointTransforms[i]));
	}

	int numControlPoints = static_cast<int>(m_controlPointsMatrixRestPose.rows());

	float beforeDDMTime = (float)GetCurrentTimeSeconds();
	//m_omegaMatrix <- This was a n x 10m matrix. '10' cause the matrix is symmetrical
	for (int ctrlPointIdx = 0; ctrlPointIdx < numControlPoints; ctrlPointIdx++) {
		g_theJobSystem->PostNewJob(new FBXDDMV0CPUJob(*this, ctrlPointIdx, allJointTransformsEigen));
	}
	
	while (g_theJobSystem->GetNumCompletedJobs() != numControlPoints) {} // Wait until all jobs complete
	
	float afterDDMTime = (float)GetCurrentTimeSeconds();

	DebuggerPrintf("Mesh: %s\n", m_mesh.GetName().c_str());
	DebuggerPrintf("DDMV0 CPU time: %f\n", afterDDMTime - beforeDDMTime);
	
	while (true) {
		Job* completedJob = g_theJobSystem->GetCompletedJob();
		if (completedJob) {
			FBXDDMV0CPUJob* v0CPUJob = dynamic_cast<FBXDDMV0CPUJob*>(completedJob);
			if (v0CPUJob) {
				m_deformedControlPoints.row(v0CPUJob->m_ctrlPointIdx) = v0CPUJob->m_deformedControlPoint;
				delete v0CPUJob;
			}
			else {
				ERROR_AND_DIE("Doesn't make sense...");
			}
		}
		else {
			break;
		}
	}

	m_needsRecalculation = false;
	recalculatedThisFrame = true;

	return m_deformedControlPoints;
}

Eigen::MatrixX3f FBXDDMModifierCPU::GetVariantv1Deform(const std::vector<Mat44>& allJointTransforms, bool& recalculatedThisFrame)
{
	if (m_needsRecalculation == false) {
		recalculatedThisFrame = false;
		return m_deformedControlPoints;
	}

	if (m_isPrecomputed == false) {
		ERROR_AND_DIE("Have to precompute first!");
	}
	if (allJointTransforms.size() != static_cast<size_t>(m_numJoints)) {
		ERROR_AND_DIE("allJointTransforms.size() != m_numJoints!");
	}
	std::vector<Eigen::Matrix<double, 4, 4>> allJointTransformsEigen;
	for (int i = 0; i < allJointTransforms.size(); i++) {
		allJointTransformsEigen.push_back(ConvertMat44ToEigen(allJointTransforms[i]));
	}

	int numControlPoints = static_cast<int>(m_controlPointsMatrixRestPose.rows());

	m_deformedControlPoints.resize(numControlPoints, 3);
	m_deformedControlPoints.setZero();

	/*
	Eigen::MatrixX3d deformedControlPoints(numControlPoints, 3);
	deformedControlPoints.setZero();

	Eigen::MatrixX4d affineControlPointsMatrix(numControlPoints, 4);
	affineControlPointsMatrix.leftCols(3) = m_controlPointsMatrixRestPose;
	affineControlPointsMatrix.rightCols(1).setOnes();
	*/
	float beforeDDMTime = (float)GetCurrentTimeSeconds();

	//m_omegaMatrix <- This was a n x 10m matrix. '10' cause the matrix is symmetrical
	for (int ctrlPointIdx = 0; ctrlPointIdx < numControlPoints; ctrlPointIdx++) {
		g_theJobSystem->PostNewJob(new FBXDDMV1CPUJob(*this, ctrlPointIdx, allJointTransformsEigen));
	}

	while (g_theJobSystem->GetNumCompletedJobs() != numControlPoints) {} // Wait until all jobs complete

	float afterDDMTime = (float)GetCurrentTimeSeconds();

	DebuggerPrintf("Mesh: %s\n", m_mesh.GetName().c_str());
	DebuggerPrintf("DDMV1 CPU time: %f\n", afterDDMTime - beforeDDMTime);

	while (true) {
		Job* completedJob = g_theJobSystem->GetCompletedJob();
		if (completedJob) {
			FBXDDMV1CPUJob* v1CPUJob = dynamic_cast<FBXDDMV1CPUJob*>(completedJob);
			if (v1CPUJob) {
				m_deformedControlPoints.row(v1CPUJob->m_ctrlPointIdx) = v1CPUJob->m_deformedControlPoint;
				delete v1CPUJob;
			}
			else {
				ERROR_AND_DIE("Doesn't make sense...");
			}
		}
		else {
			break;
		}
	}

	m_needsRecalculation = false;
	recalculatedThisFrame = true;

	return m_deformedControlPoints;
}