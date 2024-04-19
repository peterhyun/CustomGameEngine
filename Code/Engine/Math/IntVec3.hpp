#pragma once

struct IntVec3
{
public:
	int x = 0;
	int y = 0;
	int z = 0;

public:
	~IntVec3() {};
	IntVec3() {};
	IntVec3(const IntVec3& copyFrom);
	explicit IntVec3(int initialX, int initialY, int initialZ);

	static const IntVec3 ZERO;

	float GetLength() const;
	int GetTaxicabLength() const;
	int GetLengthSquared() const;

	void SetFromText(char const* text);

	void operator=(const IntVec3& copyFrom);

	bool		operator==(const IntVec3& compare) const;		// vec2 == vec2
	bool		operator!=(const IntVec3& compare) const;		// vec2 != vec2
	const IntVec3	operator+(const IntVec3& vecToAdd) const;		// vec2 + vec2
	const IntVec3	operator-(const IntVec3& vecToSubtract) const;	// vec2 - vec2
};