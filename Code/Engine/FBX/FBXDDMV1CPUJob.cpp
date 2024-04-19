#include "Engine/Fbx/FBXDDMV1CPUJob.hpp"
#include "Engine/Fbx/FBXDDMModifierCPU.hpp"
#include <Eigen/Dense>

FBXDDMV1CPUJob::FBXDDMV1CPUJob(FBXDDMModifierCPU& modifierCPU, unsigned int ctrlPointIdx, const std::vector<Eigen::Matrix<double, 4, 4>>& allJointTransformsEigen) : m_modifierCPU(modifierCPU), m_ctrlPointIdx(ctrlPointIdx), m_allJointTransformsEigen(allJointTransformsEigen)
{
}

void FBXDDMV1CPUJob::Execute()
{
	Eigen::Matrix<double, 4, 4> QMatrix_i;
	QMatrix_i.setZero();
	for (int jointIdx = 0; jointIdx < m_modifierCPU.GetNumJoints(); jointIdx++) {
		QMatrix_i += m_allJointTransformsEigen[jointIdx] * FBXDDMModifierCPU::GetSymmetricMatrix4x4From10Floats(m_modifierCPU.GetConstRefToOmegaMatrix().block(m_ctrlPointIdx, 10 * jointIdx, 1, 10));
	}
	QMatrix_i /= QMatrix_i(QMatrix_i.rows() - 1, QMatrix_i.cols() - 1);	//Normalize it

	//TODO: Perform SVD to get the R and t matrix
	Eigen::Matrix<double, 3, 3> Q_i = QMatrix_i.block(0, 0, 3, 3);
	Eigen::Matrix<double, 3, 1> q_i = QMatrix_i.block(0, 3, 3, 1);
	Eigen::Matrix<double, 3, 1> p_i = QMatrix_i.block(3, 0, 1, 3).transpose();
	Eigen::Matrix<double, 3, 3> Q_qp = Q_i - q_i * p_i.transpose();

	Eigen::Matrix<double, 3, 3> R_i = Q_qp.determinant() * (Q_qp).transpose().inverse() * FBXDDMModifierCPU::GetSymmetricMatrix3x3From6Floats(m_modifierCPU.GetConstRefToV1ConstantMatrix().block(m_ctrlPointIdx, 0, 1, 6));
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

void FBXDDMV1CPUJob::OnComplete()
{
}
