#define _USE_MATH_DEFINES
#include <math.h>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

//-----------------------------------------------------------------------------------------------
Vec3::Vec3( const Vec3& copy )
	: x( copy.x )
	, y( copy.y )
	, z( copy.z )
{
}


//-----------------------------------------------------------------------------------------------
Vec3::Vec3( float initialX, float initialY, float initialZ)
	: x( initialX )
	, y( initialY )
	, z( initialZ )
{
}

Vec3::Vec3(const Vec2& copyFrom, float initialZ):x(copyFrom.x), y(copyFrom.y), z(initialZ)
{
}

Vec3::Vec3(const Vec4& copyFrom): x(copyFrom.x), y(copyFrom.y), z(copyFrom.z)
{
}

const Vec3 Vec3::MakeFromPolarRadians(float latitudeRadians, float longtitudeRadians, float length)
{
	return MakeFromPolarDegrees(ConvertRadiansToDegrees(latitudeRadians), ConvertRadiansToDegrees(longtitudeRadians), length);
}

const Vec3 Vec3::MakeFromPolarDegrees(float latitudeDegrees, float longtitudeDegrees, float length)
{
	float cosLatitude = CosDegrees(latitudeDegrees);
	float sinLatitude = SinDegrees(latitudeDegrees);
	float cosLongtitude = CosDegrees(longtitudeDegrees);
	float sinLongtitude = SinDegrees(longtitudeDegrees);
	return Vec3(length * cosLatitude * cosLongtitude, length * cosLatitude * sinLongtitude, length * sinLatitude); 
}

float Vec3::GetLength() const {
	return sqrtf(x * x + y * y + z * z);
}

float Vec3::GetLengthXY() const {
	return sqrtf(x * x + y * y);
}

float Vec3::GetLengthSquared() const {
	return x * x + y * y + z * z;
}

float Vec3::GetLengthXYSquared() const {
	return x * x + y * y;
}

float Vec3::GetAngleAboutZRadians() const {
	return Vec2(x, y).GetOrientationRadians();
}

float Vec3::GetAngleAboutZDegrees() const {
	return ConvertRadiansToDegrees(GetAngleAboutZRadians());
}

Vec3 const Vec3::GetRotatedAboutZRadians(float deltaRadians) const {
	Vec2 XY_Vec = Vec2(x, y);
	XY_Vec.RotateRadians(deltaRadians);
	return Vec3(XY_Vec.x, XY_Vec.y, z);
}

Vec3 const Vec3::GetRotatedAboutZDegrees(float deltaDegrees) const {
	return GetRotatedAboutZRadians(ConvertDegreesToRadians(deltaDegrees));
}

Vec3 const Vec3::GetClamped(float maxLength) const {
	float length = GetLength();
	if (length > maxLength) {
		float multiplier = maxLength / length;
		return multiplier * (*this);
	}
	return *this;
}

Vec3 const Vec3::GetNormalized() const {
	float multiplier = 1 / GetLength();
	if (isinf(multiplier)) {
		return Vec3(0.0f, 0.0f, 0.0f);
	}
	return multiplier * (*this);
}

void Vec3::Normalize()
{
	float multiplier = 1 / GetLength();
	x *= multiplier;
	y *= multiplier;
	z *= multiplier;
}

void Vec3::SetFromText(char const* text)
{
	Strings splitStrings = SplitStringOnDelimeter(text, ',');
	if (splitStrings.size() < 3)
		ERROR_RECOVERABLE("Setting Vec3 from text but string contains less than 3 floats");
	x = (float)atof(splitStrings[0].c_str());
	y = (float)atof(splitStrings[1].c_str());
	z = (float)atof(splitStrings[2].c_str());
}

//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator + ( const Vec3& vecToAdd ) const
{
	return Vec3( x + vecToAdd.x, y + vecToAdd.y , z + vecToAdd.z);
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-( const Vec3& vecToSubtract ) const
{
	return Vec3( x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z );
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator-() const
{
	return Vec3( -x, -y, -z );


}
//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*( float uniformScale ) const
{
	return Vec3( uniformScale * x, uniformScale * y, uniformScale * z );
}


//------------------------------------------------------------------------------------------------
const Vec3 Vec3::operator*( const Vec3& vecToMultiply ) const
{
	return Vec3( vecToMultiply.x * x, vecToMultiply.y * y, vecToMultiply.z * z );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Vec3::operator/( float inverseScale ) const
{
	return Vec3( x/inverseScale, y/inverseScale, z/inverseScale );
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator+=( const Vec3& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
	z += vecToAdd.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator-=( const Vec3& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
	z -= vecToSubtract.z;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
	z *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
	z /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec3::operator=( const Vec3& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

void Vec3::operator=(const Vec2& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = 0.0f;
}

bool Vec3::operator<(const Vec3& other) const
{
	if (x != other.x) {
		return x < other.x;
	}
	if (y != other.y) {
		return y < other.y;
	}
	return z < other.z;
}


//-----------------------------------------------------------------------------------------------
const Vec3 operator*( float uniformScale, const Vec3& vecToScale )
{
	return Vec3( vecToScale.x * uniformScale, vecToScale.y * uniformScale, vecToScale.z * uniformScale );
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator==( const Vec3& compare ) const
{
	return (x==compare.x) && (y == compare.y) && (z == compare.z);
}


//-----------------------------------------------------------------------------------------------
bool Vec3::operator!=( const Vec3& compare ) const
{
	return !((*this) == compare);
}






