#ifndef DDMV0_H
#define DDMV0_H

void ComputeV0_CUDA(const float* allJointTransforms, int numThreadGroup, int numThreadsInGroup, int numJoints, int numControlPoints, const double* omegaMatrixGPU, const double* controlPointsMatrixGPU, float* outControlPointsMatrixGPU);

#endif