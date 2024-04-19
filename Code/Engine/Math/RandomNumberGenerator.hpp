#pragma once
struct FloatRange;
struct Vec2;

class RandomNumberGenerator {
public:
	RandomNumberGenerator(unsigned int seed = 0): m_seed(seed) {};
	int RollRandomIntLessThan(int maxNotInclusive);
	int RollRandomIntInRange(int minInclusive, int maxInclusive);
	float RollRandomFloatZeroToOne();
	float RollRandomFloatInRange(float minInclusive, float maxInclusive);
	float RollRandomFloatInRange(const FloatRange& floatRange);
	Vec2 RollRandomVec2InRange(const Vec2& minInclusive, const Vec2& maxInclusive);

public:
	static constexpr double ONE_OVER_MAX_RANDOM_UINT = 1.0 / (double)0xFFFFFFFF;
	unsigned int m_seed = 0;
	int m_position = 0;
};