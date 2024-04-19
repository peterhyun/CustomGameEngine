#pragma once

struct IntVec4
{
public:
	int x = 0;
	int y = 0;
	int z = 0;
	int w = 0;

public:
	~IntVec4() {};
	IntVec4() {};
	IntVec4(const IntVec4& copyFrom);
	explicit IntVec4(int initialX, int initialY, int initialZ, int initialW);

	static const IntVec4 ZERO;

	float GetLength() const;
	int GetTaxicabLength() const;
	int GetLengthSquared() const;
	void Clear();

	void SetFromText(char const* text);

	void operator=(const IntVec4& copyFrom);

	bool		operator==(const IntVec4& compare) const;		// vec2 == vec2
	bool		operator!=(const IntVec4& compare) const;		// vec2 != vec2
	const IntVec4	operator+(const IntVec4& vecToAdd) const;		// vec2 + vec2
	const IntVec4	operator-(const IntVec4& vecToSubtract) const;	// vec2 - vec2

	bool		operator<(const IntVec4& compare) const;

	int&		operator[](unsigned int index);
};