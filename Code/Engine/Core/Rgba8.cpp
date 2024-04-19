#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/MathUtils.hpp"

const Rgba8 Rgba8::WHITE = Rgba8(255, 255, 255, 255);
const Rgba8 Rgba8::RED = Rgba8(255, 0, 0, 255);
const Rgba8 Rgba8::RED_TRANSLUCENT = Rgba8(200, 0, 0, 180);
const Rgba8 Rgba8::LIGHTGREEN = Rgba8(224, 240, 227);
const Rgba8 Rgba8::GREEN = Rgba8(0, 255, 0, 255);
const Rgba8 Rgba8::GREEN_TRANSLUCENT = Rgba8(0, 200, 0, 180);
const Rgba8 Rgba8::PASTEL_BLUE = Rgba8(214, 233, 255);
const Rgba8 Rgba8::BLUE = Rgba8(0, 0, 255, 255);
const Rgba8 Rgba8::BLUE_TRANSLUCENT = Rgba8(0, 0, 200, 180);
const Rgba8 Rgba8::BLACK = Rgba8(0, 0, 0, 255);
const Rgba8 Rgba8::GREY = Rgba8(45, 45, 45, 255);
const Rgba8 Rgba8::GREY_TRANSLUCENT = Rgba8(60, 60, 60, 120);
const Rgba8 Rgba8::CHELSEA_GREY = Rgba8(134, 131, 123, 255);
const Rgba8 Rgba8::KENDALL_CHARCOAL = Rgba8(104, 102, 98, 255);
const Rgba8 Rgba8::YELLOW = Rgba8(255, 255, 0, 255);
const Rgba8 Rgba8::CYAN = Rgba8(0, 255, 255);
const Rgba8 Rgba8::MAGENTA = Rgba8(255, 0, 255);
const Rgba8 Rgba8::BLUEGREEN = Rgba8(13, 152, 186);
const Rgba8 Rgba8::TEAL = Rgba8(0, 128, 128);
const Rgba8 Rgba8::MAROON = Rgba8(128, 0, 0);
const Rgba8 Rgba8::ORANGE = Rgba8(255, 165, 0);
const Rgba8 Rgba8::PURPLE = Rgba8(115, 51, 128);
const Rgba8 Rgba8::PINK = Rgba8(242, 178, 183);
const Rgba8 Rgba8::PASTEL_PINK = Rgba8(252, 217, 217);

Rgba8::Rgba8(){}
Rgba8::Rgba8(uchar r, uchar g, uchar b, uchar a):r(r), g(g), b(b), a(a){}

void Rgba8::SetFromText(char const* text)
{
	Strings splitStrings = SplitStringOnDelimeter(text, ',');
	if (splitStrings.size() < 3)
		ERROR_RECOVERABLE("Setting Rgba8 from text but string contains less than 3 integers");
	r = (uchar)atoi(splitStrings[0].c_str());
	g = (uchar)atoi(splitStrings[1].c_str());
	b = (uchar)atoi(splitStrings[2].c_str());
	if (splitStrings.size() > 3)
		a = (uchar)atoi(splitStrings[3].c_str());
	else
		a = (uchar)255;
}

void Rgba8::GetAsFloats(float* colorAsFloats) const
{
	colorAsFloats[0] = RangeMapClamped((float)r, 0.0f, 255.0f, 0.0f, 1.0f);
	colorAsFloats[1] = RangeMapClamped((float)g, 0.0f, 255.0f, 0.0f, 1.0f);
	colorAsFloats[2] = RangeMapClamped((float)b, 0.0f, 255.0f, 0.0f, 1.0f);
	colorAsFloats[3] = RangeMapClamped((float)a, 0.0f, 255.0f, 0.0f, 1.0f);
}

Vec4 Rgba8::GetAsVec4() const
{
	Vec4 normalizedColor;
	normalizedColor.x = RangeMapClamped((float)r, 0.0f, 255.0f, 0.0f, 1.0f);
	normalizedColor.y = RangeMapClamped((float)g, 0.0f, 255.0f, 0.0f, 1.0f);
	normalizedColor.z = RangeMapClamped((float)b, 0.0f, 255.0f, 0.0f, 1.0f);
	normalizedColor.w = RangeMapClamped((float)a, 0.0f, 255.0f, 0.0f, 1.0f);
	return normalizedColor;
}

Rgba8 Rgba8::GetDarkenedColor(float lightenFactor, bool makeMoreTransparent) const
{
	return Rgba8(uchar((float)r * lightenFactor), uchar((float)g * lightenFactor), uchar((float)b * lightenFactor), uchar(makeMoreTransparent? a * lightenFactor : a));
}

Rgba8 Rgba8::InterpolateColors(const Rgba8& first, const Rgba8& second, float alpha)
{
	return Rgba8(
		(uchar)Interpolate((float)first.r, (float)second.r, alpha),
		(uchar)Interpolate((float)first.g, (float)second.g, alpha),
		(uchar)Interpolate((float)first.b, (float)second.b, alpha),
		(uchar)Interpolate((float)first.a, (float)second.a, alpha)
	);
}

bool Rgba8::operator<(const Rgba8& other) const
{
	if (r < other.r)
		return true;
	else if (r == other.r) {
		if (g < other.g)
			return true;
		else if (g == other.g) {
			if (b < other.b)
				return true;
			else
				return false;
		}
		else
			return false;
	}
	else
		return false;
}

bool Rgba8::operator!=(const Rgba8& other) const
{
	return (r != other.r) || (g != other.g) || (b != other.b) || (a != other.a);
}
