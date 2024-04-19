#pragma once

struct FloatRange {
public:
	FloatRange() {};
	explicit FloatRange(float min, float max);
	~FloatRange() {};

	bool IsOnRange(float floatCompared) const;
	bool IsOverlappingWith(FloatRange rangeCompared) const;

	void SetFromText(const char* text);
	bool GetOverlappingRange(FloatRange rangeCompared, FloatRange& out_overlappingRange) const;

	bool		operator==(const FloatRange& compare) const;		// vec2 == vec2
	bool		operator!=(const FloatRange& compare) const;		// vec2 != vec2
	void		operator=(const FloatRange& copyFrom);				// vec2 = vec2

	float GetClamped(float input) const;

public:
	static const FloatRange ZERO;
	static const FloatRange ONE;
	static const FloatRange ZERO_TO_ONE;
	float m_min = 0.0f;
	float m_max = 0.0f;
};