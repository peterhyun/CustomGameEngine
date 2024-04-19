#pragma once
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include <Eigen/Sparse>
#include <Eigen/Dense>
#include <vector>

class FBXMesh;
class Shader;
class Renderer;

class FBXDDMModifier {
public:
	FBXDDMModifier(FBXMesh& mesh);
	virtual ~FBXDDMModifier();

	virtual void Precompute(bool useCotangentLaplacian, int numLaplacianIterations, double lambda, double kappa, double alpha);
	virtual bool IsPrecomputed() const final;
	void ResetIsPrecomputed();

	virtual Eigen::Index GetNumJoints() const final;
	virtual const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>& GetConstRefToOmegaMatrix() const final;
	virtual const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>& GetConstRefToV1ConstantMatrix() const final;
	virtual const Eigen::Matrix<double, 1, 3> GetControlPoint(unsigned int index) const final;
	virtual void SetNeedsRecalculation() final;

	virtual Eigen::MatrixX3f GetVariantv0Deform(const std::vector<Mat44>& allJointTransforms, bool& recalculatedThisFrame) = 0;
	virtual Eigen::MatrixX3f GetVariantv1Deform(const std::vector<Mat44>& allJointTransforms, bool& recalculatedThisFrame) = 0;

	//static helper functions
	static Eigen::VectorXd GetUpperTriangleOfSymmetric4x4Matrix(const Eigen::Matrix<double, 4, 4>& matrix);
	static Eigen::VectorXd GetUpperTriangleOfSymmetric3x3Matrix(const Eigen::Matrix<double, 3, 3>& matrix);
	static Eigen::Matrix<double, 4, 4> ConvertMat44ToEigen(const Mat44& mat);
	static Eigen::Matrix<double, 4, 4> GetSymmetricMatrix4x4From10Floats(const Eigen::Matrix<double, 1, 10>& floats);
	static Eigen::Matrix<double, 3, 3> GetSymmetricMatrix3x3From6Floats(const Eigen::Matrix<double, 1, 6>& floats);
	template<typename Scalar>
	static bool DoesMatrixHaveNans(const Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>& inquiryMatrix);
	static bool DoesMatrixHaveNans(const Eigen::SparseMatrix<double>& inquiryMatrix);
	template<typename Scalar>
	static void DebugPrintEigenMatrix(const Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>& inquiryMatrix);

protected:
	FBXMesh& m_mesh;
	const Eigen::MatrixX3d m_controlPointsMatrixRestPose;
	Eigen::SparseMatrix<double> m_normalizedLaplacian;
	Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> m_omegaMatrix;
	Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> m_v1ConstantMatrix; //This is (P_i - p_i*p_i^T)/det(P_i - p_i*p_i^T)
	bool m_isPrecomputed = false;
	int m_numJoints = 0;

	const Eigen::IOFormat m_debugPrintFmt = Eigen::IOFormat(2, 0, "\t", "\n", "", "", "", "");

	bool m_needsRecalculation = true;
	Eigen::MatrixX3f m_deformedControlPoints;
};

template<typename Scalar>
bool FBXDDMModifier::DoesMatrixHaveNans(const Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>& inquiryMatrix)
{
	Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic> nanMask = inquiryMatrix.array().isNaN();
	// Check if there are any NaN values in the matrix.
	return nanMask.any();
}

template<typename Scalar>
void FBXDDMModifier::DebugPrintEigenMatrix(const Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>& inquiryMatrix)
{
	std::stringstream ss;
	for (int i = 0; i < inquiryMatrix.rows(); ++i) {
		for (int j = 0; j < inquiryMatrix.cols(); ++j) {
			ss << inquiryMatrix(i, j) << "\t";
		}
		ss << "\n";
	}
	DebuggerPrintf(Stringf("%s\n", ss.str().c_str()).c_str());
}