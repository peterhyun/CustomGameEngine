#ifndef DDMV1_H
#define DDMV1_H

void ComputeV1_CUDA(const float* allJointTransforms, int numThreadGroup, int numThreadsInGroup, int numJoints, int numControlPoints, const double* omegaMatrixGPU, const double* controlPointsMatrixGPU, const double* ddmV1ConstantsGPU, float* outControlPointsMatrixGPU);

#endif