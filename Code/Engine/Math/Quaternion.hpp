#pragma once

struct Vec3;
struct Mat44;

struct Quaternion {
public:
	static Quaternion CreateFromYawPitchRollDegrees(float yaw, float pitch, float roll);
	static Quaternion CreateFromRollPitchYawDegrees(float roll, float pitch, float yaw);
	static Quaternion CreateFromAxisAndDegrees(float angleDegrees, const Vec3& axis);
	static Quaternion CreateFromAxisAndRadians(float angleRads, const Vec3& axis);
	static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t);
	static float Dot(const Quaternion& q1, const Quaternion& q2);

	Quaternion();
	Quaternion(float w, float x, float y, float z);
	Quaternion(float rotationDegrees, const Vec3& direction);

	void Normalize();
	Quaternion GetNormalized() const;
	void Inverse();
	Quaternion GetInverse() const;
	Vec3 GetRotatedPoint(const Vec3& point) const;
	Mat44 GetRotationMatrix() const;

	// Operators ( const )
	Quaternion operator* (const Quaternion& qToMultiply) const;
	Quaternion operator* (float scale) const;
	Quaternion operator+(const Quaternion& qToAdd) const;
	Quaternion operator-() const;
	Vec3 operator*(const Vec3& vectorToRotate) const;

	friend const Quaternion operator*(float scale, const Quaternion& quatToScale);

public:
	//Uses hamilton representation
	float w = 1.0f;
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};