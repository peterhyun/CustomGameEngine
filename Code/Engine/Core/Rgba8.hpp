#pragma once

typedef unsigned char uchar;

struct Vec4;

typedef struct Rgba8 {
public:
	Rgba8();
	explicit Rgba8(uchar r, uchar g, uchar b, uchar a=255);
	
	void SetFromText(char const* text);

	void GetAsFloats(float* colorAsFloats) const;

	Vec4 GetAsVec4() const;

	Rgba8 GetDarkenedColor(float lightenFactor = 0.89f, bool makeMoreTransparent = false) const;

	static Rgba8 InterpolateColors(const Rgba8& first, const Rgba8& second, float alpha);

	bool operator< (const Rgba8& other) const;
	bool operator!= (const Rgba8& other) const;

public:
	static const Rgba8 WHITE;
	static const Rgba8 RED;
	static const Rgba8 RED_TRANSLUCENT;
	static const Rgba8 BLUE;
	static const Rgba8 BLUE_TRANSLUCENT;
	static const Rgba8 PASTEL_BLUE;
	static const Rgba8 LIGHTGREEN;
	static const Rgba8 GREEN;
	static const Rgba8 GREEN_TRANSLUCENT;
	static const Rgba8 BLACK;
	static const Rgba8 GREY;
	static const Rgba8 GREY_TRANSLUCENT;
	static const Rgba8 CHELSEA_GREY;
	static const Rgba8 KENDALL_CHARCOAL;
	static const Rgba8 YELLOW;
	static const Rgba8 CYAN;
	static const Rgba8 MAGENTA;
	static const Rgba8 BLUEGREEN;
	static const Rgba8 TEAL;
	static const Rgba8 MAROON;
	static const Rgba8 ORANGE;
	static const Rgba8 PURPLE;
	static const Rgba8 PINK;
	static const Rgba8 PASTEL_PINK;

	uchar r = 255;
	uchar g = 255;
	uchar b = 255;
	uchar a = 255;
} Rgba8;