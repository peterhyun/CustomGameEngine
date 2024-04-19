#ifndef CUDA_MATRIXMATH_FUNCTIONS
#define CUDA_MATRIXMATH_FUNCTIONS
#define tolerance 1e-3

__device__ __forceinline__
void mult4x4(const double* matA, const double* matB, double* matC) {
	// Initialize the result matrix to zeros
	for (int i = 0; i < 16; i++) {
		matC[i] = 0.0;
	}
	// Perform matrix multiplication
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			for (int k = 0; k < 4; k++) {
				matC[i * 4 + j] += matA[i * 4 + k] * matB[k * 4 + j];
			}
		}
	}
	/*
	matC[0] = matA[0] * matB[0] + matA[1] * matB[4] + matA[2] * matB[8] + matA[3] * matB[12];
	matC[1] = matA[0] * matB[1] + matA[1] * matB[5] + matA[2] * matB[9] + matA[3] * matB[13];
	matC[2] = matA[0] * matB[2] + matA[1] * matB[6] + matA[2] * matB[10] + matA[3] * matB[14];
	matC[3] = matA[0] * matB[3] + matA[1] * matB[7] + matA[2] * matB[11] + matA[3] * matB[15];

	matC[4] = matA[4] * matB[0] + matA[5] * matB[4] + matA[6] * matB[8] + matA[7] * matB[12];
	matC[5] = matA[4] * matB[1] + matA[5] * matB[5] + matA[6] * matB[9] + matA[7] * matB[13];
	matC[6] = matA[4] * matB[2] + matA[5] * matB[6] + matA[6] * matB[10] + matA[7] * matB[14];
	matC[7] = matA[4] * matB[3] + matA[5] * matB[7] + matA[6] * matB[11] + matA[7] * matB[15];

	matC[8] = matA[8] * matB[0] + matA[9] * matB[4] + matA[10] * matB[8] + matA[11] * matB[12];
	matC[9] = matA[8] * matB[1] + matA[9] * matB[5] + matA[10] * matB[9] + matA[11] * matB[13];
	matC[10] = matA[8] * matB[2] + matA[9] * matB[6] + matA[10] * matB[10] + matA[11] * matB[14];
	matC[11] = matA[8] * matB[3] + matA[9] * matB[7] + matA[10] * matB[11] + matA[11] * matB[15];

	matC[12] = matA[12] * matB[0] + matA[13] * matB[4] + matA[14] * matB[8] + matA[15] * matB[12];
	matC[13] = matA[12] * matB[1] + matA[13] * matB[5] + matA[14] * matB[9] + matA[15] * matB[13];
	matC[14] = matA[12] * matB[2] + matA[13] * matB[6] + matA[14] * matB[10] + matA[15] * matB[14];
	matC[15] = matA[12] * matB[3] + matA[13] * matB[7] + matA[14] * matB[11] + matA[15] * matB[15];
	*/
}

__device__ __forceinline__
void mult3x3(const float* matA, const float* matB, float* matC) {
	// Initialize the result matrix to zeros
	for (int i = 0; i < 9; i++) {
		matC[i] = 0.0f;
	}

	// Perform matrix multiplication
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				matC[i * 3 + j] += matA[i * 3 + k] * matB[k * 3 + j];
			}
		}
	}
}

__device__ __forceinline__
void mult3x3(const double* matA, const double* matB, double* matC) {
	// Initialize the result matrix to zeros
	for (int i = 0; i < 9; i++) {
		matC[i] = 0.0;
	}

	// Perform matrix multiplication
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (int k = 0; k < 3; k++) {
				matC[i * 3 + j] += matA[i * 3 + k] * matB[k * 3 + j];
			}
		}
	}
}

__device__ __forceinline__
void add4x4(const double* matA, const double* matB, double* matC) {
	for (int i = 0; i < 16; i++) {
		matC[i] = matA[i] + matB[i];
	}
}

__device__ __forceinline__
void add3x3(const double* matA, const double* matB, double* matC) {
	for (int i = 0; i < 9; i++) {
		matC[i] = matA[i] + matB[i];
	}
}

__device__ __forceinline__
void subtract3x3(const double* matA, const double* matB, double* matC) {
	for (int i = 0; i < 9; i++) {
		matC[i] = matA[i] - matB[i];
	}
}

__device__ __forceinline__
void subtract3x1(const float* matA, const float* matB, float* matC) {
	for (int i = 0; i < 3; i++) {
		matC[i] = matA[i] - matB[i];
	}
}

__device__ __forceinline__
void subtract3x1(const double* matA, const double* matB, double* matC) {
	for (int i = 0; i < 3; i++) {
		matC[i] = matA[i] - matB[i];
	}
}

__device__ __forceinline__
void mult3x3With3x1(const float* mat3x3, const float* mat3x1, float* out_mat3x1) {
	out_mat3x1[0] = mat3x3[0] * mat3x1[0] + mat3x3[1] * mat3x1[1] + mat3x3[2] * mat3x1[2];
	out_mat3x1[1] = mat3x3[3] * mat3x1[0] + mat3x3[4] * mat3x1[1] + mat3x3[5] * mat3x1[2];
	out_mat3x1[2] = mat3x3[6] * mat3x1[0] + mat3x3[7] * mat3x1[1] + mat3x3[8] * mat3x1[2];
}

__device__ __forceinline__
void mult3x3With3x1(const double* mat3x3, const double* mat3x1, double* out_mat3x1) {
	out_mat3x1[0] = mat3x3[0] * mat3x1[0] + mat3x3[1] * mat3x1[1] + mat3x3[2] * mat3x1[2];
	out_mat3x1[1] = mat3x3[3] * mat3x1[0] + mat3x3[4] * mat3x1[1] + mat3x3[5] * mat3x1[2];
	out_mat3x1[2] = mat3x3[6] * mat3x1[0] + mat3x3[7] * mat3x1[1] + mat3x3[8] * mat3x1[2];
}

__device__ __forceinline__
void mult4x4With4x1(const float* mat4x4, const float* mat4x1, float* out_mat4x1) {
	out_mat4x1[0] = mat4x4[0] * mat4x1[0] + mat4x4[1] * mat4x1[1] + mat4x4[2] * mat4x1[2] + mat4x4[3] * mat4x1[3];
	out_mat4x1[1] = mat4x4[4] * mat4x1[0] + mat4x4[5] * mat4x1[1] + mat4x4[6] * mat4x1[2] + mat4x4[7] * mat4x1[3];
	out_mat4x1[2] = mat4x4[8] * mat4x1[0] + mat4x4[9] * mat4x1[1] + mat4x4[10] * mat4x1[2] + mat4x4[11] * mat4x1[3];
	out_mat4x1[3] = mat4x4[12] * mat4x1[0] + mat4x4[13] * mat4x1[1] + mat4x4[14] * mat4x1[2] + mat4x4[15] * mat4x1[3];
}

__device__ __forceinline__
void mult4x4With4x1(const double* mat4x4, const double* mat4x1, double* out_mat4x1) {
	out_mat4x1[0] = mat4x4[0] * mat4x1[0] + mat4x4[1] * mat4x1[1] + mat4x4[2] * mat4x1[2] + mat4x4[3] * mat4x1[3];
	out_mat4x1[1] = mat4x4[4] * mat4x1[0] + mat4x4[5] * mat4x1[1] + mat4x4[6] * mat4x1[2] + mat4x4[7] * mat4x1[3];
	out_mat4x1[2] = mat4x4[8] * mat4x1[0] + mat4x4[9] * mat4x1[1] + mat4x4[10] * mat4x1[2] + mat4x4[11] * mat4x1[3];
	out_mat4x1[3] = mat4x4[12] * mat4x1[0] + mat4x4[13] * mat4x1[1] + mat4x4[14] * mat4x1[2] + mat4x4[15] * mat4x1[3];
}

__device__ __forceinline__
void getLeftTop3x3From4x4(const double* mat4x4, double* mat3x3) {
	mat3x3[0] = mat4x4[0]; mat3x3[1] = mat4x4[1]; mat3x3[2] = mat4x4[2];
	mat3x3[3] = mat4x4[4]; mat3x3[4] = mat4x4[5]; mat3x3[5] = mat4x4[6];
	mat3x3[6] = mat4x4[8]; mat3x3[7] = mat4x4[9]; mat3x3[8] = mat4x4[10];
}

__device__ __forceinline__
void mult3x1With1x3(const double* mat3x1, const double* mat1x3, double* mat3x3) {
	mat3x3[0] = mat3x1[0] * mat1x3[0]; mat3x3[1] = mat3x1[0] * mat1x3[1]; mat3x3[2] = mat3x1[0] * mat1x3[2];
	mat3x3[3] = mat3x1[1] * mat1x3[0]; mat3x3[4] = mat3x1[1] * mat1x3[1]; mat3x3[5] = mat3x1[1] * mat1x3[2];
	mat3x3[6] = mat3x1[2] * mat1x3[0]; mat3x3[7] = mat3x1[2] * mat1x3[1]; mat3x3[8] = mat3x1[2] * mat1x3[2];
}

__device__ __forceinline__
double getDeterminantOf3x3(const double* m3x3) {
	return m3x3[0] * (m3x3[4] * m3x3[8] - m3x3[7] * m3x3[5]) -
		m3x3[1] * (m3x3[3] * m3x3[8] - m3x3[5] * m3x3[6]) +
		m3x3[2] * (m3x3[3] * m3x3[7] - m3x3[4] * m3x3[6]);
}

__device__ __forceinline__
float absf(float input) {
	if (input >= 0)
		return input;
	else
		return -input;
}

__device__ __forceinline__
void transpose3x3(const float* in_m3x3, float* out_m3x3) {
	out_m3x3[0] = in_m3x3[0];	out_m3x3[1] = in_m3x3[3];	out_m3x3[2] = in_m3x3[6];
	out_m3x3[3] = in_m3x3[1];	out_m3x3[4] = in_m3x3[4];	out_m3x3[5] = in_m3x3[7];
	out_m3x3[6] = in_m3x3[2];	out_m3x3[7] = in_m3x3[5];	out_m3x3[8] = in_m3x3[8];
}

__device__ __forceinline__
void transpose3x3(const double* in_m3x3, double* out_m3x3) {
	out_m3x3[0] = in_m3x3[0];	out_m3x3[1] = in_m3x3[3];	out_m3x3[2] = in_m3x3[6];
	out_m3x3[3] = in_m3x3[1];	out_m3x3[4] = in_m3x3[4];	out_m3x3[5] = in_m3x3[7];
	out_m3x3[6] = in_m3x3[2];	out_m3x3[7] = in_m3x3[5];	out_m3x3[8] = in_m3x3[8];
}

__device__ __forceinline__
void calcInvOfInversableMat3x3(const double* in_m3x3, double* out_m3x3) {
	double det = getDeterminantOf3x3(in_m3x3);
	double inv_det = 1.0 / det;
	out_m3x3[0] = (in_m3x3[4] * in_m3x3[8] - in_m3x3[7] * in_m3x3[5]) * inv_det;
	out_m3x3[1] = (in_m3x3[2] * in_m3x3[7] - in_m3x3[1] * in_m3x3[8]) * inv_det;
	out_m3x3[2] = (in_m3x3[1] * in_m3x3[5] - in_m3x3[2] * in_m3x3[4]) * inv_det;
	out_m3x3[3] = (in_m3x3[5] * in_m3x3[6] - in_m3x3[3] * in_m3x3[8]) * inv_det;
	out_m3x3[4] = (in_m3x3[0] * in_m3x3[8] - in_m3x3[2] * in_m3x3[6]) * inv_det;
	out_m3x3[5] = (in_m3x3[3] * in_m3x3[2] - in_m3x3[0] * in_m3x3[5]) * inv_det;
	out_m3x3[6] = (in_m3x3[3] * in_m3x3[7] - in_m3x3[6] * in_m3x3[4]) * inv_det;
	out_m3x3[7] = (in_m3x3[6] * in_m3x3[1] - in_m3x3[0] * in_m3x3[7]) * inv_det;
	out_m3x3[8] = (in_m3x3[0] * in_m3x3[4] - in_m3x3[3] * in_m3x3[1]) * inv_det;
}

__device__ __forceinline__
void calcInvMat3x3(const double* in_m3x3, double* out_m3x3) {
	double det = getDeterminantOf3x3(in_m3x3);
	if (det > tolerance) {
		calcInvOfInversableMat3x3(in_m3x3, out_m3x3);
	}
	else {
		//Using Moore-Penrose pseudoinverse
		double A_t[9];
		transpose3x3(in_m3x3, A_t);
		double A_t_x_A[9];
		mult3x3(A_t, in_m3x3, A_t_x_A);
		det = getDeterminantOf3x3(A_t_x_A);
		
		if (det > tolerance) {
			double A_t_x_A_inv[9];
			calcInvOfInversableMat3x3(A_t_x_A, A_t_x_A_inv);
			mult3x3(A_t_x_A_inv, A_t, out_m3x3);
		}
		else {
			double reg = tolerance;
			double A_reged[9];
			int maxWhileLoopCounter = 0;
			do {
				double scaledI[9] = {
					reg, 0.0, 0.0,
					0.0, reg, 0.0,
					0.0, 0.0, reg
				};
				add3x3(scaledI, in_m3x3, A_reged);
				reg = reg + tolerance;
				det = getDeterminantOf3x3(A_reged);
				maxWhileLoopCounter = maxWhileLoopCounter + 1;
			} while (det <= tolerance && maxWhileLoopCounter < 1000);
			calcInvOfInversableMat3x3(A_reged, out_m3x3);
		}
		/*
		out_m3x3[0] = 1.0f;
		out_m3x3[1] = 0.0f;
		out_m3x3[2] = 0.0f;
		out_m3x3[3] = 0.0f;
		out_m3x3[4] = 1.0f;
		out_m3x3[5] = 0.0f;
		out_m3x3[6] = 0.0f;
		out_m3x3[7] = 0.0f;
		out_m3x3[8] = 1.0f;
		*/
	}
}

#endif