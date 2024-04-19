#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

const FloatRange FloatRange::ZERO(0.f, 0.f);
const FloatRange FloatRange::ONE(1.f, 1.f);
const FloatRange FloatRange::ZERO_TO_ONE(0.0f, 1.0f);

FloatRange::FloatRange(float min, float max): m_min(min), m_max(max)
{
}

bool FloatRange::IsOnRange(float floatCompared) const
{
	return (floatCompared >= m_min) && (floatCompared <= m_max);
}

bool FloatRange::IsOverlappingWith(FloatRange rangeCompared) const
{
	return ((rangeCompared.m_min >= m_min) && (rangeCompared.m_min <= m_max)) || ((m_min >= rangeCompared.m_min) && (m_min <= rangeCompared.m_max));
}

void FloatRange::SetFromText(const char* text)
{
	Strings splitStrings = SplitStringOnDelimeter(text, '~');
	if (splitStrings.size() < 2)
		ERROR_RECOVERABLE("Setting FloatRange from text but string contains less than 2 floats");
	m_min = (float)atof(splitStrings[0].c_str());
	m_max = (float)atof(splitStrings[1].c_str());
}

bool FloatRange::GetOverlappingRange(FloatRange rangeCompared, FloatRange& out_overlappingRange) const
{
	if (!IsOverlappingWith(rangeCompared)) {
		return false;
	}
	out_overlappingRange.m_min = GetMax(m_min, rangeCompared.m_min);
	out_overlappingRange.m_max = GetMin(m_max, rangeCompared.m_max);
	return true;
}

bool FloatRange::operator==(const FloatRange& compare) const
{
	return (m_min == compare.m_min) && (m_max == compare.m_max);
}

bool FloatRange::operator!=(const FloatRange& compare) const
{
	return (m_min != compare.m_min) || (m_max != compare.m_max);
}

void FloatRange::operator=(const FloatRange& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}

float FloatRange::GetClamped(float input) const
{
	if (input <= m_min) {
		return m_min;
	}
	if (input >= m_max) {
		return m_max;
	}
	else {
		return input;
	}
}
