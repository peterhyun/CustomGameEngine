#include "Engine/IKSolver/JacobianIKSolver.hpp"
#include "Engine/IKSolver/IKSocket.hpp"
#include "Engine/FBX/FBXJoint.hpp"
#include "Engine/FBX/FBXUtils.hpp"
#include "Engine/FBX/FBXModel.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

JacobianIKSolver::JacobianIKSolver(FBXJoint* startJoint, unsigned int maxIterations, float deltaTimeForEachIter) : m_startJoint(startJoint), m_maxIterations(maxIterations), m_deltaTimeForEachIter(deltaTimeForEachIter)
{
}

bool JacobianIKSolver::Solve()
{
	GUARANTEE_OR_DIE(IsReadyToSolve(), "Not able to solve yet (Invalid Settings)");

	IKSocket& socket = m_endJoint->GetRefToIKSocket();
	Mat44 socketGlobalTransform = socket.GetGlobalTransform();

	Mat44 startJointGlobalTransform = m_startJoint->GetGlobalTransformForThisFrame();
	Vec3 startJointGlobalPos = startJointGlobalTransform.GetTranslation3D();
	Vec3 fromStartJointToSocket = socketGlobalTransform.GetTranslation3D() - startJointGlobalPos;

	float maxLengthBetweenStartJointToSocket = GetMaxDistanceBetweenTwoJointsOnTheSameChain(*m_startJoint, *m_endJoint);
	if (fromStartJointToSocket.GetLengthSquared() > maxLengthBetweenStartJointToSocket * maxLengthBetweenStartJointToSocket) {
		Vec3 modifiedFromStartJointToSocket = fromStartJointToSocket.GetNormalized() * maxLengthBetweenStartJointToSocket;
		socketGlobalTransform.SetTranslation3D(startJointGlobalPos + modifiedFromStartJointToSocket);
	}

	Eigen::Matrix<float, 4, 4> globalTransform = ConvertMat44ToEigen(socketGlobalTransform);

	//TODO: convert the 4th column of global transform to a vector3f
	Eigen::Vector3f targetPos = globalTransform.block<3, 1>(0, 3);

	//TODO: convert the globalTransform to a Eigen::Matrix<float, 3, 3>
	Eigen::Matrix<float, 3, 3> targetOriMat = globalTransform.block<3, 3>(0, 0);
	Eigen::Quaternionf targetOri(targetOriMat);
	return Solve(*m_endJoint, targetPos, targetOri);
}

void JacobianIKSolver::SetStartJoint(FBXJoint& startJoint)
{
	m_startJoint = &startJoint;
}

void JacobianIKSolver::SetEndJoint(FBXJoint& endJoint)
{
	m_endJoint = &endJoint;
}

std::string JacobianIKSolver::GetStartJointName() const
{
	if (m_startJoint == nullptr)
		return "";
	return m_startJoint->GetName();
}

std::string JacobianIKSolver::GetEndJointName() const
{
	if (m_endJoint == nullptr)
		return "";
	return m_endJoint->GetName();
}

bool JacobianIKSolver::IsReadyToSolve() const
{
	if (!(m_startJoint != nullptr && m_endJoint != nullptr)) {
		return false;
	}
	if (!(m_startJoint != m_endJoint)) {
		return false;
	}
	if (!(AreJointsInTheSameChain(*m_startJoint, *m_endJoint))) {
		return false;
	}
	return true;
}

Eigen::Matrix<float, 4, 4> JacobianIKSolver::ConvertMat44ToEigen(const Mat44& mat)
{
	Eigen::Matrix<float, 4, 4> matToReturn;

	matToReturn << mat.m_values[Mat44::Ix], mat.m_values[Mat44::Jx], mat.m_values[Mat44::Kx], mat.m_values[Mat44::Tx],
		mat.m_values[Mat44::Iy], mat.m_values[Mat44::Jy], mat.m_values[Mat44::Ky], mat.m_values[Mat44::Ty],
		mat.m_values[Mat44::Iz], mat.m_values[Mat44::Jz], mat.m_values[Mat44::Kz], mat.m_values[Mat44::Tz],
		mat.m_values[Mat44::Iw], mat.m_values[Mat44::Jw], mat.m_values[Mat44::Kw], mat.m_values[Mat44::Tw];

	return matToReturn;
}

bool JacobianIKSolver::Solve(FBXJoint& endJoint, const Eigen::Vector3f& targetPos, const Eigen::Quaternionf& targetOri)
{
	unsigned int iterCount = 0;
	while (iterCount < m_maxIterations) {
		Mat44 endJointTransform = endJoint.GetGlobalTransformForThisFrame();

		const Eigen::Matrix<float, 4, 4> globalTransform = ConvertMat44ToEigen(endJointTransform);
		const Eigen::Vector3f currentPos = globalTransform.block<3, 1>(0, 3);
		Eigen::Matrix<float, 3, 3> globalOriMat = globalTransform.block<3, 3>(0, 0);
		Eigen::Quaternionf currentOriQuat(globalOriMat);
		Eigen::Vector3f fromCurrentPosToTargetPos = (targetPos - currentPos);
		float errorP = fromCurrentPosToTargetPos.norm();
		float errorQ = GetQuaternionLogMagnitude(targetOri, currentOriQuat);
		//Checking early exit
		if (errorP + errorQ < m_errorThreshold) {
			endJoint.GetFBXModel()->SetDDMNeedsRecalculation();
			return true;
		}

		Eigen::Quaternionf fromCurrentOriToTargetOri = targetOri * currentOriQuat.inverse();

		auto x_dot = GetXDot(fromCurrentPosToTargetPos, fromCurrentOriToTargetOri);

		Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> J = CreateJacobianMatrix(endJoint, targetPos);
		Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> I;
		int numJCols = (int)J.cols();
		I.resize(numJCols, numJCols);
		I.setIdentity();
		auto J_plus = (J.transpose() * J + m_regularizationLambdaToBeSquared * m_regularizationLambdaToBeSquared * I).inverse() * J.transpose();

		//detlaQ is a [numJCols * 1] vector
		auto theta_dot = J_plus * x_dot;
		ApplyThetaDotToJoints(theta_dot, endJoint);
		iterCount++;
	}
	endJoint.GetFBXModel()->SetDDMNeedsRecalculation();
	return false;	//Couldn't solve it (reached max iterations)
}

Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> JacobianIKSolver::CreateJacobianMatrix(FBXJoint& endJoint, const Eigen::Vector3f& targetPos)
{
	//return Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic>();
	//1. Figure out the dimensions for the Jacobian Matrix. numRows are 6. numCols are...
	int numCols = GetNumberOfColsOfJacobianMatrix(endJoint);
	Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> jacobianMatrix;
	jacobianMatrix.resize(6, numCols);
	jacobianMatrix.setZero();

	int currentJointColIdx = numCols;
	FBXJoint* currentJoint = &endJoint;
	while (true) {
		bool isXAxisDOF, isYAxisDOF, isZAxisDOF;
		currentJoint->GetDOFAxisSettings(isXAxisDOF, isYAxisDOF, isZAxisDOF);
		int numDOFAxis = currentJoint->GetNumDOFs();

		float solverWeight = currentJoint->GetIKSolverWeight();

		const Mat44 globalTransformOfJoint = currentJoint->GetGlobalTransformForThisFrame();
		const Vec3 jointXAxisGlobal = globalTransformOfJoint.GetIBasis3D();
		const Vec3 jointYAxisGlobal = globalTransformOfJoint.GetJBasis3D();
		const Vec3 jointZAxisGlobal = globalTransformOfJoint.GetKBasis3D();
		const Vec3 jointPosGlobal = globalTransformOfJoint.GetTranslation3D();

		const Vec3 fromJointToTargetPos = Vec3(targetPos[0], targetPos[1], targetPos[2]) - jointPosGlobal;
		Vec3 w;
		currentJointColIdx -= numDOFAxis;
		for (int axisIdx = 0; axisIdx < 3; axisIdx++) {	//In X, Y, Z order
			if (axisIdx == 0) {
				if (isXAxisDOF == false)
					continue;
				else
					w = jointXAxisGlobal;
			}
			if (axisIdx == 1) {
				if (isYAxisDOF == false)
					continue;
				else
					w = jointYAxisGlobal;
			}
			if (axisIdx == 2) {
				if (isZAxisDOF == false)
					continue;
				else
					w = jointZAxisGlobal;
			}

			Vec3 w_cross_p = CrossProduct3D(w, fromJointToTargetPos);
			jacobianMatrix.block(0, currentJointColIdx, 3, 1) = Eigen::Vector3f(w_cross_p.x, w_cross_p.y, w_cross_p.z) * solverWeight;
			jacobianMatrix.block(3, currentJointColIdx, 3, 1) = Eigen::Vector3f(w.x, w.y, w.z) * solverWeight;
			currentJointColIdx++;
		}
		if (currentJoint == m_startJoint) {
			currentJointColIdx -= numDOFAxis;
			break;
		}
		else {
			FBXJoint* parentJoint = currentJoint->GetParentJoint();
			GUARANTEE_OR_DIE(parentJoint != nullptr, "IK Solver logic fucked up!");
			currentJoint = parentJoint;
			currentJointColIdx -= numDOFAxis;
		}
	}
	GUARANTEE_OR_DIE(currentJointColIdx == 0, "Check IK Solver logic for currentJointColIdx");
	return jacobianMatrix;
}

int JacobianIKSolver::GetNumberOfColsOfJacobianMatrix(FBXJoint& endJoint) const
{
	FBXJoint* currentJoint = &endJoint;
	int numCols = 0;
	while (true) {	//Gonna break when it's true
		numCols += currentJoint->GetNumDOFs();
		if (currentJoint == m_startJoint) {
			break;
		}
		else {
			FBXJoint* parentJoint = currentJoint->GetParentJoint();
			GUARANTEE_OR_DIE(parentJoint != nullptr, "IK Solver logic fucked up!");
			currentJoint = parentJoint;
		}
	}
	return numCols;
}


Eigen::Vector<float, 6> JacobianIKSolver::GetXDot(const Eigen::Vector3f& fromCurrentPosToTargetPos, const Eigen::Quaternionf& fromCurrentOriToTargetOri)
{
	Eigen::Vector<float, 6> x_dot;

	Eigen::Vector3f posError = fromCurrentPosToTargetPos * m_deltaTimeForEachIter;

	Eigen::AngleAxisf relativeAxisAngle(fromCurrentOriToTargetOri);
	Eigen::Vector3f oriError = relativeAxisAngle.axis() * (relativeAxisAngle.angle() * m_deltaTimeForEachIter);

	x_dot.segment<3>(0) = posError;
	x_dot.segment<3>(3) = oriError;
	return x_dot;
}

float JacobianIKSolver::GetQuaternionLogMagnitude(const Eigen::Quaternionf& q1, const Eigen::Quaternionf& q2) const
{
	Eigen::Quaternionf q_rel = q1.inverse() * q2;
	// Convert relative rotation quaternion to axis-angle representation
	Eigen::AngleAxisf axis_angle(q_rel);

	// Calculate the magnitude of the axis-angle representation (angle of rotation)
	float log_magnitude = axis_angle.angle();

	return log_magnitude;
}

void JacobianIKSolver::ApplyThetaDotToJoints(const Eigen::VectorXf& thetaDot, FBXJoint& endJoint)
{
	//Now that I have the deltaQ... calculate each delta quaternion from it and apply it to each joint rotation.
	FBXJoint* currentJoint = &endJoint;
	int currentDeltaQRowIdx = (int)thetaDot.rows();
	//Now we move through joints...
	while (true) {
		bool isXAxisDOF, isYAxisDOF, isZAxisDOF;
		currentJoint->GetDOFAxisSettings(isXAxisDOF, isYAxisDOF, isZAxisDOF);
		int numDOFAxis = currentJoint->GetNumDOFs();
		Vec3 w;
		currentDeltaQRowIdx -= numDOFAxis;
		Eigen::Quaternionf totalDeltaQuat(1.0f, 0.0f, 0.0f, 0.0f);
		for (int axisIdx = 0; axisIdx < 3; axisIdx++) {	//In X, Y, Z order
			if (axisIdx == 0) {
				if (isXAxisDOF == false)
					continue;
				else
					w = Vec3(1.0f, 0.0f, 0.0f);
			}
			if (axisIdx == 1) {
				if (isYAxisDOF == false)
					continue;
				else
					w = Vec3(0.0f, 1.0f, 0.0f);
			}
			if (axisIdx == 2) {
				if (isZAxisDOF == false)
					continue;
				else
					w = Vec3(0.0f, 0.0f, 1.0f);
			}
			Eigen::AngleAxisf angleAxis(thetaDot[currentDeltaQRowIdx], Eigen::Vector3f(w.x, w.y, w.z));
			Eigen::Quaternionf deltaQuatForThisAxis(angleAxis);
			totalDeltaQuat = totalDeltaQuat * deltaQuatForThisAxis;
			currentDeltaQRowIdx++;
		}

		//TODO: Apply this local delta quaternion to each joint
		Quaternion localDeltaRotate = currentJoint->GetLocalDeltaRotate();
		Quaternion finalLocalDeltaRotate = localDeltaRotate * Quaternion(totalDeltaQuat.w(), totalDeltaQuat.x(), totalDeltaQuat.y(), totalDeltaQuat.z());
		currentJoint-> SetLocalDeltaRotate(finalLocalDeltaRotate);

		if (currentJoint == m_startJoint) {
			currentDeltaQRowIdx -= numDOFAxis;
			break;
		}
		else {
			FBXJoint* parentJoint = currentJoint->GetParentJoint();
			GUARANTEE_OR_DIE(parentJoint != nullptr, "IK Solver logic fucked up!");
			currentJoint = parentJoint;
			currentDeltaQRowIdx -= numDOFAxis;
		}
	}
	GUARANTEE_OR_DIE(currentDeltaQRowIdx == 0, "Check IK Solver logic for currentJointColIdx");

	//Now update the global matrix
	FBXJoint* parentJointOfStartJoint = m_startJoint->GetParentJoint();
	if (parentJointOfStartJoint) {
		m_startJoint->RecursivelyUpdateGlobalTransformBindPoseForThisFrame(parentJointOfStartJoint->GetGlobalTransformForThisFrame());
	}
	else {
		m_startJoint->RecursivelyUpdateGlobalTransformBindPoseForThisFrame(Mat44());
	}
}