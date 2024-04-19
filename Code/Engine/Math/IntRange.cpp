#include "Engine/Math/IntRange.hpp"

const IntRange IntRange::ZERO(0, 0);
const IntRange IntRange::ONE(1, 1);
const IntRange IntRange::ZERO_TO_ONE(0, 1);

IntRange::IntRange(int min, int max):m_min(min), m_max(max)
{
}

bool IntRange::IsOnRange(int intCompared) const
{
	return (intCompared >= m_min) && (intCompared <= m_max);
}

bool IntRange::IsOverlappingWith(IntRange rangeCompared)
{
	return ((rangeCompared.m_min >= m_min) && (rangeCompared.m_min <= m_max)) || ((m_min >= rangeCompared.m_min) && (m_min <= rangeCompared.m_max));
}

bool IntRange::operator==(const IntRange& compare) const
{
	return (m_min == compare.m_min) && (m_max == compare.m_max);
}

bool IntRange::operator!=(const IntRange& compare) const
{
	return (m_min != compare.m_min) || (m_max != compare.m_max);
}

void IntRange::operator=(const IntRange& copyFrom)
{
	m_min = copyFrom.m_min;
	m_max = copyFrom.m_max;
}
