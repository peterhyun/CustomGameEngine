#pragma once

struct IntRange {
public:
	IntRange() {};
	explicit IntRange(int min, int max);
	~IntRange() {};

	bool IsOnRange(int intCompared) const;
	bool IsOverlappingWith(IntRange rangeCompared);

	bool		operator==(const IntRange& compare) const;		// vec2 == vec2
	bool		operator!=(const IntRange& compare) const;		// vec2 != vec2
	void		operator=(const IntRange& copyFrom);				// vec2 = vec2

public:
	static const IntRange ZERO;
	static const IntRange ONE;
	static const IntRange ZERO_TO_ONE;
	int m_min = 0;
	int m_max = 0;
};