#include <math.h>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

const IntVec2 IntVec2::ZERO(0, 0);

IntVec2::IntVec2(const IntVec2& copyFrom) : x(copyFrom.x), y(copyFrom.y)
{
}

IntVec2::IntVec2(int initialX, int initialY) : x(initialX), y(initialY)
{
}

float IntVec2::GetLength() const
{
	return sqrtf((float)(x * x + y * y));
}

int IntVec2::GetTaxicabLength() const
{
	return (x>0?x:-x) + (y>0?y:-y);
}

int IntVec2::GetLengthSquared() const
{
	return x * x + y * y;
}

float IntVec2::GetOrientationRadians() const
{
	return atan2f((float)y, (float)x);
}

float IntVec2::GetOrientationDegrees() const
{
	return ConvertRadiansToDegrees(GetOrientationRadians());
}

IntVec2 const IntVec2::GetRotated90Degrees() const
{
	return IntVec2(-y, x);
}

IntVec2 const IntVec2::GetRotatedMinus90Degrees() const
{
	return IntVec2(y, -x);
}

void IntVec2::SetFromText(char const* text)
{
	Strings splitStrings = SplitStringOnDelimeter(text, ',');
	if (splitStrings.size() < 2)
		ERROR_RECOVERABLE("Setting IntVec2 from text but string contains less than 2 floats");
	x = atoi(splitStrings[0].c_str());
	y = atoi(splitStrings[1].c_str());
}

void IntVec2::Rotate90Degrees()
{
	int temp = x;
	x = -y;
	y = temp;
}

void IntVec2::RotateMinus90Degrees()
{
	int temp = x;
	x = y;
	y = -temp;
}

void IntVec2::operator=(const IntVec2& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
}

bool IntVec2::operator<(const IntVec2& other) const
{
	if (x < other.x)
		return true;
	else if (x == other.x)
		if (y < other.y)
			return true;
		else
			return false;
	else
		return false;
}

bool IntVec2::operator==(const IntVec2& compare) const
{
	return (x == compare.x) && (y == compare.y);
}

bool IntVec2::operator!=(const IntVec2& compare) const
{
	return (x != compare.x) || (y != compare.y);
}

const IntVec2 IntVec2::operator+(const IntVec2& vecToAdd) const
{
	return IntVec2(x + vecToAdd.x, y + vecToAdd.y);
}

const IntVec2 IntVec2::operator-(const IntVec2& vecToSubtract) const
{
	return IntVec2(x - vecToSubtract.x, y - vecToSubtract.y);
}
