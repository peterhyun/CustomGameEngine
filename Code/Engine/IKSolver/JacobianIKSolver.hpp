#pragma once
#include <Eigen/Dense>
#include <string>

class FBXJoint;
struct Mat44;
struct Vec3;

//Note: This is for single chain only. Maybe later add multi-end effectors!
class JacobianIKSolver {
public:
	JacobianIKSolver(FBXJoint* startJoint, unsigned int maxIterations = 100, float deltaTimeForEachIter = 0.02f);
	bool Solve();
	void SetStartJoint(FBXJoint& startJoint);
	void SetEndJoint(FBXJoint& endJoint);
	std::string GetStartJointName() const;
	std::string GetEndJointName() const;
	bool IsReadyToSolve() const;
	//Helper functions
	static Eigen::Matrix<float, 4, 4> ConvertMat44ToEigen(const Mat44& mat);

private:
	bool Solve(FBXJoint& endJoint, const Eigen::Vector3f& targetPos, const Eigen::Quaternionf& targetOri);
	Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> CreateJacobianMatrix(FBXJoint& endJoint, const Eigen::Vector3f& targetPos);
	int GetNumberOfColsOfJacobianMatrix(FBXJoint& endJoint) const;
	Eigen::Vector<float, 6> GetXDot(const Eigen::Vector3f& fromCurrentPosToTargetPos, const Eigen::Quaternionf& fromCurrentOriToTargetOri);
	float GetQuaternionLogMagnitude(const Eigen::Quaternionf& q1, const Eigen::Quaternionf& q2) const;
	void ApplyThetaDotToJoints(const Eigen::VectorXf& thetaDot, FBXJoint& endJoint);

private:
	FBXJoint* m_startJoint = nullptr;
	FBXJoint* m_endJoint = nullptr;
	const unsigned int m_maxIterations = 100;
	const float m_deltaTimeForEachIter = 0.01f;
	const float m_errorThreshold = 0.1f;
	const float m_regularizationLambdaToBeSquared = 0.2f;
};