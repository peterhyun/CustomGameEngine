#include "Engine/FBX/CudaFiles/DDMV0.cuh"
#include "Engine/FBX/CudaFiles/FastSVD3.cuh"
#include "Engine/FBX/CudaFiles/CudaMatrixMathFunctions.cuh"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <cuda.h>

__global__
void ComputeV0Global(const float* allJointTransforms, int numJoints, int numControlPoints, const double* omegaMatrix, const double* controlPointsMatrix, float* outControlPointsMatrix) {
	int cpIdx = blockDim.x * blockIdx.x + threadIdx.x;
	if (cpIdx >= numControlPoints) {
		return;
	}

	double QMatrix_i[16] = {
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0
	};

	for (int jointIdx = 0; jointIdx < numJoints; jointIdx++) {
		int omegaStartIdx = (10 * numJoints) * cpIdx + jointIdx * 10;
		double symMat[16] = {
			omegaMatrix[omegaStartIdx + 0], omegaMatrix[omegaStartIdx + 1], omegaMatrix[omegaStartIdx + 2], omegaMatrix[omegaStartIdx + 3],
			omegaMatrix[omegaStartIdx + 1], omegaMatrix[omegaStartIdx + 4], omegaMatrix[omegaStartIdx + 5], omegaMatrix[omegaStartIdx + 6],
			omegaMatrix[omegaStartIdx + 2], omegaMatrix[omegaStartIdx + 5], omegaMatrix[omegaStartIdx + 7], omegaMatrix[omegaStartIdx + 8],
			omegaMatrix[omegaStartIdx + 3], omegaMatrix[omegaStartIdx + 6], omegaMatrix[omegaStartIdx + 8], omegaMatrix[omegaStartIdx + 9]
		};

		int jointTransformStartIdx = 16 * jointIdx;
		double jointTransform[16] = {
			(double)allJointTransforms[jointTransformStartIdx + 0], (double)allJointTransforms[jointTransformStartIdx + 4], (double)allJointTransforms[jointTransformStartIdx + 8],  (double)allJointTransforms[jointTransformStartIdx + 12],
			(double)allJointTransforms[jointTransformStartIdx + 1], (double)allJointTransforms[jointTransformStartIdx + 5], (double)allJointTransforms[jointTransformStartIdx + 9],  (double)allJointTransforms[jointTransformStartIdx + 13],
			(double)allJointTransforms[jointTransformStartIdx + 2], (double)allJointTransforms[jointTransformStartIdx + 6], (double)allJointTransforms[jointTransformStartIdx + 10], (double)allJointTransforms[jointTransformStartIdx + 14],
			(double)allJointTransforms[jointTransformStartIdx + 3], (double)allJointTransforms[jointTransformStartIdx + 7], (double)allJointTransforms[jointTransformStartIdx + 11], (double)allJointTransforms[jointTransformStartIdx + 15]
		};
		/*
		double jointTransform[16] = {
			(double)allJointTransforms[jointTransformStartIdx + 0], (double)allJointTransforms[jointTransformStartIdx + 1], (double)allJointTransforms[jointTransformStartIdx + 2],  (double)allJointTransforms[jointTransformStartIdx + 3],
			(double)allJointTransforms[jointTransformStartIdx + 4], (double)allJointTransforms[jointTransformStartIdx + 5], (double)allJointTransforms[jointTransformStartIdx + 6],  (double)allJointTransforms[jointTransformStartIdx + 7],
			(double)allJointTransforms[jointTransformStartIdx + 8], (double)allJointTransforms[jointTransformStartIdx + 9], (double)allJointTransforms[jointTransformStartIdx + 10], (double)allJointTransforms[jointTransformStartIdx + 11],
			(double)allJointTransforms[jointTransformStartIdx + 12], (double)allJointTransforms[jointTransformStartIdx + 13], (double)allJointTransforms[jointTransformStartIdx + 14], (double)allJointTransforms[jointTransformStartIdx + 15]
		};
		*/

		double productMat[16] = {
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0
		};

		mult4x4(jointTransform, symMat, productMat);
		add4x4(QMatrix_i, productMat, QMatrix_i);
	}

	for (int i = 0; i < 16; i++) {
		QMatrix_i[i] /= QMatrix_i[15];
	}

	double Q_i[9];
	getLeftTop3x3From4x4(QMatrix_i, Q_i);
	double q_i[3] = {QMatrix_i[3], QMatrix_i[7], QMatrix_i[11]};
	double p_i[3] = {QMatrix_i[12], QMatrix_i[13], QMatrix_i[14]};
	double q_i_mult_p_i[9];
	mult3x1With1x3(q_i, p_i, q_i_mult_p_i);
	
	double U_S_Vt[9];
	subtract3x3(Q_i, q_i_mult_p_i, U_S_Vt);

	float U[9];
	float S[9];
	float V[9];

	svd((float)U_S_Vt[0], (float)U_S_Vt[1], (float)U_S_Vt[2],
		(float)U_S_Vt[3], (float)U_S_Vt[4], (float)U_S_Vt[5],
		(float)U_S_Vt[6], (float)U_S_Vt[7], (float)U_S_Vt[8],
		U[0], U[1], U[2],
		U[3], U[4], U[5],
		U[6], U[7], U[8],
		S[0], S[1], S[2],
		S[3], S[4], S[5],
		S[6], S[7], S[8],
		V[0], V[1], V[2],
		V[3], V[4], V[5],
		V[6], V[7], V[8]
	);

	float V_t[9] = {
		V[0], V[3], V[6],
		V[1], V[4], V[7],
		V[2], V[5], V[8]
	};

	float R_i[9];
	mult3x3(U, V_t, R_i);

	float R_i_mult_p_i[3];
	float p_i_float[3] = {(float)p_i[0], (float)p_i[1], (float)p_i[2]};
	mult3x3With3x1(R_i, p_i_float, R_i_mult_p_i);
	float t_i[3];
	float q_i_float[3] = {(float)q_i[0], (float)q_i[1], (float)q_i[2]};
	subtract3x1(q_i_float, R_i_mult_p_i, t_i);

	float gamma_i[16] = {
		R_i[0], R_i[1], R_i[2], t_i[0],
		R_i[3], R_i[4], R_i[5], t_i[1],
		R_i[6], R_i[7], R_i[8], t_i[2],
		0.0f, 0.0f, 0.0f, 1.0f
	};

	/*
	float gamma_i[16] = {
		1.0f, 0.0f, 0.0f, 10.0f,
		0.0f, 1.0f, 0.0f, 10.0f,
		0.0f, 0.0f, 1.0f, 10.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	*/

	int controlPointsStartIdx = cpIdx * 3;
	float affineCurrentControlPoint[4] = {
		(float)controlPointsMatrix[controlPointsStartIdx], (float)controlPointsMatrix[controlPointsStartIdx + 1], (float)controlPointsMatrix[controlPointsStartIdx + 2], 1.0f
	};

	float transformedAffineCP[4];
	mult4x4With4x1(gamma_i, affineCurrentControlPoint, transformedAffineCP);

	outControlPointsMatrix[controlPointsStartIdx]	  = transformedAffineCP[0];
	outControlPointsMatrix[controlPointsStartIdx + 1] = transformedAffineCP[1];
	outControlPointsMatrix[controlPointsStartIdx + 2] = transformedAffineCP[2];
}

void ComputeV0_CUDA(const float* allJointTransforms, int numThreadGroup, int numThreadsInGroup, int numJoints, int numControlPoints, const double* omegaMatrixGPU, const double* controlPointsMatrixGPU, float* outControlPointsMatrixGPU) {
	cudaEvent_t start, stop;
	float milliseconds = 0;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	// Record the start event
	cudaEventRecord(start, 0);
	ComputeV0Global<<<numThreadGroup, numThreadsInGroup>>>(allJointTransforms, numJoints, numControlPoints, omegaMatrixGPU, controlPointsMatrixGPU, outControlPointsMatrixGPU);
	// Record the stop event
	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);

	// Calculate the elapsed time
	cudaEventElapsedTime(&milliseconds, start, stop); 
	
	DebuggerPrintf("DDMV0 Kernel execution time: %f milliseconds\n", milliseconds);

	// Clean up
	cudaEventDestroy(start);
	cudaEventDestroy(stop);
}