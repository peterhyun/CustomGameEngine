#define _USE_MATH_DEFINES
#include <math.h>
#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
//#include "Engine/Core/EngineCommon.hpp"

const Vec4 Vec4::E1(1.0f, 0.0f, 0.0f, 0.0f);
const Vec4 Vec4::E2(0.0f, 1.0f, 0.0f, 0.0f);
const Vec4 Vec4::E3(0.0f, 0.0f, 1.0f, 0.0f);
const Vec4 Vec4::E4(0.0f, 0.0f, 0.0f, 1.0f);

//-----------------------------------------------------------------------------------------------
Vec4::Vec4(const Vec4& copy)
	: x(copy.x)
	, y(copy.y)
	, z(copy.z)
	, w(copy.w)
{
}


//-----------------------------------------------------------------------------------------------
Vec4::Vec4(float initialX, float initialY, float initialZ, float initialW)
	: x(initialX)
	, y(initialY)
	, z(initialZ)
	, w(initialW)
{
}

Vec4::Vec4(const Vec3& copyFrom, float initialW) :x(copyFrom.x), y(copyFrom.y), z(copyFrom.z), w(initialW)
{
}

void Vec4::Clear()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
	w = 0.0f;
}

//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator + (const Vec4& vecToAdd) const
{
	return Vec4(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z, w + vecToAdd.w);
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator-(const Vec4& vecToSubtract) const
{
	return Vec4(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w);
}


//------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator-() const
{
	return Vec4(-x, -y, -z, -w);


}
//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator*(float uniformScale) const
{
	return Vec4(uniformScale * x, uniformScale * y, uniformScale * z, uniformScale * w);
}


//------------------------------------------------------------------------------------------------
const Vec4 Vec4::operator*(const Vec4& vecToMultiply) const
{
	return Vec4(vecToMultiply.x * x, vecToMultiply.y * y, vecToMultiply.z * z, vecToMultiply.w * w);
}


//-----------------------------------------------------------------------------------------------
const Vec4 Vec4::operator/(float inverseScale) const
{
	return Vec4(x / inverseScale, y / inverseScale, z / inverseScale, w / inverseScale);
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator+=(const Vec4& vecToAdd)
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
	w += vecToAdd.w;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator-=(const Vec4& vecToSubtract)
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
	w -= vecToSubtract.w;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator*=(const float uniformScale)
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
	w *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator/=(const float uniformDivisor)
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
	w /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec4::operator=(const Vec4& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}

void Vec4::operator=(const Vec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = 0.0f;
}

bool Vec4::operator<(const Vec4& other) const
{
	if (x != other.x) {
		return x < other.x;
	}
	if (y != other.y) {
		return y < other.y;
	}
	if (z != other.z) {
		return z < other.z;
	}
	return w < other.w;
}

float& Vec4::operator[](unsigned int index)
{
	if (index > 3)
		ERROR_AND_DIE(Stringf("Vec4::operator[] requires index to be less than 4! input index: %index", index));

	float* components[] = { &x, &y, &z, &w };
	return *components[index];
}

//-----------------------------------------------------------------------------------------------
const Vec4 operator*(float uniformScale, const Vec4& vecToScale)
{
	return Vec4(vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale, vecToScale.w * uniformScale);
}


//-----------------------------------------------------------------------------------------------
bool Vec4::operator==(const Vec4& compare) const
{
	return (x == compare.x) && (y == compare.y) && (z == compare.z) && (w == compare.w);
}


//-----------------------------------------------------------------------------------------------
bool Vec4::operator!=(const Vec4& compare) const
{
	return !((*this) == compare);
}






