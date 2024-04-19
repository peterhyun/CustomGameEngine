#include "Engine/FBX/CudaFiles/DDMV1.cuh"
#include "Engine/FBX/CudaFiles/CudaMatrixMathFunctions.cuh"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <cuda.h>

__global__
void ComputeV1Global(const float* allJointTransforms, int numJoints, int numControlPoints, const double* omegaMatrix, const double* controlPointsMatrix, const double* ddmV1Constants, float* outControlPointsMatrix) {
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
	double q_i[3] = { QMatrix_i[3], QMatrix_i[7], QMatrix_i[11] };
	double p_i[3] = { QMatrix_i[12], QMatrix_i[13], QMatrix_i[14] };
	double q_i_mult_p_i[9];
	mult3x1With1x3(q_i, p_i, q_i_mult_p_i);

	double Q_qp[9];
	subtract3x3(Q_i, q_i_mult_p_i, Q_qp);

	double determinant = getDeterminantOf3x3(Q_qp);
	double Q_qp_t[9];
	transpose3x3(Q_qp, Q_qp_t);

	double Q_qp_t_inv[9];
	calcInvMat3x3(Q_qp_t, Q_qp_t_inv);

	double Q_qpDet_x_Q_qp_t_inv[9] = {
		determinant * Q_qp_t_inv[0], determinant * Q_qp_t_inv[1], determinant * Q_qp_t_inv[2],
		determinant * Q_qp_t_inv[3], determinant * Q_qp_t_inv[4], determinant * Q_qp_t_inv[5],
		determinant * Q_qp_t_inv[6], determinant * Q_qp_t_inv[7], determinant * Q_qp_t_inv[8]
	};

	int symMatStartIdx = 6 * cpIdx;
	double v1ConstSymMat[9] = {
		ddmV1Constants[symMatStartIdx + 0],		ddmV1Constants[symMatStartIdx + 1], ddmV1Constants[symMatStartIdx + 2],
		ddmV1Constants[symMatStartIdx + 1], ddmV1Constants[symMatStartIdx + 3], ddmV1Constants[symMatStartIdx + 4],
		ddmV1Constants[symMatStartIdx + 2], ddmV1Constants[symMatStartIdx + 4], ddmV1Constants[symMatStartIdx + 5]
	};

	double R_i[9];
	mult3x3(Q_qpDet_x_Q_qp_t_inv, v1ConstSymMat, R_i);

	double R_i_mult_p_i[3];
	mult3x3With3x1(R_i, p_i, R_i_mult_p_i);
	double t_i[3];
	subtract3x1(q_i, R_i_mult_p_i, t_i);

	double gamma_i[16] = {
		R_i[0], R_i[1], R_i[2], t_i[0],
		R_i[3], R_i[4], R_i[5], t_i[1],
		R_i[6], R_i[7], R_i[8], t_i[2],
		0.0, 0.0, 0.0, 1.0
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
	double affineCurrentControlPoint[4] = {
		controlPointsMatrix[controlPointsStartIdx], controlPointsMatrix[controlPointsStartIdx + 1], controlPointsMatrix[controlPointsStartIdx + 2], 1.0
	};

	double transformedAffineCP[4];
	mult4x4With4x1(gamma_i, affineCurrentControlPoint, transformedAffineCP);

	outControlPointsMatrix[controlPointsStartIdx] = (float)transformedAffineCP[0];
	outControlPointsMatrix[controlPointsStartIdx + 1] = (float)transformedAffineCP[1];
	outControlPointsMatrix[controlPointsStartIdx + 2] = (float)transformedAffineCP[2];
}

void ComputeV1_CUDA(const float* allJointTransforms, int numThreadGroup, int numThreadsInGroup, int numJoints, int numControlPoints, const double* omegaMatrixGPU, const double* controlPointsMatrixGPU, const double* ddmV1Constants, float* outControlPointsMatrixGPU) {
	cudaEvent_t start, stop;
	float milliseconds = 0;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	// Record the start event
	cudaEventRecord(start, 0);
	ComputeV1Global << <numThreadGroup, numThreadsInGroup >> > (allJointTransforms, numJoints, numControlPoints, omegaMatrixGPU, controlPointsMatrixGPU, ddmV1Constants, outControlPointsMatrixGPU);
	// Record the stop event
	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);

	// Calculate the elapsed time
	cudaEventElapsedTime(&milliseconds, start, stop);

	DebuggerPrintf("DDMV1 Kernel execution time: %f milliseconds\n", milliseconds);

	// Clean up
	cudaEventDestroy(start);
	cudaEventDestroy(stop);
}