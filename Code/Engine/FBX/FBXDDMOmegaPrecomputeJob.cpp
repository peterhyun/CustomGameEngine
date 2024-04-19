#include "Engine/Fbx/FBXDDMOmegaPrecomputeJob.hpp"

FBXDDMOmegaPrecomputeJob::FBXDDMOmegaPrecomputeJob(class FBXDDMModifier& modifier, unsigned int ctrlPointIdx, unsigned int jointIdx, const Eigen::SparseMatrix<double>& PsiMatrix, const Eigen::MatrixXd& weightsPrimeMatrix, const Eigen::Matrix<double, 1, 10>& pRow, double alpha)
	: m_modifier(modifier), m_ctrlPointIdx(ctrlPointIdx), m_jointIdx(jointIdx), m_PsiMatrix(PsiMatrix), m_weightsPrimeMatrix(weightsPrimeMatrix), m_pRow(pRow), m_alpha(alpha)
{
}

void FBXDDMOmegaPrecomputeJob::Execute()
{
	Eigen::Matrix<double, 1, 10> currentPsi = m_PsiMatrix.block(m_ctrlPointIdx, m_jointIdx * 10, 1, 10);
	m_currentOmega = ((1.0 - m_alpha) * currentPsi) + (m_alpha * m_weightsPrimeMatrix(m_ctrlPointIdx, m_jointIdx) * m_pRow);
}

void FBXDDMOmegaPrecomputeJob::OnComplete()
{
}
