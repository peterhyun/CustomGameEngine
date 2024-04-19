#include <math.h>
#include "Engine/Math/IntVec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

const IntVec3 IntVec3::ZERO(0, 0, 0);

IntVec3::IntVec3(const IntVec3& copyFrom) : x(copyFrom.x), y(copyFrom.y), z(copyFrom.z)
{
}

IntVec3::IntVec3(int initialX, int initialY, int initialZ) : x(initialX), y(initialY), z(initialZ)
{
}

float IntVec3::GetLength() const
{
	return sqrtf((float)(x * x + y * y + z * z));
}

int IntVec3::GetTaxicabLength() const
{
	return (x > 0 ? x : -x) + (y > 0 ? y : -y) + (z > 0 ? z : -z);
}

int IntVec3::GetLengthSquared() const
{
	return x * x + y * y + z * z;
}

void IntVec3::SetFromText(char const* text)
{
	Strings splitStrings = SplitStringOnDelimeter(text, ',');
	if (splitStrings.size() < 2)
		ERROR_RECOVERABLE("Setting IntVec3 from text but string contains less than 3 floats");
	x = atoi(splitStrings[0].c_str());
	y = atoi(splitStrings[1].c_str());
	z = atoi(splitStrings[2].c_str());
}

void IntVec3::operator=(const IntVec3& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
}

bool IntVec3::operator==(const IntVec3& compare) const
{
	return (x == compare.x) && (y == compare.y) && (z == compare.z);
}

bool IntVec3::operator!=(const IntVec3& compare) const
{
	return (x != compare.x) || (y != compare.y) || (z != compare.z);
}

const IntVec3 IntVec3::operator+(const IntVec3& vecToAdd) const
{
	return IntVec3(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z);
}

const IntVec3 IntVec3::operator-(const IntVec3& vecToSubtract) const
{
	return IntVec3(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z);
}
