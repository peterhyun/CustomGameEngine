#pragma once
#include "Engine/Math/Mat44.hpp"
#include "Engine/Fbx/FBXDDMModifier.hpp"
#include <Eigen/Sparse>
#include <Eigen/Dense>
#include <vector>

class Renderer;
class FBXMesh;

class FBXDDMModifierCPU : public FBXDDMModifier {
	friend class FBXMesh;
public:
	FBXDDMModifierCPU(FBXMesh& mesh);
	virtual ~FBXDDMModifierCPU();

	virtual Eigen::MatrixX3f GetVariantv0Deform(const std::vector<Mat44>& allJointTransforms, bool& recalculatedThisFrame) override;
	virtual Eigen::MatrixX3f GetVariantv1Deform(const std::vector<Mat44>& allJointTransforms, bool& recalculatedThisFrame) override;
};