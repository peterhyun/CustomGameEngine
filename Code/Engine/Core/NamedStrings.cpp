#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void NamedStrings::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	XmlAttribute const* attrib = element.FirstAttribute();
	while (attrib)
	{
		SetValue(attrib->Name(), attrib->Value());
		attrib = attrib->Next();
	}
}

void NamedStrings::SetValue(std::string const& keyName, std::string const& newValue)
{
	m_keyValuePairs[keyName] = newValue;
}

std::string NamedStrings::GetValue(std::string const& keyName, std::string const& defaultValue) const
{
    auto foundPair = m_keyValuePairs.find(keyName);
    if (foundPair != m_keyValuePairs.end())
        return foundPair->second;
	else {
		DebuggerPrintf(Stringf("Could not get value for %s", keyName.c_str()).c_str());
		return defaultValue;
	}
}

bool NamedStrings::GetValue(std::string const& keyName, bool defaultValue) const
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		std::string const& value = foundPair->second;
		if (value == "true")
			return true;
		else
			return false;
	}
	else {
		DebuggerPrintf(Stringf("Could not get value for %s", keyName.c_str()).c_str());
		return defaultValue;
	}
}

int NamedStrings::GetValue(std::string const& keyName, int defaultValue) const
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end())
		return atoi(foundPair->second.c_str());
	else {
		DebuggerPrintf(Stringf("Could not get value for %s", keyName.c_str()).c_str());
		return defaultValue;
	}
}

float NamedStrings::GetValue(std::string const& keyName, float defaultValue) const
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end())
		return (float)atof(foundPair->second.c_str());
	else {
		DebuggerPrintf(Stringf("Could not get value for %s", keyName.c_str()).c_str());
		return defaultValue;
	}
}

std::string NamedStrings::GetValue(std::string const& keyName, char const* defaultValue) const
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end())
		return foundPair->second;
	else {
		DebuggerPrintf(Stringf("Could not get value for %s", keyName.c_str()).c_str());
		return defaultValue;
	}
}

Rgba8 NamedStrings::GetValue(std::string const& keyName, Rgba8 const& defaultValue) const
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		Rgba8 rgba8;
		rgba8.SetFromText(foundPair->second.c_str());
		return rgba8;
	}
	else {
		DebuggerPrintf(Stringf("Could not get value for %s", keyName.c_str()).c_str());
		return defaultValue;
	}
}

Vec2 NamedStrings::GetValue(std::string const& keyName, Vec2 const& defaultValue) const
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		Vec2 vec2;
		vec2.SetFromText(foundPair->second.c_str());
		return vec2;
	}
	else {
		DebuggerPrintf(Stringf("Could not get value for %s", keyName.c_str()).c_str());
		return defaultValue;
	}
}

IntVec2 NamedStrings::GetValue(std::string const& keyName, IntVec2 const& defaultValue) const
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		IntVec2 intvec2;
		intvec2.SetFromText(foundPair->second.c_str());
		return intvec2;
	}
	else {
		DebuggerPrintf(Stringf("Could not get value for %s", keyName.c_str()).c_str());
		return defaultValue;
	}
}

Vec3 NamedStrings::GetValue(std::string const& keyName, Vec3 const& defaultValue) const
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		Vec3 vec3;
		vec3.SetFromText(foundPair->second.c_str());
		return vec3;
	}
	else {
		DebuggerPrintf(Stringf("Could not get value for %s", keyName.c_str()).c_str());
		return defaultValue;
	}
}

void NamedStrings::DebugPrintContents()
{
	for (auto it = m_keyValuePairs.begin(); it != m_keyValuePairs.end() ; it++) {
		DebuggerPrintf("[%s: %s]\n", it->first.c_str(), it->second.c_str());
	}
}
