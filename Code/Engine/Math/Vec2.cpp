#include <math.h>
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

const Vec2 Vec2::ZERO(0.f, 0.f);
const Vec2 Vec2::ONE(1.f, 1.f);

//-----------------------------------------------------------------------------------------------
Vec2::Vec2( const Vec2& copy )
	: x( copy.x )
	, y( copy.y )
{
}


//-----------------------------------------------------------------------------------------------
Vec2::Vec2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{
}

Vec2::Vec2(const IntVec2& copyFrom): x((float)copyFrom.x), y((float)copyFrom.y)
{
}

Vec2::Vec2(const Vec3& copyFrom): x(copyFrom.x), y(copyFrom.y)
{
}

Vec2::Vec2(const Vec4& copyFrom): x(copyFrom.x), y(copyFrom.y)
{
}

Vec2 const Vec2::MakeFromPolarRadians(float orientationRadians, float length) {
	return Vec2(length * cosf(orientationRadians), length * sinf(orientationRadians));
}

Vec2 const Vec2::MakeFromPolarDegrees(float orientationDegrees, float length) {

	return Vec2(length * cosf(ConvertDegreesToRadians(orientationDegrees)), length * sinf(ConvertDegreesToRadians(orientationDegrees)));
}

//Accessors (const methods)
float Vec2::GetLength() const {
	return sqrtf(x * x + y * y);
}

float Vec2::GetLengthSquared() const {
	return x * x + y * y;
}

float Vec2::GetOrientationRadians() const {
	return atan2f(y, x);
}

float Vec2::GetOrientationDegrees() const {
	return ConvertRadiansToDegrees(atan2f(y, x));
}

Vec2 const Vec2::GetRotated90Degrees() const {
	return Vec2(-y, x);
}

Vec2 const Vec2::GetRotatedMinus90Degrees() const {
	return Vec2(y, -x);
}

Vec2 const Vec2::GetRotatedRadians(float deltaRadians) const {
	float cos_deltaRadians = cosf(deltaRadians);
	float sin_deltaRadians = sinf(deltaRadians);
	return Vec2(cos_deltaRadians * x - sin_deltaRadians * y, sin_deltaRadians * x + cos_deltaRadians * y);
}

Vec2 const Vec2::GetRotatedDegrees(float deltaDegrees) const {
	return GetRotatedRadians(ConvertDegreesToRadians(deltaDegrees));
}

Vec2 const Vec2::GetClamped(float maxLength) const {
	float length = GetLength();
	if (length > maxLength) {
		float multiplier = maxLength / length;
		return multiplier * (*this);
	}
	return *this;
}

Vec2 const Vec2::GetNormalized() const {
	float multiplier = 1 / GetLength();
	if (isinf(multiplier)) {
		return Vec2(0.0f, 0.0f);
	}
	return multiplier * (*this);
}

Vec2 const Vec2::GetReflected(const Vec2& normalizedNormalVector) const
{
	Vec2 flippedVector(-x, -y);
	float dotProduct = DotProduct2D(flippedVector, normalizedNormalVector);
	return (2.0f * dotProduct * normalizedNormalVector) - flippedVector;
}

//Mutators (non-const methods)
void Vec2::SetOrientationRadians(float newOrientationRadians) {
	float length = GetLength();
	x = length * cosf(newOrientationRadians);
	y = length * sinf(newOrientationRadians);
}
void Vec2::SetOrientationDegrees(float newOrientationDegrees) {
	SetOrientationRadians(ConvertDegreesToRadians(newOrientationDegrees));
}

void Vec2::SetPolarRadians(float newOrientationRadians, float newLength) {
	x = newLength * cosf(newOrientationRadians);
	y = newLength * sinf(newOrientationRadians);
}

void Vec2::SetPolarDegrees(float newOrientationDegrees, float newLength) {
	SetPolarRadians(ConvertDegreesToRadians(newOrientationDegrees), newLength);
}

void Vec2::SetFromText(char const* text)
{
	Strings splitStrings = SplitStringOnDelimeter(text, ',');
	if (splitStrings.size() < 2)
		ERROR_RECOVERABLE("Setting Vec2 from text but string contains less than 2 floats");
	x = (float)atof(splitStrings[0].c_str());
	y = (float)atof(splitStrings[1].c_str());
}

void Vec2::Rotate90Degrees() {
	float temp = x;
	x = -y;
	y = temp;
}

void Vec2::RotateMinus90Degrees() {
	float temp = x;
	x = y;
	y = -temp;
}

void Vec2::RotateRadians(float deltaRadians) {
	float cos_deltaRadians = cosf(deltaRadians);
	float sin_deltaRadians = sinf(deltaRadians);
	float x_temp = x;
	x = cos_deltaRadians * x - sin_deltaRadians * y;
	y = sin_deltaRadians * x_temp + cos_deltaRadians * y;
}

void Vec2::RotateDegrees(float deltaDegrees) {
	RotateRadians(ConvertDegreesToRadians(deltaDegrees));
}

void Vec2::SetLength(float newLength) {
	float multiplier = newLength / GetLength();
	x *= multiplier;
	y *= multiplier;
}

void Vec2::ClampLength(float maxLength) {
	float length = GetLength();
	if (length > maxLength) {
		float multiplier = maxLength / length;
		x *= multiplier;
		y *= multiplier;
	}
}

void Vec2::Normalize() {
	float length = GetLength();
	if (length < 1e-10) {
		x = 0.0f;
		y = 0.0f;
		return;
	}
	float multiplier = 1 / GetLength();
	x *= multiplier;
	y *= multiplier;
}

float Vec2::NormalizeAndGetPreviousLength() {
	float length = GetLength();
	Normalize();
	return length;
}

void Vec2::Reflect(const Vec2& normalizedNormalVector)
{
	Vec2 flippedVector(-x, -y);
	float dotProduct = DotProduct2D(flippedVector, normalizedNormalVector);
	Vec2 reflectedVector = (2.0f * dotProduct * normalizedNormalVector) - flippedVector;
	x = reflectedVector.x;
	y = reflectedVector.y;
}

//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator + ( const Vec2& vecToAdd ) const
{
	return Vec2( x + vecToAdd.x, y + vecToAdd.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-( const Vec2& vecToSubtract ) const
{
	return Vec2( x - vecToSubtract.x, y - vecToSubtract.y );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator-() const
{
	return Vec2( -x, -y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( float uniformScale ) const
{
	return Vec2( uniformScale * x, uniformScale * y );
}


//------------------------------------------------------------------------------------------------
const Vec2 Vec2::operator*( const Vec2& vecToMultiply ) const
{
	return Vec2( vecToMultiply.x * x, vecToMultiply.y * y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Vec2::operator/( float inverseScale ) const
{
	return Vec2( x/inverseScale, y/inverseScale );
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator+=( const Vec2& vecToAdd )
{
	x += vecToAdd.x;
	y += vecToAdd.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator-=( const Vec2& vecToSubtract )
{
	x -= vecToSubtract.x;
	y -= vecToSubtract.y;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator*=( const float uniformScale )
{
	x *= uniformScale;
	y *= uniformScale;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator/=( const float uniformDivisor )
{
	x /= uniformDivisor;
	y /= uniformDivisor;
}


//-----------------------------------------------------------------------------------------------
void Vec2::operator=( const Vec2& copyFrom )
{
	x = copyFrom.x;
	y = copyFrom.y;
}

bool Vec2::operator<(const Vec2& other) const
{
	if (x != other.x) {
		return x < other.x;
	}
	return y < other.y;
}


//-----------------------------------------------------------------------------------------------
const Vec2 operator*( float uniformScale, const Vec2& vecToScale )
{
	return Vec2( vecToScale.x * uniformScale, vecToScale.y * uniformScale );
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator==( const Vec2& compare ) const
{
	return (x==compare.x) && (y == compare.y);
}


//-----------------------------------------------------------------------------------------------
bool Vec2::operator!=( const Vec2& compare ) const
{
	return !((*this) == compare);
}






