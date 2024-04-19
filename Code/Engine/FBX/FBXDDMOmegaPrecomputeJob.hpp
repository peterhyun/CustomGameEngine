#pragma once
#include "Engine/Multithread/Job.hpp"
#include <Eigen/Dense>
#include <Eigen/Sparse>

class FBXDDMOmegaPrecomputeJob : public Job {
public:
	FBXDDMOmegaPrecomputeJob(class FBXDDMModifier& modifier, unsigned int ctrlPointIdx, unsigned int jointIdx, const Eigen::SparseMatrix<double>& PsiMatrix, const Eigen::MatrixXd& weightsPrimeMatrix, const Eigen::Matrix<double, 1, 10>& pRow, double alpha);
	void Execute() override;
	void OnComplete() override;

public:
	Eigen::Matrix<double, 1, 10> m_currentOmega;	//Output to use
	const unsigned int m_ctrlPointIdx = 0;
	const unsigned int m_jointIdx = 0;

private:
	class FBXDDMModifier& m_modifier;
	const Eigen::SparseMatrix<double>& m_PsiMatrix;
	const Eigen::MatrixXd& m_weightsPrimeMatrix;
	const Eigen::Matrix<double, 1, 10> m_pRow;
	double m_alpha = 0.0f;
};