#include "Engine/Fbx/FBXDDMModifier.hpp"
#include "Engine/Fbx/FBXMesh.hpp"
#include "Engine/Fbx/FBXDDMOmegaPrecomputeJob.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "ThirdParty/igl/cotmatrix.h"
#include "ThirdParty/igl/adjacency_matrix.h"
#include "ThirdParty/igl/sum.h"
#include <Eigen/SparseCholesky>
#include <Eigen/SVD>

FBXDDMModifier::FBXDDMModifier(FBXMesh& mesh) : m_mesh(mesh), m_controlPointsMatrixRestPose(mesh.GetControlPointsMatrixRestPose()), m_numJoints(mesh.GetNumJoints())
{
	int numControlPoints = static_cast<int>(mesh.GetControlPointsMatrixRestPose().rows());
	m_deformedControlPoints.resize(numControlPoints, 3);
	m_deformedControlPoints.setZero();
}

FBXDDMModifier::~FBXDDMModifier()
{
}

void FBXDDMModifier::Precompute(bool useCotangentLaplacian, int numLaplacianIterations, double lambda, double kappa, double alpha)
{
	if (numLaplacianIterations <= 0) {
		ERROR_AND_DIE(Stringf("numLaplacianIterations should be positive! Input %d", numLaplacianIterations));
	}
	if (lambda <= 0.0) {
		ERROR_AND_DIE(Stringf("Invalid lambda value %.3lf", lambda));
	}
	if (kappa < 0.0 || kappa >= lambda) {
		ERROR_AND_DIE(Stringf("Invalid kappa value %.3lf", kappa));
	}
	if (alpha < 0.0 || alpha > 1.0) {
		ERROR_AND_DIE(Stringf("Invalid alpha value %.3lf", alpha));
	}

	Eigen::Index numControlPoints = m_controlPointsMatrixRestPose.rows();

	Eigen::MatrixX3i facesMatrix = m_mesh.GetFacesMatrix();
	Eigen::MatrixXd weightsMatrix = m_mesh.GetWeightsMatrix();
	if (weightsMatrix.rows() != numControlPoints) {
		ERROR_AND_DIE("weightsMatrix #rows and m_controlPointsMatrixRestPose #rows should be the same");
	}

	Eigen::SparseMatrix<double> laplacian;
	laplacian.setZero();
	if (useCotangentLaplacian) {
		igl::cotmatrix(m_controlPointsMatrixRestPose, facesMatrix, laplacian);

		laplacian = -laplacian;	//Have to make it a positive semidefinite matrix! (igl makes it negative semidefinite)
		if (DoesMatrixHaveNans(laplacian)) {
			ERROR_RECOVERABLE("cotWeightedLaplacian has Nans. Using adjacency matrices instead!");

			//Don't use cotmatrix. Use adjacency matrices
			laplacian.setZero();
			igl::adjacency_matrix(facesMatrix, laplacian);
			Eigen::SparseVector<double> rowSum(laplacian.rows());
			rowSum.setZero();
			igl::sum(laplacian, 1, rowSum);

			Eigen::SparseMatrix<double> rowSumDiagonal(rowSum.size(), rowSum.size());
			rowSumDiagonal.setZero();
			for (int i = 0; i < rowSum.size(); ++i) {
				rowSumDiagonal.coeffRef(i, i) = rowSum.coeff(i);
			}
			laplacian = rowSumDiagonal - laplacian;
			if (DoesMatrixHaveNans(laplacian)) {
				ERROR_AND_DIE("Adjancency matrix based laplacian cannot have nans!");
			}
		}
	}
	else {
		igl::adjacency_matrix(facesMatrix, laplacian);
		Eigen::SparseVector<double> rowSum(laplacian.rows());
		rowSum.setZero();
		igl::sum(laplacian, 1, rowSum);

		Eigen::SparseMatrix<double> rowSumDiagonal(rowSum.size(), rowSum.size());
		rowSumDiagonal.setZero();
		for (int i = 0; i < rowSum.size(); ++i) {
			rowSumDiagonal.coeffRef(i, i) = rowSum.coeff(i);
		}
		laplacian = rowSumDiagonal - laplacian;
		if (DoesMatrixHaveNans(laplacian)) {
			ERROR_AND_DIE("Adjancency matrix based laplacian cannot have nans!");
		}
	}

	//Calculate L_bar
	double L_bar_startCalc = GetCurrentTimeSeconds();
	Eigen::SparseMatrix<double> laplDiagonalInv(laplacian.rows(), laplacian.cols());
	laplDiagonalInv.setZero();
	for (int k = 0; k < laplacian.outerSize(); ++k) {
		for (Eigen::SparseMatrix<double>::InnerIterator it(laplacian, k); it; ++it) {
			if ((it.row() == it.col()) && (it.value() != 0.0)) {
				laplDiagonalInv.insert(it.row(), it.col()) = 1.0 / it.value();
			}
		}
	}
	m_normalizedLaplacian = (laplacian * laplDiagonalInv);//.sparseView().eval();	//Order might be switched
	double L_bar_endCalc = GetCurrentTimeSeconds();
	if (DoesMatrixHaveNans(m_normalizedLaplacian)) {
		ERROR_AND_DIE("m_normalizedLaplacian has Nans");
	}
	DebuggerPrintf(Stringf("Laplacian Calc: %.3lf\n", L_bar_endCalc - L_bar_startCalc).c_str());

	Eigen::SparseMatrix<double> identity(m_normalizedLaplacian.rows(), m_normalizedLaplacian.cols());
	identity.setIdentity();

	//First calculating 12 rev! Two ways to do this. 1. Use iterative solvers to get approximate C = (I + kL) ^ (-p) or 2. Skip calculating C^(-p) directly and get the W' matrix iteratively
	//Trying the latter first, but definitely try the first method too
	double ldltSolverStartTime = GetCurrentTimeSeconds();
	Eigen::SparseMatrix<double> c = (identity + kappa * m_normalizedLaplacian);
	Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> ldltSolverC(c.transpose());
	Eigen::MatrixXd weightsPrimeMatrix(weightsMatrix);
	for (int iter = 0; iter < numLaplacianIterations; iter++) {
		weightsPrimeMatrix = ldltSolverC.solve(weightsPrimeMatrix);
	}
	double ldltSolverEndTime = GetCurrentTimeSeconds();
	if (DoesMatrixHaveNans(weightsPrimeMatrix)) {
		ERROR_AND_DIE("weightsPrimeMatrix has Nans");
	}
	DebuggerPrintf(Stringf("WeightsPrimeSolverTime: %.3lf\n", ldltSolverEndTime - ldltSolverStartTime).c_str());

	//Next is u_k * u_k^T (Gonna cache all the data)
	Eigen::MatrixX4d affineControlPointsMatrix(numControlPoints, 4);
	affineControlPointsMatrix.leftCols(3) = m_controlPointsMatrixRestPose;
	affineControlPointsMatrix.rightCols(1).setOnes();

	double UxUCachedStartTime = GetCurrentTimeSeconds();
	Eigen::Matrix<double, Eigen::Dynamic, 10> UxUCached(numControlPoints, 10);
	for (int ctrlPointIdx = 0; ctrlPointIdx < numControlPoints; ctrlPointIdx++) {
		const auto& affineCtrlPoint = affineControlPointsMatrix.row(ctrlPointIdx);
		Eigen::Matrix<double, 4, 4> u_k_x_u_k_transpose = (affineCtrlPoint.transpose() * affineCtrlPoint);
		Eigen::VectorXd u_k_x_u_k_transpose_extracted = GetUpperTriangleOfSymmetric4x4Matrix(u_k_x_u_k_transpose);
		UxUCached.row(ctrlPointIdx) = u_k_x_u_k_transpose_extracted;
	}
	double UxUCachedEndTime = GetCurrentTimeSeconds();
	if (DoesMatrixHaveNans<double>(UxUCached)) {
		ERROR_AND_DIE("UxUCached has Nans");
	}
	DebuggerPrintf(Stringf("UxUCachedCalcTime: %.3lf\n", UxUCachedEndTime - UxUCachedStartTime).c_str());

	double UxUMultipliedWithWStartTime = GetCurrentTimeSeconds();
	Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> UxUMultipliedWithW(numControlPoints, 10 * m_numJoints);
	for (int jointIndex = 0; jointIndex < m_numJoints; jointIndex++) {
		const auto& weightsMatrixColumn = weightsMatrix.col(jointIndex);
		for (int x = 0; x < 10; x++) {
			const auto& UxUColumn = UxUCached.col(x);
			UxUMultipliedWithW.col(10 * jointIndex + x) = weightsMatrixColumn.array() * UxUColumn.array();
		}
	}
	double UxUMultipliedWithWEndTime = GetCurrentTimeSeconds();
	if (DoesMatrixHaveNans(UxUMultipliedWithW)) {
		ERROR_AND_DIE("UxUMultipliedWithW has Nans");
	}
	DebuggerPrintf(Stringf("UxUMultipliedWithWCalcTime: %.3lf\n", UxUMultipliedWithWEndTime - UxUMultipliedWithWStartTime).c_str());

	double PsiMatrixStartTime = GetCurrentTimeSeconds();
	Eigen::SparseMatrix<double> PsiMatrix = UxUMultipliedWithW.sparseView();
	Eigen::SparseMatrix<double> b = (identity + lambda * m_normalizedLaplacian);
	Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> ldltSolverB(b.transpose());
	for (int iter = 0; iter < numLaplacianIterations; iter++) {
		PsiMatrix = ldltSolverB.solve(PsiMatrix);
	}
	double PsiMatrixEndTime = GetCurrentTimeSeconds();
	if (DoesMatrixHaveNans(PsiMatrix)) {
		ERROR_AND_DIE("PsiMatrix has Nans");
	}
	DebuggerPrintf(Stringf("PsiMatrixSolverTime: %.3lf\n", PsiMatrixEndTime - PsiMatrixStartTime).c_str());

	double PMatrixStartTime = GetCurrentTimeSeconds();
	Eigen::Matrix<double, Eigen::Dynamic, 10> PMatrix(numControlPoints, 10);	//This corresponds to the matrix [p_i * p_i^T, p_i // p_i^T, 1] in the paper
	m_v1ConstantMatrix.resize(numControlPoints, 6);
	m_v1ConstantMatrix.setZero();
	for (int ctrlPointIndex = 0; ctrlPointIndex < numControlPoints; ctrlPointIndex++) {
		Eigen::Matrix<double, 3, 1> p_i;
		p_i.setZero();

		Eigen::Matrix<double, 3, 3> P_i;
		P_i.setZero();

		double normalizerValue = 0.0;
		for (int jointIndex = 0; jointIndex < m_numJoints; jointIndex++) {
			p_i += Eigen::Matrix<double, 3, 1>(PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10 + 3), PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10 + 6), PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10 + 8));
			Eigen::Matrix<double, 3, 3> tempToAddToP_i;
			tempToAddToP_i << PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10), PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10 + 1), PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10 + 2),
				PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10 + 1), PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10 + 4), PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10 + 5),
				PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10 + 2), PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10 + 5), PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10 + 7);
			P_i += tempToAddToP_i;
			normalizerValue += PsiMatrix.coeff(ctrlPointIndex, jointIndex * 10 + 9);
		}
		if (normalizerValue == 0.0) {
			ERROR_AND_DIE("Have to deal with this situation where normalizerValue is 0.0f!");
		}
		p_i /= normalizerValue;
		P_i /= normalizerValue;

		Eigen::Matrix<double, 4, 4> p_i_matrix;
		p_i_matrix.block(0, 0, 3, 3) = p_i * p_i.transpose();
		p_i_matrix.block(0, 3, 3, 1) = p_i;
		p_i_matrix.block(3, 0, 1, 3) = p_i.transpose();
		p_i_matrix(3, 3) = 1.0;
		PMatrix.row(ctrlPointIndex) = GetUpperTriangleOfSymmetric4x4Matrix(p_i_matrix);

		//For variant1 precomputation...
		Eigen::Matrix<double, 3, 3> P_i_matrix = P_i;
		P_i_matrix = P_i_matrix - (p_i * p_i.transpose());
		if (DoesMatrixHaveNans<double>(P_i_matrix)) {
			ERROR_AND_DIE("P_i_matrix_preNormalization has Nans");
		}
		if (P_i_matrix.determinant() != 0.0) {
			P_i_matrix = P_i_matrix / P_i_matrix.determinant();
		}
		else {
			double regularization_constant = 1e-6;	//Adding a small regularization constant to avoid singularity
			P_i_matrix += P_i_matrix + regularization_constant * Eigen::Matrix<double, 3, 3>::Identity();
			if (P_i_matrix.determinant() == 0.0) {
				ERROR_AND_DIE("Need to add a bigger regularization constant!");
			}
			P_i_matrix = P_i_matrix / P_i_matrix.determinant();
		}
		if (DoesMatrixHaveNans<double>(P_i_matrix)) {
			ERROR_AND_DIE("P_i_matrix_postNormalization has Nans");
		}
		m_v1ConstantMatrix.row(ctrlPointIndex) = GetUpperTriangleOfSymmetric3x3Matrix(P_i_matrix);
	}
	double PMatrixEndTime = GetCurrentTimeSeconds();

	DebuggerPrintf(Stringf("PMatrixCalcTime: %.3lf\n", PMatrixEndTime - PMatrixStartTime).c_str());

	double omegaMatrixStartTime = GetCurrentTimeSeconds();
	m_omegaMatrix.resize(numControlPoints, m_numJoints * 10);
	m_omegaMatrix.setZero();
	for (int ctrlPointIdx = 0; ctrlPointIdx < numControlPoints; ctrlPointIdx++) {
		Eigen::Matrix<double, 1, 10> pRow = PMatrix.row(ctrlPointIdx);
		for (int jointIdx = 0; jointIdx < m_numJoints; jointIdx++) {
			g_theJobSystem->PostNewJob(new FBXDDMOmegaPrecomputeJob(*this, ctrlPointIdx, jointIdx, PsiMatrix, weightsPrimeMatrix, pRow, alpha));
			/*
			Eigen::Matrix<double, 1, 10> currentPsi = PsiMatrix.block(ctrlPointIdx, jointIdx * 10, 1, 10);
			Eigen::Matrix<double, 1, 10> currentOmega = ((1.0 - alpha) * currentPsi) + (alpha * weightsPrimeMatrix(ctrlPointIdx, jointIdx) * pRow);
			m_omegaMatrix.block(ctrlPointIdx, 10 * jointIdx, 1, 10) = currentOmega;
			*/
		}
	}
	while (g_theJobSystem->GetNumCompletedJobs() != numControlPoints * m_numJoints) {} // Wait until all jobs complete

	while (true) {
		Job* completedJob = g_theJobSystem->GetCompletedJob();
		if (completedJob) {
			FBXDDMOmegaPrecomputeJob* omegaPrecomputeJob = dynamic_cast<FBXDDMOmegaPrecomputeJob*>(completedJob);
			if (omegaPrecomputeJob) {
				m_omegaMatrix.block(omegaPrecomputeJob->m_ctrlPointIdx, 10 * omegaPrecomputeJob->m_jointIdx, 1, 10) = omegaPrecomputeJob->m_currentOmega;
				delete omegaPrecomputeJob;
			}
			else {
				ERROR_AND_DIE("Doesn't make sense...");
			}
		}
		else {
			break;
		}
	}

	double omegaMatrixEndTime = GetCurrentTimeSeconds();

	DebuggerPrintf(Stringf("omegaMatrixCalcTime: %.3lf\n", omegaMatrixEndTime - omegaMatrixStartTime).c_str());
	m_isPrecomputed = true;
}

bool FBXDDMModifier::IsPrecomputed() const
{
	return m_isPrecomputed;
}

void FBXDDMModifier::ResetIsPrecomputed()
{
	m_isPrecomputed = false;
}

Eigen::Index FBXDDMModifier::GetNumJoints() const
{
	return m_numJoints;
}

const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>& FBXDDMModifier::GetConstRefToOmegaMatrix() const
{
	return m_omegaMatrix;
}

const Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>& FBXDDMModifier::GetConstRefToV1ConstantMatrix() const
{
	return m_v1ConstantMatrix;
}

const Eigen::Matrix<double, 1, 3> FBXDDMModifier::GetControlPoint(unsigned int index) const
{
	if (index >= m_controlPointsMatrixRestPose.rows()) {
		ERROR_AND_DIE(Stringf("index is %d whereas m_controlPointsMatrixRestPose.rows(): %d", index, m_controlPointsMatrixRestPose.rows()));
	}
	return m_controlPointsMatrixRestPose.row(index);
}

void FBXDDMModifier::SetNeedsRecalculation()
{
	m_needsRecalculation = true;
}

Eigen::VectorXd FBXDDMModifier::GetUpperTriangleOfSymmetric4x4Matrix(const Eigen::Matrix<double, 4, 4>& matrix)
{
	Eigen::VectorXd result(10);
	result << matrix(0, 0), matrix(0, 1), matrix(0, 2), matrix(0, 3), matrix(1, 1), matrix(1, 2), matrix(1, 3), matrix(2, 2), matrix(2, 3), matrix(3, 3);
	return result;
}

Eigen::VectorXd FBXDDMModifier::GetUpperTriangleOfSymmetric3x3Matrix(const Eigen::Matrix<double, 3, 3>& matrix)
{
	Eigen::VectorXd result(6);
	result << matrix(0, 0), matrix(0, 1), matrix(0, 2), matrix(1, 1), matrix(1, 2), matrix(2, 2);
	return result;
}

Eigen::Matrix<double, 4, 4> FBXDDMModifier::ConvertMat44ToEigen(const Mat44& mat)
{
	Eigen::Matrix<double, 4, 4> matToReturn;

	matToReturn << mat.m_values[Mat44::Ix], mat.m_values[Mat44::Jx], mat.m_values[Mat44::Kx], mat.m_values[Mat44::Tx],
		mat.m_values[Mat44::Iy], mat.m_values[Mat44::Jy], mat.m_values[Mat44::Ky], mat.m_values[Mat44::Ty],
		mat.m_values[Mat44::Iz], mat.m_values[Mat44::Jz], mat.m_values[Mat44::Kz], mat.m_values[Mat44::Tz],
		mat.m_values[Mat44::Iw], mat.m_values[Mat44::Jw], mat.m_values[Mat44::Kw], mat.m_values[Mat44::Tw];

	return matToReturn;
}

Eigen::Matrix<double, 4, 4> FBXDDMModifier::GetSymmetricMatrix4x4From10Floats(const Eigen::Matrix<double, 1, 10>& floats)
{
	Eigen::Matrix<double, 4, 4> symmetricMatrix;

	symmetricMatrix << floats(0), floats(1), floats(2), floats(3),
		floats(1), floats(4), floats(5), floats(6),
		floats(2), floats(5), floats(7), floats(8),
		floats(3), floats(6), floats(8), floats(9);

	return symmetricMatrix;
}

Eigen::Matrix<double, 3, 3> FBXDDMModifier::GetSymmetricMatrix3x3From6Floats(const Eigen::Matrix<double, 1, 6>& floats)
{
	Eigen::Matrix<double, 3, 3> symmetricMatrix;

	symmetricMatrix << floats(0), floats(1), floats(2),
		floats(1), floats(3), floats(4),
		floats(2), floats(4), floats(5);

	return symmetricMatrix;
}

bool FBXDDMModifier::DoesMatrixHaveNans(const Eigen::SparseMatrix<double>& inquiryMatrix)
{
	for (int k = 0; k < inquiryMatrix.outerSize(); ++k)
	{
		for (Eigen::SparseMatrix<double>::InnerIterator it(inquiryMatrix, k); it; ++it)
		{
			if (std::isnan(it.value()))
			{
				return true; // Found a NaN value, return true
			}
		}
	}

	return false; // No NaN values found
}