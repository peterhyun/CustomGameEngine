#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

int ParseXmlAttribute(XmlElement const& element, char const* attributeName, int defaultValue)
{
	return element.IntAttribute(attributeName, defaultValue);
}

char ParseXmlAttribute(XmlElement const& element, char const* attributeName, char defaultValue)
{
	const char* retrievedAttribute = element.Attribute(attributeName);
	if (retrievedAttribute) {
		return retrievedAttribute[0];
	}
	else {
		DebuggerPrintf(Stringf("ParseXmlAttribute char default value used for %s\n", attributeName).c_str());
		return defaultValue;
	}
}

bool ParseXmlAttribute(XmlElement const& element, char const* attributeName, bool defaultValue)
{
	return element.BoolAttribute(attributeName, defaultValue);
}

float ParseXmlAttribute(XmlElement const& element, char const* attributeName, float defaultValue)
{
	return element.FloatAttribute(attributeName, defaultValue);
}

Rgba8 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue)
{
	const char * retrievedAttribute = element.Attribute(attributeName);
	if (retrievedAttribute) {
		Rgba8 rgba8;
		rgba8.SetFromText(retrievedAttribute);
		return rgba8;
	}
	else {
		DebuggerPrintf(Stringf("ParseXmlAttribute Rgba8 default value used for %s\n", attributeName).c_str());
		return defaultValue;
	}
}

Vec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec2 const& defaultValue)
{
	const char* retrievedAttribute = element.Attribute(attributeName);
	if (retrievedAttribute) {
		Vec2 vec2;
		vec2.SetFromText(retrievedAttribute);
		return vec2;
	}
	else {
		DebuggerPrintf(Stringf("ParseXmlAttribute Vec2 default value used for %s\n", attributeName).c_str());
		return defaultValue;
	}
}

IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue)
{
	const char* retrievedAttribute = element.Attribute(attributeName);
	if (retrievedAttribute) {
		IntVec2 intvec2;
		intvec2.SetFromText(retrievedAttribute);
		return intvec2;
	}
	else {
		DebuggerPrintf(Stringf("ParseXmlAttribute IntVec2 default value used for %s\n", attributeName).c_str());
		return defaultValue;
	}
}

Vec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec3 const& defaultValue)
{
	const char* retrievedAttribute = element.Attribute(attributeName);
	if (retrievedAttribute) {
		Vec3 vec3;
		vec3.SetFromText(retrievedAttribute);
		return vec3;
	}
	else {
		DebuggerPrintf(Stringf("ParseXmlAttribute Vec3 default value used for %s\n", attributeName).c_str());
		return defaultValue;
	}
}

EulerAngles ParseXmlAttribute(XmlElement const& element, char const* attributeName, EulerAngles const& defaultValue)
{
	const char* retrievedAttribute = element.Attribute(attributeName);
	if (retrievedAttribute) {
		EulerAngles eulerAngles;
		eulerAngles.SetFromText(retrievedAttribute);
		return eulerAngles;
	}
	else {
		DebuggerPrintf(Stringf("ParseXmlAttribute EulerAngles default value used for %s\n", attributeName).c_str());
		return defaultValue;
	}
}

FloatRange ParseXmlAttribute(XmlElement const& element, char const* attributeName, FloatRange const& defaultValue)
{
	const char* retrievedAttribute = element.Attribute(attributeName);
	if (retrievedAttribute) {
		FloatRange floatRange;
		floatRange.SetFromText(retrievedAttribute);
		return floatRange;
	}
	else {
		DebuggerPrintf(Stringf("ParseXmlAttribute FloatRange default value used for %s\n", attributeName).c_str());
		return defaultValue;
	}
}

std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue)
{
	const char* retrievedAttribute = element.Attribute(attributeName);
	if (retrievedAttribute) {
		return retrievedAttribute;
	}
	else {
		DebuggerPrintf(Stringf("ParseXmlAttribute std::string default value used for %s\n", attributeName).c_str());
		return defaultValue;
	}
}

std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, char const* defaultValue)
{
	const char* retrievedAttribute = element.Attribute(attributeName);
	if (retrievedAttribute) {
		return retrievedAttribute;
	}
	else {
		DebuggerPrintf(Stringf("ParseXmlAttribute std::string default value used for %s\n", attributeName).c_str());
		return defaultValue;
	}
}

Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValues)
{
	const char* retrievedAttribute = element.Attribute(attributeName);
	if (retrievedAttribute) {
		const Strings& stringVectorToReturn = SplitStringOnDelimeter(retrievedAttribute, ',');
		return stringVectorToReturn;
	}
	else {
		DebuggerPrintf(Stringf("ParseXmlAttribute Strings default value used for %s\n", attributeName).c_str());
		return defaultValues;
	}
}
