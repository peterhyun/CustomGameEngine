#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"

Mat44::Mat44()
{
	for (int i = 0; i < 16 ; i++)
		m_values[i] = 0.0f;
	m_values[Ix] = 1.0f;
	m_values[Jy] = 1.0f;
	m_values[Kz] = 1.0f;
	m_values[Tw] = 1.0f;
}

Mat44::Mat44(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translation2D)
{
	for (int i = 0; i < 16; i++)
		m_values[i] = 0.0f;
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Kz] = 1.0f;
	m_values[Tx] = translation2D.x;
	m_values[Ty] = translation2D.y;
	m_values[Tw] = 1.0f;
}

Mat44::Mat44(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translation3D)
{
	for (int i = 0; i < 16; i++)
		m_values[i] = 0.0f;
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Tx] = translation3D.x;
	m_values[Ty] = translation3D.y;
	m_values[Tz] = translation3D.z;
	m_values[Tw] = 1.0f;
}

Mat44::Mat44(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	for (int i = 0; i < 16; i++)
		m_values[i] = 0.0f;
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;

	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

Mat44::Mat44(float const* sixteenValuesBasisMajor)
{
	for (int i = 0; i < 16 ;i++)
		m_values[i] = sixteenValuesBasisMajor[i];
}

Mat44 const Mat44::CreateTranslation2D(Vec2 const& translationXY)
{
	Mat44 translation;
	translation.m_values[Tx] = translationXY.x;
	translation.m_values[Ty] = translationXY.y;
	return translation;
}

Mat44 const Mat44::CreateTranslation3D(Vec3 const& translationXYZ)
{
	Mat44 translation;
	translation.m_values[Tx] = translationXYZ.x;
	translation.m_values[Ty] = translationXYZ.y;
	translation.m_values[Tz] = translationXYZ.z;
	return translation;
}

Mat44 const Mat44::CreateUniformScale2D(float uniformScaleXY)
{
	Mat44 scale;
	scale.m_values[Ix] = uniformScaleXY;
	scale.m_values[Jy] = uniformScaleXY;
	return scale;
}

Mat44 const Mat44::CreateUniformScale3D(float uniformScaleXYZ)
{
	Mat44 scale;
	scale.m_values[Ix] = uniformScaleXYZ;
	scale.m_values[Jy] = uniformScaleXYZ;
	scale.m_values[Kz] = uniformScaleXYZ;
	return scale;
}

Mat44 const Mat44::CreateNonUniformScale2D(Vec2 const& nonUniformScaleXY)
{
	Mat44 scale;
	scale.m_values[Ix] = nonUniformScaleXY.x;
	scale.m_values[Jy] = nonUniformScaleXY.y;
	return scale;
}

Mat44 const Mat44::CreateNonUniformScale3D(Vec3 const& nonUniformScaleXYZ)
{
	Mat44 scale;
	scale.m_values[Ix] = nonUniformScaleXYZ.x;
	scale.m_values[Jy] = nonUniformScaleXYZ.y;
	scale.m_values[Kz] = nonUniformScaleXYZ.z;
	return scale;
}

Mat44 const Mat44::CreateZRotationDegrees(float rotationDegreesAboutZ)
{
	Mat44 rotation;
	float c = CosDegrees(rotationDegreesAboutZ);
	float s = SinDegrees(rotationDegreesAboutZ);
	rotation.m_values[Ix] = c;
	rotation.m_values[Iy] = s;
	rotation.m_values[Jx] = -s;
	rotation.m_values[Jy] = c;
	return rotation;
}

Mat44 const Mat44::CreateXRotationDegrees(float rotationDegreesAboutX)
{
	Mat44 rotation;
	float c = CosDegrees(rotationDegreesAboutX);
	float s = SinDegrees(rotationDegreesAboutX);
	rotation.m_values[Jy] = c;
	rotation.m_values[Jz] = s;
	rotation.m_values[Ky] = -s;
	rotation.m_values[Kz] = c;
	return rotation;
}

Mat44 const Mat44::CreateYRotationDegrees(float rotationDegreesAboutY)
{
	Mat44 rotation;
	float c = CosDegrees(rotationDegreesAboutY);
	float s = SinDegrees(rotationDegreesAboutY);
	rotation.m_values[Ix] = c;
	rotation.m_values[Iz] = -s;
	rotation.m_values[Kx] = s;
	rotation.m_values[Kz] = c;
	return rotation;
}

Mat44 const Mat44::CreateOrthoProjection(float left, float right, float bottom, float top, float zNear, float zFar)
{
	float input[16] = {0.0f};
	input[Ix] = 2 / (right - left);
	input[Tx] = (left + right) / (left - right);
	input[Jy] = 2 / (top - bottom);
	input[Ty] = (bottom + top) / (bottom - top);
	input[Kz] = 1 / (zFar - zNear);
	input[Tz] = -zNear / (zFar - zNear);
	input[Tw] = 1.0f;
	return Mat44(input);
}

//aspect = Width / Height
Mat44 const Mat44::CreatePerspectiveProjection(float fovYDegrees, float aspect, float zNear, float zFar)
{
	float input[16] = { 0.0f };
	float invTanHalfFov = 1.0f / TanDegrees(fovYDegrees * 0.5f);
	input[Ix] = invTanHalfFov / aspect;
	input[Jy] = invTanHalfFov;
	input[Kz] = zFar / (zFar - zNear);
	input[Tz] = -zNear * input[Kz];
	input[Kw] = 1.0;
	return Mat44(input);
}

Vec2 const Mat44::TransformVectorQuantity2D(Vec2 const& vectorQuantityXY) const
{
	return Vec2(
		m_values[Ix] * vectorQuantityXY.x + m_values[Jx] * vectorQuantityXY.y,
		m_values[Iy] * vectorQuantityXY.x + m_values[Jy] * vectorQuantityXY.y
	);
}

Vec3 const Mat44::TransformVectorQuantity3D(Vec3 const& vectorQuantityXYZ) const
{
	return Vec3(
		m_values[Ix] * vectorQuantityXYZ.x + m_values[Jx] * vectorQuantityXYZ.y + m_values[Kx] * vectorQuantityXYZ.z,
		m_values[Iy] * vectorQuantityXYZ.x + m_values[Jy] * vectorQuantityXYZ.y + m_values[Ky] * vectorQuantityXYZ.z,
		m_values[Iz] * vectorQuantityXYZ.x + m_values[Jz] * vectorQuantityXYZ.y + m_values[Kz] * vectorQuantityXYZ.z
	);
}

Vec2 const Mat44::TransformPosition2D(Vec2 const& positionXY) const
{
	return Vec2(
		m_values[Ix] * positionXY.x + m_values[Jx] * positionXY.y + m_values[Tx],
		m_values[Iy] * positionXY.x + m_values[Jy] * positionXY.y + m_values[Ty]
	);
}

Vec3 const Mat44::TransformPosition3D(Vec3 const& position3D) const
{
	return Vec3(
		m_values[Ix] * position3D.x + m_values[Jx] * position3D.y + m_values[Kx] * position3D.z + m_values[Tx],
		m_values[Iy] * position3D.x + m_values[Jy] * position3D.y + m_values[Ky] * position3D.z + m_values[Ty],
		m_values[Iz] * position3D.x + m_values[Jz] * position3D.y + m_values[Kz] * position3D.z + m_values[Tz]
	);
}

Vec4 const Mat44::TransformHomogeneous3D(Vec4 const& homogeneousPoint3D) const
{
	return Vec4(
		m_values[Ix] * homogeneousPoint3D.x + m_values[Jx] * homogeneousPoint3D.y + m_values[Kx] * homogeneousPoint3D.z + m_values[Tx] * homogeneousPoint3D.w,
		m_values[Iy] * homogeneousPoint3D.x + m_values[Jy] * homogeneousPoint3D.y + m_values[Ky] * homogeneousPoint3D.z + m_values[Ty] * homogeneousPoint3D.w,
		m_values[Iz] * homogeneousPoint3D.x + m_values[Jz] * homogeneousPoint3D.y + m_values[Kz] * homogeneousPoint3D.z + m_values[Tz] * homogeneousPoint3D.w,
		m_values[Iw] * homogeneousPoint3D.x + m_values[Jw] * homogeneousPoint3D.y + m_values[Kw] * homogeneousPoint3D.z + m_values[Tw] * homogeneousPoint3D.w
	);
}

float* Mat44::GetAsFloatArray()
{
	return m_values;
}

float const* Mat44::GetAsFloatArray() const
{
	return m_values;
}

Vec2 const Mat44::GetIBasis2D() const
{
	return Vec2(m_values[Ix], m_values[Iy]);
}

Vec2 const Mat44::GetJBasis2D() const
{
	return Vec2(m_values[Jx], m_values[Jy]);
}

Vec2 const Mat44::GetTranslation2D() const
{
	return Vec2(m_values[Tx], m_values[Ty]);
}

Vec3 const Mat44::GetIBasis3D() const
{
	return Vec3(m_values[Ix], m_values[Iy], m_values[Iz]);
}

Vec3 const Mat44::GetJBasis3D() const
{
	return Vec3(m_values[Jx], m_values[Jy], m_values[Jz]);
}

Vec3 const Mat44::GetKBasis3D() const
{
	return Vec3(m_values[Kx], m_values[Ky], m_values[Kz]);
}

Vec3 const Mat44::GetTranslation3D() const
{
	return Vec3(m_values[Tx], m_values[Ty], m_values[Tz]);
}

Vec4 const Mat44::GetIBasis4D() const
{
	return Vec4(m_values[Ix], m_values[Iy], m_values[Iz], m_values[Iw]);
}

Vec4 const Mat44::GetJBasis4D() const
{
	return Vec4(m_values[Jx], m_values[Jy], m_values[Jz], m_values[Jw]);
}

Vec4 const Mat44::GetKBasis4D() const
{
	return Vec4(m_values[Kx], m_values[Ky], m_values[Kz], m_values[Kw]);
}

Vec4 const Mat44::GetTranslation4D() const
{
	return Vec4(m_values[Tx], m_values[Ty], m_values[Tz], m_values[Tw]);
}

Mat44 const Mat44::GetOrthonormalInverse() const
{
	Mat44 rotationPart = *this;
	rotationPart.m_values[Tx] = 0.0f;
	rotationPart.m_values[Ty] = 0.0f;
	rotationPart.m_values[Tz] = 0.0f;
	rotationPart.Transpose();

	rotationPart.AppendTranslation3D(-Vec3(m_values[Tx], m_values[Ty], m_values[Tz]));

	return rotationPart;
}

void Mat44::SetTranslation2D(Vec2 const& translationXY)
{
	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.0f;
	m_values[Tw] = 1.0f;
}

void Mat44::SetTranslation3D(Vec3 const& translationXYZ)
{
	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.0f;
}

void Mat44::SetIJ2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.0f;
	m_values[Iw] = 0.0f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.0f;
	m_values[Jw] = 0.0f;
}

void Mat44::SetIJT2D(Vec2 const& iBasis2D, Vec2 const& jBasis2D, Vec2 const& translationXY)
{
	m_values[Ix] = iBasis2D.x;
	m_values[Iy] = iBasis2D.y;
	m_values[Iz] = 0.0f;
	m_values[Iw] = 0.0f;

	m_values[Jx] = jBasis2D.x;
	m_values[Jy] = jBasis2D.y;
	m_values[Jz] = 0.0f;
	m_values[Jw] = 0.0f;

	m_values[Tx] = translationXY.x;
	m_values[Ty] = translationXY.y;
	m_values[Tz] = 0.0f;
	m_values[Tw] = 1.0f;

}

void Mat44::SetIJK3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.0f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.0f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.0f;
}

void Mat44::SetIJKT3D(Vec3 const& iBasis3D, Vec3 const& jBasis3D, Vec3 const& kBasis3D, Vec3 const& translationXYZ)
{
	m_values[Ix] = iBasis3D.x;
	m_values[Iy] = iBasis3D.y;
	m_values[Iz] = iBasis3D.z;
	m_values[Iw] = 0.0f;

	m_values[Jx] = jBasis3D.x;
	m_values[Jy] = jBasis3D.y;
	m_values[Jz] = jBasis3D.z;
	m_values[Jw] = 0.0f;

	m_values[Kx] = kBasis3D.x;
	m_values[Ky] = kBasis3D.y;
	m_values[Kz] = kBasis3D.z;
	m_values[Kw] = 0.0f;

	m_values[Tx] = translationXYZ.x;
	m_values[Ty] = translationXYZ.y;
	m_values[Tz] = translationXYZ.z;
	m_values[Tw] = 1.0f;
}

void Mat44::SetIJKT4D(Vec4 const& iBasis4D, Vec4 const& jBasis4D, Vec4 const& kBasis4D, Vec4 const& translation4D)
{
	m_values[Ix] = iBasis4D.x;
	m_values[Iy] = iBasis4D.y;
	m_values[Iz] = iBasis4D.z;
	m_values[Iw] = iBasis4D.w;

	m_values[Jx] = jBasis4D.x;
	m_values[Jy] = jBasis4D.y;
	m_values[Jz] = jBasis4D.z;
	m_values[Jw] = jBasis4D.w;


	m_values[Kx] = kBasis4D.x;
	m_values[Ky] = kBasis4D.y;
	m_values[Kz] = kBasis4D.z;
	m_values[Kw] = kBasis4D.w;

	m_values[Tx] = translation4D.x;
	m_values[Ty] = translation4D.y;
	m_values[Tz] = translation4D.z;
	m_values[Tw] = translation4D.w;
}

void Mat44::Transpose()
{
	Mat44 copyOfThis = *this;
	m_values[Iy] = copyOfThis.m_values[Jx];
	m_values[Iz] = copyOfThis.m_values[Kx];
	m_values[Iw] = copyOfThis.m_values[Tx];

	m_values[Jx] = copyOfThis.m_values[Iy];
	m_values[Jz] = copyOfThis.m_values[Ky];
	m_values[Jw] = copyOfThis.m_values[Ty];

	m_values[Kx] = copyOfThis.m_values[Iz];
	m_values[Ky] = copyOfThis.m_values[Jz];
	m_values[Kw] = copyOfThis.m_values[Tz];

	m_values[Tx] = copyOfThis.m_values[Iw];
	m_values[Ty] = copyOfThis.m_values[Jw];
	m_values[Tz] = copyOfThis.m_values[Kw];
}

// Forward is canonical, Up is secondary, Left tertiary
void Mat44::Orthonormalize_XFwd_YLeft_ZUp()
{
	Vec3 I = this->GetIBasis3D();
	Vec3 K = this->GetKBasis3D();

	Vec3 I_hat = I.GetNormalized();
	Vec3 K_i = GetVectorProjectedOnto3D(K, I);
	Vec3 K_hat = (K - K_i).GetNormalized();

	Vec3 J_hat = CrossProduct3D(K_hat, I_hat);

	this->SetIJK3D(I_hat, J_hat, K_hat);
}

void Mat44::Append(Mat44 const& appendThis)
{
	Mat44 cp = *this;

	m_values[Ix] = cp.m_values[Ix] * appendThis.m_values[Ix] + cp.m_values[Jx] * appendThis.m_values[Iy] + cp.m_values[Kx] * appendThis.m_values[Iz] + cp.m_values[Tx] * appendThis.m_values[Iw];
	m_values[Iy] = cp.m_values[Iy] * appendThis.m_values[Ix] + cp.m_values[Jy] * appendThis.m_values[Iy] + cp.m_values[Ky] * appendThis.m_values[Iz] + cp.m_values[Ty] * appendThis.m_values[Iw];
	m_values[Iz] = cp.m_values[Iz] * appendThis.m_values[Ix] + cp.m_values[Jz] * appendThis.m_values[Iy] + cp.m_values[Kz] * appendThis.m_values[Iz] + cp.m_values[Tz] * appendThis.m_values[Iw];
	m_values[Iw] = cp.m_values[Iw] * appendThis.m_values[Ix] + cp.m_values[Jw] * appendThis.m_values[Iy] + cp.m_values[Kw] * appendThis.m_values[Iz] + cp.m_values[Tw] * appendThis.m_values[Iw];

	m_values[Jx] = cp.m_values[Ix] * appendThis.m_values[Jx] + cp.m_values[Jx] * appendThis.m_values[Jy] + cp.m_values[Kx] * appendThis.m_values[Jz] + cp.m_values[Tx] * appendThis.m_values[Jw];
	m_values[Jy] = cp.m_values[Iy] * appendThis.m_values[Jx] + cp.m_values[Jy] * appendThis.m_values[Jy] + cp.m_values[Ky] * appendThis.m_values[Jz] + cp.m_values[Ty] * appendThis.m_values[Jw];
	m_values[Jz] = cp.m_values[Iz] * appendThis.m_values[Jx] + cp.m_values[Jz] * appendThis.m_values[Jy] + cp.m_values[Kz] * appendThis.m_values[Jz] + cp.m_values[Tz] * appendThis.m_values[Jw];
	m_values[Jw] = cp.m_values[Iw] * appendThis.m_values[Jx] + cp.m_values[Jw] * appendThis.m_values[Jy] + cp.m_values[Kw] * appendThis.m_values[Jz] + cp.m_values[Tw] * appendThis.m_values[Jw];

	m_values[Kx] = cp.m_values[Ix] * appendThis.m_values[Kx] + cp.m_values[Jx] * appendThis.m_values[Ky] + cp.m_values[Kx] * appendThis.m_values[Kz] + cp.m_values[Tx] * appendThis.m_values[Kw];
	m_values[Ky] = cp.m_values[Iy] * appendThis.m_values[Kx] + cp.m_values[Jy] * appendThis.m_values[Ky] + cp.m_values[Ky] * appendThis.m_values[Kz] + cp.m_values[Ty] * appendThis.m_values[Kw];
	m_values[Kz] = cp.m_values[Iz] * appendThis.m_values[Kx] + cp.m_values[Jz] * appendThis.m_values[Ky] + cp.m_values[Kz] * appendThis.m_values[Kz] + cp.m_values[Tz] * appendThis.m_values[Kw];
	m_values[Kw] = cp.m_values[Iw] * appendThis.m_values[Kx] + cp.m_values[Jw] * appendThis.m_values[Ky] + cp.m_values[Kw] * appendThis.m_values[Kz] + cp.m_values[Tw] * appendThis.m_values[Kw];

	m_values[Tx] = cp.m_values[Ix] * appendThis.m_values[Tx] + cp.m_values[Jx] * appendThis.m_values[Ty] + cp.m_values[Kx] * appendThis.m_values[Tz] + cp.m_values[Tx] * appendThis.m_values[Tw];
	m_values[Ty] = cp.m_values[Iy] * appendThis.m_values[Tx] + cp.m_values[Jy] * appendThis.m_values[Ty] + cp.m_values[Ky] * appendThis.m_values[Tz] + cp.m_values[Ty] * appendThis.m_values[Tw];
	m_values[Tz] = cp.m_values[Iz] * appendThis.m_values[Tx] + cp.m_values[Jz] * appendThis.m_values[Ty] + cp.m_values[Kz] * appendThis.m_values[Tz] + cp.m_values[Tz] * appendThis.m_values[Tw];
	m_values[Tw] = cp.m_values[Iw] * appendThis.m_values[Tx] + cp.m_values[Jw] * appendThis.m_values[Ty] + cp.m_values[Kw] * appendThis.m_values[Tz] + cp.m_values[Tw] * appendThis.m_values[Tw];
}

void Mat44::AppendZRotation(float degreesRotationAboutZ)
{
	Append(Mat44::CreateZRotationDegrees(degreesRotationAboutZ));
}

void Mat44::AppendYRotation(float degreesRotationAboutY)
{
	Append(Mat44::CreateYRotationDegrees(degreesRotationAboutY));
}

void Mat44::AppendXRotation(float degreesRotationAboutX)
{
	Append(Mat44::CreateXRotationDegrees(degreesRotationAboutX));
}

void Mat44::AppendTranslation2D(Vec2 const& translationXY)
{
	Append(Mat44::CreateTranslation2D(translationXY));
}

void Mat44::AppendTranslation3D(Vec3 const& translationXYZ)
{
	Append(Mat44::CreateTranslation3D(translationXYZ));
}

void Mat44::AppendScaleUniform2D(float uniformScaleXY)
{
	Append(Mat44::CreateUniformScale2D(uniformScaleXY));
}

void Mat44::AppendScaleUniform3D(float uniformScaleXYZ)
{
	Append(Mat44::CreateUniformScale3D(uniformScaleXYZ));
}

void Mat44::AppendScaleNonUniform2D(Vec2 const& nonUniformScaleXY)
{
	Append(Mat44::CreateNonUniformScale2D(nonUniformScaleXY));
}

void Mat44::AppendScaleNonUniform3D(Vec3 const& nonUniformScaleXYZ)
{
	Append(Mat44::CreateNonUniformScale3D(nonUniformScaleXYZ));
}

void Mat44::RemoveTranslation3D()
{
	m_values[Tx] = 0.0f;
	m_values[Ty] = 0.0f;
	m_values[Tz] = 0.0f;
}

void Mat44::RemoveRotation3D()
{
	m_values[Ix] = 1.0f;
	m_values[Iy] = 0.0f;
	m_values[Iz] = 0.0f;

	m_values[Jx] = 0.0f;
	m_values[Jy] = 1.0f;
	m_values[Jz] = 0.0f;

	m_values[Kx] = 0.0f;
	m_values[Ky] = 0.0f;
	m_values[Kz] = 1.0f;
}

void Mat44::CopyRotation(const Mat44& matToCopyRotFrom)
{
	m_values[Ix] = matToCopyRotFrom.m_values[Ix];
	m_values[Iy] = matToCopyRotFrom.m_values[Iy];
	m_values[Iz] = matToCopyRotFrom.m_values[Iz];

	m_values[Jx] = matToCopyRotFrom.m_values[Jx];
	m_values[Jy] = matToCopyRotFrom.m_values[Jy];
	m_values[Jz] = matToCopyRotFrom.m_values[Jz];

	m_values[Kx] = matToCopyRotFrom.m_values[Kx];
	m_values[Ky] = matToCopyRotFrom.m_values[Ky];
	m_values[Kz] = matToCopyRotFrom.m_values[Kz];
}
