#include <cstdlib>
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "ThirdParty/Squirrel/RawNoise.hpp"
#include "Engine/Math/Vec2.hpp"

int RandomNumberGenerator::RollRandomIntLessThan(int maxNotInclusive) {
	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_seed);
	return randomUInt % maxNotInclusive;
}

int RandomNumberGenerator::RollRandomIntInRange(int minInclusive, int maxInclusive) {
	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_seed);
	return randomUInt % (maxInclusive - minInclusive + 1) + minInclusive;
	/* //Squirrel's implementation
	*  int range = 1 + maxInclusive - minInclusive;
	*  return minInclusive + (rand() % range);
	*/
}

float RandomNumberGenerator::RollRandomFloatZeroToOne() {
	return Get1dNoiseZeroToOne(m_position++, m_seed);
	//unsigned int randomUInt = Get1dNoiseUint(m_position++, m_seed);
	//return float((double)randomUInt * ONE_OVER_MAX_RANDOM_UINT);
}

float RandomNumberGenerator::RollRandomFloatInRange(float minInclusive, float maxInclusive) {
	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_seed);
	return float((double)randomUInt * double(maxInclusive - minInclusive) * ONE_OVER_MAX_RANDOM_UINT + (double)minInclusive);
}

float RandomNumberGenerator::RollRandomFloatInRange(const FloatRange& floatRange)
{
	unsigned int randomUInt = Get1dNoiseUint(m_position++, m_seed);
	return float((double)randomUInt * double(floatRange.m_max - floatRange.m_min) * ONE_OVER_MAX_RANDOM_UINT + (double)floatRange.m_min);
}

Vec2 RandomNumberGenerator::RollRandomVec2InRange(const Vec2& minInclusive, const Vec2& maxInclusive)
{
	return Vec2(RollRandomFloatInRange(minInclusive.x, maxInclusive.x),
		RollRandomFloatInRange(minInclusive.y, maxInclusive.y));
}