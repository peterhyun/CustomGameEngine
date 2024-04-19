#include "Engine/Fbx/FBXDDMModifierGPU.hpp"
#include "Engine/Fbx/FBXMesh.hpp"
#include "Engine/Fbx/FBXDDMOmegaPrecomputeJob.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/ComputeOutputBuffer.hpp"
#include "Engine/Renderer/StructuredBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/FBX/CudaFiles/DDMV0.cuh"
#include "Engine/FBX/CudaFiles/DDMV1.cuh"
#include "ThirdParty/igl/cotmatrix.h"
#include "ThirdParty/igl/adjacency_matrix.h"
#include "ThirdParty/igl/sum.h"
#include "ThirdParty/igl/direct_delta_mush.h"
#include <Eigen/SparseCholesky>
#include <Eigen/SVD>

FBXDDMModifierGPU::FBXDDMModifierGPU(FBXMesh& mesh) : FBXDDMModifier(mesh)
{
	CudaErrorCheck(cudaMallocManaged((void**)&m_controlPointsMatrixRestPoseGPU, m_controlPointsMatrixRestPose.size() * sizeof(double)));

	Eigen::MatrixXd controlPointsMatrixRestPoseTranspose = m_controlPointsMatrixRestPose.transpose();
	CudaErrorCheck(cudaMemcpy(m_controlPointsMatrixRestPoseGPU, controlPointsMatrixRestPoseTranspose.data(), controlPointsMatrixRestPoseTranspose.size() * sizeof(double), cudaMemcpyKind::cudaMemcpyHostToDevice));

	CudaErrorCheck(cudaMallocManaged((void**)&m_deformedControlPointsGPU, m_controlPointsMatrixRestPose.size() * sizeof(float)));
}

FBXDDMModifierGPU::~FBXDDMModifierGPU()
{
	CudaErrorCheck(cudaFree(m_v1ConstantMatrixGPU));
	CudaErrorCheck(cudaFree(m_omegaMatrixGPU));
	CudaErrorCheck(cudaFree(m_deformedControlPointsGPU));
	CudaErrorCheck(cudaFree(m_controlPointsMatrixRestPoseGPU));
}

void FBXDDMModifierGPU::Precompute(bool useCotangentLaplacian, int numLaplacianIterations, double lambda, double kappa, double alpha)
{
	FBXDDMModifier::Precompute(useCotangentLaplacian, numLaplacianIterations, lambda, kappa, alpha);

	CudaErrorCheck(cudaMallocManaged((void**)&m_omegaMatrixGPU, m_omegaMatrix.size() * sizeof(double)));

	Eigen::MatrixXd omegaMatrixTranspose = m_omegaMatrix.transpose();
	CudaErrorCheck(cudaMemcpy(m_omegaMatrixGPU, omegaMatrixTranspose.data(), omegaMatrixTranspose.size() * sizeof(double), cudaMemcpyKind::cudaMemcpyHostToDevice));

	CudaErrorCheck(cudaMallocManaged((void**)&m_v1ConstantMatrixGPU, m_v1ConstantMatrix.size() * sizeof(double)));

	Eigen::MatrixXd v1ConstantMatrixTranspose = m_v1ConstantMatrix.transpose();
	CudaErrorCheck(cudaMemcpy(m_v1ConstantMatrixGPU, v1ConstantMatrixTranspose.data(), v1ConstantMatrixTranspose.size() * sizeof(double), cudaMemcpyKind::cudaMemcpyHostToDevice));
}

Eigen::MatrixX3f FBXDDMModifierGPU::GetVariantv0Deform(const std::vector<Mat44>& allJointTransforms, bool& recalculatedThisFrame)
{
	//Disable this for renderdoc debug
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

	//
	float* allJointTransformsGPU = nullptr;
	CudaErrorCheck(cudaMallocManaged((void**)&allJointTransformsGPU, allJointTransforms.size() * sizeof(Mat44)));
	CudaErrorCheck(cudaMemcpy(allJointTransformsGPU, allJointTransforms.data(), allJointTransforms.size() * sizeof(Mat44), cudaMemcpyKind::cudaMemcpyHostToDevice));

	DebuggerPrintf("Mesh: %s\n", m_mesh.GetName().c_str());
	ComputeV0_CUDA(allJointTransformsGPU, 256, 256, m_mesh.GetNumJoints(), (int)m_mesh.GetControlPointsMatrixRestPose().rows(), m_omegaMatrixGPU, m_controlPointsMatrixRestPoseGPU, m_deformedControlPointsGPU);
	cudaDeviceSynchronize();

	CudaErrorCheck(cudaFree(allJointTransformsGPU));

	Eigen::Matrix3Xf deformedControlPointsTranspose;
	deformedControlPointsTranspose.resize(3, m_controlPointsMatrixRestPose.rows());
	//CudaErrorCheck(cudaMemcpy(deformedControlPointsTranspose.data(), m_deformedControlPointsGPU, deformedControlPointsTranspose.size() * sizeof(float), cudaMemcpyKind::cudaMemcpyDeviceToHost));
	deformedControlPointsTranspose = Eigen::Map<Eigen::MatrixXf>(m_deformedControlPointsGPU, 3, m_controlPointsMatrixRestPose.rows());

	m_deformedControlPoints = deformedControlPointsTranspose.transpose();
	m_needsRecalculation = false;

	recalculatedThisFrame = true;
	return m_deformedControlPoints;
}

Eigen::MatrixX3f FBXDDMModifierGPU::GetVariantv1Deform(const std::vector<Mat44>& allJointTransforms, bool& recalculatedThisFrame)
{
	//Disable this for renderdoc debug
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

	//
	float* allJointTransformsGPU = nullptr;
	CudaErrorCheck(cudaMallocManaged((void**)&allJointTransformsGPU, allJointTransforms.size() * sizeof(Mat44)));
	CudaErrorCheck(cudaMemcpy(allJointTransformsGPU, allJointTransforms.data(), allJointTransforms.size() * sizeof(Mat44), cudaMemcpyKind::cudaMemcpyHostToDevice));

	DebuggerPrintf("Mesh: %s\n", m_mesh.GetName().c_str());
	ComputeV1_CUDA(allJointTransformsGPU, 256, 256, m_mesh.GetNumJoints(), (int)m_mesh.GetControlPointsMatrixRestPose().rows(), m_omegaMatrixGPU, m_controlPointsMatrixRestPoseGPU, m_v1ConstantMatrixGPU, m_deformedControlPointsGPU);
	cudaDeviceSynchronize();

	CudaErrorCheck(cudaFree(allJointTransformsGPU));

	Eigen::Matrix3Xf deformedControlPointsTranspose;
	deformedControlPointsTranspose.resize(3, m_controlPointsMatrixRestPose.rows());
	//CudaErrorCheck(cudaMemcpy(deformedControlPointsTranspose.data(), m_deformedControlPointsGPU, deformedControlPointsTranspose.size() * sizeof(float), cudaMemcpyKind::cudaMemcpyDeviceToHost));
	deformedControlPointsTranspose = Eigen::Map<Eigen::MatrixXf>(m_deformedControlPointsGPU, 3, m_controlPointsMatrixRestPose.rows());

	m_deformedControlPoints = deformedControlPointsTranspose.transpose();
	m_needsRecalculation = false;
	recalculatedThisFrame = true;

	return m_deformedControlPoints;
}

Eigen::MatrixX3f FBXDDMModifierGPU::GetVariantv0DeformAlwaysCalculated(const std::vector<Mat44>& allJointTransforms)
{
	if (m_isPrecomputed == false) {
		ERROR_AND_DIE("Have to precompute first!");
	}
	if (allJointTransforms.size() != static_cast<size_t>(m_numJoints)) {
		ERROR_AND_DIE("allJointTransforms.size() != m_numJoints!");
	}

	//
	float* allJointTransformsGPU = nullptr;
	CudaErrorCheck(cudaMallocManaged((void**)&allJointTransformsGPU, allJointTransforms.size() * sizeof(Mat44)));
	CudaErrorCheck(cudaMemcpy(allJointTransformsGPU, allJointTransforms.data(), allJointTransforms.size() * sizeof(Mat44), cudaMemcpyKind::cudaMemcpyHostToDevice));

	ComputeV0_CUDA(allJointTransformsGPU, 256, 256, m_mesh.GetNumJoints(), (int)m_mesh.GetControlPointsMatrixRestPose().rows(), m_omegaMatrixGPU, m_controlPointsMatrixRestPoseGPU, m_deformedControlPointsGPU);
	cudaDeviceSynchronize();

	CudaErrorCheck(cudaFree(allJointTransformsGPU));

	Eigen::Matrix3Xf deformedControlPointsTranspose;
	deformedControlPointsTranspose.resize(3, m_controlPointsMatrixRestPose.rows());
	//CudaErrorCheck(cudaMemcpy(deformedControlPointsTranspose.data(), m_deformedControlPointsGPU, deformedControlPointsTranspose.size() * sizeof(float), cudaMemcpyKind::cudaMemcpyDeviceToHost));
	deformedControlPointsTranspose = Eigen::Map<Eigen::MatrixXf>(m_deformedControlPointsGPU, 3, m_controlPointsMatrixRestPose.rows());

	m_deformedControlPoints = deformedControlPointsTranspose.transpose();
	return m_deformedControlPoints;
}

void FBXDDMModifierGPU::CudaErrorCheck(cudaError_t callCudaFunctionHere)
{
	if (callCudaFunctionHere != cudaSuccess) {
		ERROR_AND_DIE(Stringf("cudaErr: %d", callCudaFunctionHere));
	}
}