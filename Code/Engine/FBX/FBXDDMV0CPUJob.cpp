#include "Engine/Fbx/FBXDDMV0CPUJob.hpp"
#include "Engine/Fbx/FBXDDMModifierCPU.hpp"
#include "ThirdParty/svd3.h"
#include <Eigen/Dense>

FBXDDMV0CPUJob::FBXDDMV0CPUJob(FBXDDMModifierCPU& modifierCPU, unsigned int ctrlPointIdx, const std::vector<Eigen::Matrix<double, 4, 4>>& allJointTransformsEigen) : m_modifierCPU(modifierCPU), m_ctrlPointIdx(ctrlPointIdx), m_allJointTransformsEigen(allJointTransformsEigen)
{
}

void FBXDDMV0CPUJob::Execute()
{
	Eigen::Matrix<double, 4, 4> QMatrix_i;
	QMatrix_i.setZero();
	for (int jointIdx = 0; jointIdx < m_modifierCPU.GetNumJoints(); jointIdx++) {
		Eigen::Matrix4d symMat = FBXDDMModifierCPU::GetSymmetricMatrix4x4From10Floats(m_modifierCPU.GetConstRefToOmegaMatrix().block(m_ctrlPointIdx, 10 * jointIdx, 1, 10));
		Eigen::Matrix4d productMat = m_allJointTransformsEigen[jointIdx] * symMat;
		QMatrix_i += productMat;		
	}

	QMatrix_i /= QMatrix_i(QMatrix_i.rows() - 1, QMatrix_i.cols() - 1);	//Normalize it

	//TODO: Perform SVD to get the R and t matrix
	Eigen::Matrix<double, 3, 3> Q_i = QMatrix_i.block(0, 0, 3, 3);
	Eigen::Matrix<double, 3, 1> q_i = QMatrix_i.block(0, 3, 3, 1);
	Eigen::Matrix<double, 3, 1> p_i = QMatrix_i.block(3, 0, 1, 3).transpose();
	Eigen::Matrix<double, 3, 3> U_S_Vt = Q_i - q_i * p_i.transpose();

	Eigen::Matrix<float, 3, 3> U;
	Eigen::Matrix<float, 3, 3> S;
	Eigen::Matrix<float, 3, 3> V;
	svd((float)U_S_Vt(0, 0), (float)U_S_Vt(0, 1), (float)U_S_Vt(0, 2), 
		(float)U_S_Vt(1, 0), (float)U_S_Vt(1, 1), (float)U_S_Vt(1, 2),
		(float)U_S_Vt(2, 0), (float)U_S_Vt(2, 1), (float)U_S_Vt(2, 2),
		U(0, 0), U(0, 1), U(0, 2),
		U(1, 0), U(1, 1), U(1, 2),
		U(2, 0), U(2, 1), U(2, 2),
		S(0, 0), S(0, 1), S(0, 2),
		S(1, 0), S(1, 1), S(1, 2),
		S(2, 0), S(2, 1), S(2, 2),
		V(0, 0), V(0, 1), V(0, 2),
		V(1, 0), V(1, 1), V(1, 2),
		V(2, 0), V(2, 1), V(2, 2)
	);
	Eigen::Matrix<double, 3, 3> R_i = (U * V.transpose()).cast<double>();

	/*
	Eigen::JacobiSVD<Eigen::Matrix<double, 3, 3>> svdSolver(U_S_Vt, Eigen::ComputeFullU | Eigen::ComputeFullV);
	Eigen::Matrix<double, 3, 3> R_i = svdSolver.matrixU() * svdSolver.matrixV().transpose();
	*/
	Eigen::Matrix<double, 3, 1> t_i = q_i - R_i * p_i;

	Eigen::Matrix<double, 4, 4> gamma_i;
	gamma_i.block(0, 0, 3, 3) = R_i;
	gamma_i.block(3, 0, 1, 3).setZero();
	gamma_i.block(3, 3, 1, 1).setOnes();
	gamma_i.block(0, 3, 3, 1) = t_i;

	Eigen::Matrix<double, 4, 1> affineCurrentControlPoint;
	affineCurrentControlPoint.block(0, 0, 3, 1) = m_modifierCPU.GetControlPoint(m_ctrlPointIdx).transpose();
	affineCurrentControlPoint.block(3, 0, 1, 1).setOnes();

	m_deformedControlPoint = (gamma_i * affineCurrentControlPoint).block(0, 0, 3, 1).transpose().cast<float>();
}

void FBXDDMV0CPUJob::OnComplete()
{
}
