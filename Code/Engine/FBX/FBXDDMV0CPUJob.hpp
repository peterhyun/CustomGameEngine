#pragma once
#include "Engine/Multithread/Job.hpp"
#include <vector>
#include <Eigen/Dense>

class FBXDDMV0CPUJob : public Job {
public:
	FBXDDMV0CPUJob(class FBXDDMModifierCPU& modifierCPU, unsigned int ctrlPointIdx, const std::vector<Eigen::Matrix<double, 4, 4>>& allJointTransformsEigen);
	void Execute() override;
	void OnComplete() override;

public:
	const unsigned int m_ctrlPointIdx = 0;
	Eigen::Matrix<float, 1, 3> m_deformedControlPoint;

private:
	class FBXDDMModifierCPU& m_modifierCPU;
	const std::vector<Eigen::Matrix<double, 4, 4>>& m_allJointTransformsEigen;
};