#include "Engine/Math/Quaternion.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#define _USE_MATH_DEFINES
#include <cmath>

Quaternion::Quaternion()
{
}

Quaternion::Quaternion(float w, float x, float y, float z): w(w), x(x), y(y), z(z)
{
}

Quaternion::Quaternion(float rotationDegrees, const Vec3& direction)
{
	float halfDegrees = rotationDegrees * 0.5f;
	float c = CosDegrees(halfDegrees);
	float s = SinDegrees(halfDegrees);
	w = c;
	x = s * direction.x;
	y = s * direction.y;
	z = s * direction.z;
}

Quaternion Quaternion::CreateFromYawPitchRollDegrees(float yaw, float pitch, float roll)
{
	float halfYaw = yaw * 0.5f;
	float halfPitch = pitch * 0.5f;
	float halfRoll = roll * 0.5f;

	float cosYaw = CosDegrees(halfYaw);
	float sinYaw = SinDegrees(halfYaw);
	float cosPitch = CosDegrees(halfPitch);
	float sinPitch = SinDegrees(halfPitch);
	float cosRoll = CosDegrees(halfRoll);
	float sinRoll = SinDegrees(halfRoll);

	Quaternion result;
	result.w = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;
	result.x = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
	result.y = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;
	result.z = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
	return result;
}

Quaternion Quaternion::CreateFromRollPitchYawDegrees(float roll, float pitch, float yaw)
{
	float halfYaw = yaw * 0.5f;
	float halfPitch = pitch * 0.5f;
	float halfRoll = roll * 0.5f;

	float cosYaw = CosDegrees(halfYaw);
	float sinYaw = SinDegrees(halfYaw);
	float cosPitch = CosDegrees(halfPitch);
	float sinPitch = SinDegrees(halfPitch);
	float cosRoll = CosDegrees(halfRoll);
	float sinRoll = SinDegrees(halfRoll);

	Quaternion result;
	result.w = cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw;
	result.x = sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw;
	result.y = cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw;
	result.z = cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw;
	return result;
}

Quaternion Quaternion::CreateFromAxisAndDegrees(float angleDegrees, const Vec3& axis)
{
	Vec3 normalizedAxis = axis.GetNormalized();

	float halfAngleDegrees = angleDegrees * 0.5f;
	float sinHalfAngle = SinDegrees(halfAngleDegrees);

	return Quaternion(CosDegrees(halfAngleDegrees), sinHalfAngle * normalizedAxis.x, sinHalfAngle * normalizedAxis.y, sinHalfAngle * normalizedAxis.z);
}

Quaternion Quaternion::CreateFromAxisAndRadians(float angleRads, const Vec3& axis)
{
	Vec3 normalizedAxis = axis.GetNormalized();

	float halfAngleRads = angleRads * 0.5f;
	float sinHalfRads = SinRadians(halfAngleRads);

	return Quaternion(CosRadians(halfAngleRads), sinHalfRads * normalizedAxis.x, sinHalfRads * normalizedAxis.y, sinHalfRads * normalizedAxis.z);
}

Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t)
{
	Quaternion quat1 = q1.GetNormalized();
	Quaternion quat2 = q2.GetNormalized();

	//Find the shortest path
	float dotProduct = Quaternion::Dot(quat1, quat2);
	if (dotProduct < 0.0f) {
		quat2 = -quat2;
		dotProduct = -dotProduct;
	}

	const float threshold = 0.9995f;  // Threshold for linear interpolation
	float blend1, blend2;

	if (dotProduct > threshold) {
		// Linear interpolation if quaternions are close
		blend1 = 1.0f - t;
		blend2 = t;
	}
	else {
		// Slerping...
		float angle = std::acos(dotProduct);
		float sinAngle = std::sin(angle);
		blend1 = std::sin((1.0f - t) * angle) / sinAngle;
		blend2 = std::sin(t * angle) / sinAngle;
	}

	Quaternion result = blend1 * quat1 + blend2 * quat2;
	return result.GetNormalized();
}

float Quaternion::Dot(const Quaternion& q1, const Quaternion& q2)
{
	return q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z;
}

void Quaternion::Normalize()
{
	float length = sqrtf(x*x + y*y + z*z + w*w);
	if (length == 0.0f) {
		return;
	}
	float inv_length = 1.0f / length;
	x *= inv_length;
	y *= inv_length;
	z *= inv_length;
	w *= inv_length;
}

Quaternion Quaternion::GetNormalized() const
{
	Quaternion quat(*this);
	quat.Normalize();
	return quat;
}

void Quaternion::Inverse()
{
	x *= -1.0f;
	y *= -1.0f;
	z *= -1.0f;
}

Quaternion Quaternion::GetInverse() const
{
	return Quaternion(w, -x, -y, -z);
}

Vec3 Quaternion::GetRotatedPoint(const Vec3& point) const
{
	Quaternion pointQuatRepresentation(0.0f, point.x, point.y, point.z);
	Quaternion result = (*this) * pointQuatRepresentation * GetInverse();
	return Vec3(result.x, result.y, result.z);
}

Mat44 Quaternion::GetRotationMatrix() const
{
	Quaternion normalizedQuat = GetNormalized();

	const float& nw = normalizedQuat.w;
	const float& nx = normalizedQuat.x;
	const float& ny = normalizedQuat.y;
	const float& nz = normalizedQuat.z;

	float xx = nx * nx;
	float xy = nx * ny;
	float xz = nx * nz;
	float xw = nx * nw;

	float yy = ny * ny;
	float yz = ny * nz;
	float yw = ny * nw;

	float zz = nz * nz;
	float zw = nz * nw;

	// Construct the rotation matrix
	Mat44 rotationMatrix;
	rotationMatrix.m_values[Mat44::Ix] = 1.0f - 2.0f * (yy + zz);
	rotationMatrix.m_values[Mat44::Iy] = 2.0f * (xy + zw);
	rotationMatrix.m_values[Mat44::Iz] = 2.0f * (xz - yw);
	rotationMatrix.m_values[Mat44::Iw] = 0.0f;

	rotationMatrix.m_values[Mat44::Jx] = 2.0f * (xy - zw);
	rotationMatrix.m_values[Mat44::Jy] = 1.0f - 2.0f * (xx + zz);
	rotationMatrix.m_values[Mat44::Jz] = 2.0f * (yz + xw);
	rotationMatrix.m_values[Mat44::Jw] = 0.0f;
	
	rotationMatrix.m_values[Mat44::Kx] = 2.0f * (xz + yw);
	rotationMatrix.m_values[Mat44::Ky] = 2.0f * (yz - xw);
	rotationMatrix.m_values[Mat44::Kz] = 1.0f - 2.0f * (xx + yy);
	rotationMatrix.m_values[Mat44::Kw] = 0.0f;

	rotationMatrix.m_values[Mat44::Tx] = 0.0f;
	rotationMatrix.m_values[Mat44::Ty] = 0.0f;
	rotationMatrix.m_values[Mat44::Tz] = 0.0f;
	rotationMatrix.m_values[Mat44::Tw] = 1.0f;

	return rotationMatrix;
}

Quaternion Quaternion::operator*(const Quaternion& qToMultiply) const
{
	/*
	return Quaternion(
		w * qToMultiply.w - x * qToMultiply.x - y * qToMultiply.y - z * qToMultiply.z,
		w * qToMultiply.x + x * qToMultiply.w - y * qToMultiply.z + z * qToMultiply.y,
		w * qToMultiply.y + x * qToMultiply.z + y * qToMultiply.w - z * qToMultiply.x,
		w * qToMultiply.z - x * qToMultiply.y + y * qToMultiply.x + z * qToMultiply.w
		);
	*/

	return Quaternion(
		w * qToMultiply.w - x * qToMultiply.x - y * qToMultiply.y - z * qToMultiply.z,
		w * qToMultiply.x + x * qToMultiply.w + y * qToMultiply.z - z * qToMultiply.y,
		w * qToMultiply.y + y * qToMultiply.w + z * qToMultiply.x - x * qToMultiply.z,
		w * qToMultiply.z + z * qToMultiply.w + x * qToMultiply.y - y * qToMultiply.x
	);
}

Quaternion Quaternion::operator*(float scale) const
{
	return Quaternion(w * scale, x * scale, y * scale, z * scale);
}

Quaternion Quaternion::operator+(const Quaternion& qToAdd) const
{
	return Quaternion(w + qToAdd.w, x + qToAdd.x, y + qToAdd.y, z + qToAdd.z);
}

Quaternion Quaternion::operator-() const
{
	return Quaternion(-w, -x, -y, -z);
}

const Quaternion operator*(float scale, const Quaternion& quatToScale)
{
	return quatToScale * scale;
}

Vec3 Quaternion::operator*(const Vec3& vectorToRotate) const
{
	Quaternion vectorQuat(0.0f, vectorToRotate.x, vectorToRotate.y, vectorToRotate.z);
	Quaternion result = (*this) * vectorQuat * this->GetInverse();
	return Vec3(result.x, result.y, result.z);
}