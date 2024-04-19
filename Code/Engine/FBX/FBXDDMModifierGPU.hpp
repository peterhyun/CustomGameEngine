#pragma once
#include "Engine/Math/Mat44.hpp"
#include "Engine/Fbx/FBXDDMModifier.hpp"
#include <Eigen/Sparse>
#include <Eigen/Dense>
#include <vector>
#include <cuda_runtime.h>

class Renderer;
class FBXMesh;
class StructuredBuffer;
class ConstantBuffer;
class ComputeOutputBuffer;
class ComputeShader;
class Shader;

struct DDMConstants {
	int m_numControlPoints = 0;
	int m_numJoints = 0;
	int m_numCPsToVerticesMapMatrixWidth = 0;
	int m_padding = 0;
};

class FBXDDMModifierGPU : public FBXDDMModifier{
	friend class FBXMesh;
public:
	FBXDDMModifierGPU(FBXMesh& mesh);
	~FBXDDMModifierGPU();

	virtual void Precompute(bool useCotangentLaplacian, int numLaplacianIterations, double lambda, double kappa, double alpha) override;
	
	virtual Eigen::MatrixX3f GetVariantv0Deform(const std::vector<Mat44>& allJointTransforms, bool& recalculatedThisFrame) override;
	virtual Eigen::MatrixX3f GetVariantv1Deform(const std::vector<Mat44>& allJointTransforms, bool& recalculatedThisFrame) override;

	virtual Eigen::MatrixX3f GetVariantv0DeformAlwaysCalculated(const std::vector<Mat44>& allJointTransforms);

private:
	static void CudaErrorCheck(cudaError_t callCudaFunctionHere);

private:
	double* m_omegaMatrixGPU = nullptr;
	double* m_controlPointsMatrixRestPoseGPU = nullptr;
	double* m_v1ConstantMatrixGPU = nullptr;
	float* m_deformedControlPointsGPU = nullptr;
};