#include <math.h>
#include "Engine/Math/IntVec4.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

const IntVec4 IntVec4::ZERO(0, 0, 0, 0);

IntVec4::IntVec4(const IntVec4& copyFrom) : x(copyFrom.x), y(copyFrom.y), z(copyFrom.z), w(copyFrom.w)
{
}

IntVec4::IntVec4(int initialX, int initialY, int initialZ, int initialW) : x(initialX), y(initialY), z(initialZ), w(initialW)
{
}

float IntVec4::GetLength() const
{
	return sqrtf((float)(x * x + y * y + z * z + w * w));
}

int IntVec4::GetTaxicabLength() const
{
	return (x > 0 ? x : -x) + (y > 0 ? y : -y) + (z > 0 ? z : -z) + (w > 0 ? w : -w);
}

int IntVec4::GetLengthSquared() const
{
	return x * x + y * y + z * z + w * w;
}

void IntVec4::Clear()
{
	x = 0;
	y = 0;
	z = 0;
	w = 0;
}

void IntVec4::SetFromText(char const* text)
{
	Strings splitStrings = SplitStringOnDelimeter(text, ',');
	if (splitStrings.size() < 3)
		ERROR_RECOVERABLE("Setting IntVec4 from text but string contains less than 4 floats");
	x = atoi(splitStrings[0].c_str());
	y = atoi(splitStrings[1].c_str());
	z = atoi(splitStrings[2].c_str());
	w = atoi(splitStrings[3].c_str());
}

void IntVec4::operator=(const IntVec4& copyFrom)
{
	x = copyFrom.x;
	y = copyFrom.y;
	z = copyFrom.z;
	w = copyFrom.w;
}

bool IntVec4::operator==(const IntVec4& compare) const
{
	return (x == compare.x) && (y == compare.y) && (z == compare.z) && (w == compare.w);
}

bool IntVec4::operator!=(const IntVec4& compare) const
{
	return (x != compare.x) || (y != compare.y) || (z != compare.z) || (w != compare.w);
}

const IntVec4 IntVec4::operator+(const IntVec4& vecToAdd) const
{
	return IntVec4(x + vecToAdd.x, y + vecToAdd.y, z + vecToAdd.z, w + vecToAdd.w);
}

const IntVec4 IntVec4::operator-(const IntVec4& vecToSubtract) const
{
	return IntVec4(x - vecToSubtract.x, y - vecToSubtract.y, z - vecToSubtract.z, w - vecToSubtract.w);
}

bool IntVec4::operator<(const IntVec4& compare) const
{
	if (x != compare.x) {
		return x < compare.x;
	}
	if (y != compare.y) {
		return y < compare.y;
	}
	if (z != compare.z) {
		return z < compare.z;
	}
	return w < compare.w;
}

int& IntVec4::operator[](unsigned int index)
{
	if(index > 3)
		ERROR_AND_DIE(Stringf("IntVec4::operator[] requires index to be less than 4! input index: %index", index));

	int* components[] = { &x, &y, &z, &w };
	return *components[index];
}