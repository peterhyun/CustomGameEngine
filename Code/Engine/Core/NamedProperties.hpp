#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/HashedCaseInsensitiveString.hpp"
#include <string>
#include <sstream>
#include <map>
#include <typeinfo>

class NamedPropertyBase
{
	friend class NamedProperties;
protected:
	virtual ~NamedPropertyBase() {};
};

template<typename T>
class NamedPropertyOfType : public NamedPropertyBase	//This is the middle class that stores the values
{
	friend class NamedProperties;
public:
	T GetValue() const;

protected:
	NamedPropertyOfType(const T& value);
	T m_value;
};

class NamedProperties
{
public:
	void PopulateFromXmlElementAttributes(XmlElement const& element);
	template<typename T>
	void SetValue(const std::string& caseInsensitiveKeyName, const T& value);
	void SetValue(const std::string& caseInsensitiveKeyName, const char* value);
	template<typename T>
	T    GetValue(const std::string& caseInsensitiveKeyName, const T& defaultValue);
	size_t Num() const;
	bool HasKey(const std::string& caseInsensitiveKeyName) const;
	void Clear();
	
public:
	std::map<HashedCaseInsensitiveString, NamedPropertyBase*> m_keyValuePairs;
};

inline void NamedProperties::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	XmlAttribute const* attrib = element.FirstAttribute();
	while (attrib)
	{
		SetValue(attrib->Name(), std::string(attrib->Value()));
		attrib = attrib->Next();
	}
}

inline void NamedProperties::SetValue(const std::string& caseInsensitiveKeyName, const char* value)
{
	SetValue<std::string>(caseInsensitiveKeyName, std::string(value));
}

inline size_t NamedProperties::Num() const
{
	return m_keyValuePairs.size();
}

inline bool NamedProperties::HasKey(const std::string& keyName) const
{
	auto foundPair = m_keyValuePairs.find(keyName);
	return (foundPair != m_keyValuePairs.end());
}

inline void NamedProperties::Clear()
{
	for (auto pair : m_keyValuePairs) {
		delete pair.second;
	}
	m_keyValuePairs.clear();
}

template<typename T>
inline void NamedProperties::SetValue(const std::string& keyName, const T& value)
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		NamedPropertyOfType<T>* typedProperty = dynamic_cast<NamedPropertyOfType<T>*>(foundPair->second);
		if (typedProperty != nullptr) {
			// If cast is successful, update the value
			typedProperty->m_value = value;
			return; // Value updated, so we can return early
		}
		else {
			// If cast failed, clean up the existing entry before replacing it
			delete foundPair->second;
		}
	}
	m_keyValuePairs[keyName] = new NamedPropertyOfType<T>(value);
}

template<typename T>
inline T NamedProperties::GetValue(const std::string& keyName, const T& defaultValue)
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		NamedPropertyBase* property = foundPair->second;
		NamedPropertyOfType<T>* typedProperty = dynamic_cast<NamedPropertyOfType<T>*>(property);
		if (typedProperty) {
			return typedProperty->m_value;
		}
		else {
			std::type_info const& typeInfoForDefaultVal = typeid(defaultValue);
			const char* keyName_c_str = keyName.c_str();
			const char* typeName = typeInfoForDefaultVal.name();
			ERROR_RECOVERABLE(Stringf("NamedProperties::GetValue KeyName: %s/ Requested type: %s", keyName_c_str, typeName));
			return defaultValue;
		}
	}
	else {
		return defaultValue;
	}
}

template<>
inline bool NamedProperties::GetValue<bool>(const std::string& keyName, const bool& defaultValue)
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		NamedPropertyBase* property = foundPair->second;
		NamedPropertyOfType<bool>* typedProperty = dynamic_cast<NamedPropertyOfType<bool>*>(property);
		if (typedProperty) {
			return typedProperty->m_value;
		}
		else {
			//Convert it to a string
			NamedPropertyOfType<std::string>* stringProperty = dynamic_cast<NamedPropertyOfType<std::string>*>(property);
			if (stringProperty) {
				std::string const& value = stringProperty->m_value;
				if (value == "true")
					return true;
				else if (value == "false")
					return false;
				else {
					ERROR_RECOVERABLE(Stringf("Tried converting the string %s to a bool", value.c_str()));
					return defaultValue;
				}
			}
			else {
				std::type_info const& typeInfoForDefaultVal = typeid(defaultValue);
				const char* keyName_c_str = keyName.c_str();
				const char* typeName = typeInfoForDefaultVal.name();
				ERROR_RECOVERABLE(Stringf("NamedProperties::GetValue KeyName: %s/ Requested type: %s", keyName_c_str, typeName));
				return defaultValue;
			}
		}
	}
	else {
		return defaultValue;
	}
}

template<>
inline float NamedProperties::GetValue<float>(const std::string& keyName, const float& defaultValue)
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		NamedPropertyBase* property = foundPair->second;
		NamedPropertyOfType<float>* typedProperty = dynamic_cast<NamedPropertyOfType<float>*>(property);
		if (typedProperty) {
			return typedProperty->m_value;
		}
		else {
			//Convert it to a string
			NamedPropertyOfType<std::string>* stringProperty = dynamic_cast<NamedPropertyOfType<std::string>*>(property);
			if (stringProperty) {
				std::string const& value = stringProperty->m_value;
				return (float)atof(value.c_str());
			}
			else {
				std::type_info const& typeInfoForDefaultVal = typeid(defaultValue);
				const char* keyName_c_str = keyName.c_str();
				const char* typeName = typeInfoForDefaultVal.name();
				ERROR_RECOVERABLE(Stringf("NamedProperties::GetValue KeyName: %s/ Requested type: %s", keyName_c_str, typeName));
				return defaultValue;
			}
		}
	}
	else {
		return defaultValue;
	}
}

template<>
inline int NamedProperties::GetValue<int>(const std::string& keyName, const int& defaultValue)
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		NamedPropertyBase* property = foundPair->second;
		NamedPropertyOfType<int>* typedProperty = dynamic_cast<NamedPropertyOfType<int>*>(property);
		if (typedProperty) {
			return typedProperty->m_value;
		}
		else {
			//Convert it to a string
			NamedPropertyOfType<std::string>* stringProperty = dynamic_cast<NamedPropertyOfType<std::string>*>(property);
			if (stringProperty) {
				std::string const& value = stringProperty->m_value;
				return atoi(value.c_str());
			}
			else {
				std::type_info const& typeInfoForDefaultVal = typeid(defaultValue);
				const char* keyName_c_str = keyName.c_str();
				const char* typeName = typeInfoForDefaultVal.name();
				ERROR_RECOVERABLE(Stringf("NamedProperties::GetValue KeyName: %s/ Requested type: %s", keyName_c_str, typeName));
				return defaultValue;
			}
		}
	}
	else {
		return defaultValue;
	}
}

template<>
inline double NamedProperties::GetValue<double>(const std::string& keyName, const double& defaultValue)
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		NamedPropertyBase* property = foundPair->second;
		NamedPropertyOfType<double>* typedProperty = dynamic_cast<NamedPropertyOfType<double>*>(property);
		if (typedProperty) {
			return typedProperty->m_value;
		}
		else {
			//Convert it to a string
			NamedPropertyOfType<std::string>* stringProperty = dynamic_cast<NamedPropertyOfType<std::string>*>(property);
			if (stringProperty) {
				std::string const& value = stringProperty->m_value;
				return atof(value.c_str());
			}
			else {
				std::type_info const& typeInfoForDefaultVal = typeid(defaultValue);
				const char* keyName_c_str = keyName.c_str();
				const char* typeName = typeInfoForDefaultVal.name();
				ERROR_RECOVERABLE(Stringf("NamedProperties::GetValue KeyName: %s/ Requested type: %s", keyName_c_str, typeName));
				return defaultValue;
			}
		}
	}
	else {
		return defaultValue;
	}
}

template<>
inline Rgba8 NamedProperties::GetValue<Rgba8>(const std::string& keyName, const Rgba8& defaultValue)
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		NamedPropertyBase* property = foundPair->second;
		NamedPropertyOfType<Rgba8>* typedProperty = dynamic_cast<NamedPropertyOfType<Rgba8>*>(property);
		if (typedProperty) {
			return typedProperty->m_value;
		}
		else {
			//Convert it to a string
			NamedPropertyOfType<std::string>* stringProperty = dynamic_cast<NamedPropertyOfType<std::string>*>(property);
			if (stringProperty) {
				std::string const& value = stringProperty->m_value;
				Rgba8 rgba8;
				rgba8.SetFromText(value.c_str());
				return rgba8;
			}
			else {
				std::type_info const& typeInfoForDefaultVal = typeid(defaultValue);
				const char* keyName_c_str = keyName.c_str();
				const char* typeName = typeInfoForDefaultVal.name();
				ERROR_RECOVERABLE(Stringf("NamedProperties::GetValue KeyName: %s/ Requested type: %s", keyName_c_str, typeName));
				return defaultValue;
			}
		}
	}
	else {
		return defaultValue;
	}
}

template<>
inline Vec2 NamedProperties::GetValue<Vec2>(const std::string& keyName, const Vec2& defaultValue)
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		NamedPropertyBase* property = foundPair->second;
		NamedPropertyOfType<Vec2>* typedProperty = dynamic_cast<NamedPropertyOfType<Vec2>*>(property);
		if (typedProperty) {
			return typedProperty->m_value;
		}
		else {
			//Convert it to a string
			NamedPropertyOfType<std::string>* stringProperty = dynamic_cast<NamedPropertyOfType<std::string>*>(property);
			if (stringProperty) {
				std::string const& value = stringProperty->m_value;
				Vec2 Vec2;
				Vec2.SetFromText(value.c_str());
				return Vec2;
			}
			else {
				std::type_info const& typeInfoForDefaultVal = typeid(defaultValue);
				const char* keyName_c_str = keyName.c_str();
				const char* typeName = typeInfoForDefaultVal.name();
				ERROR_RECOVERABLE(Stringf("NamedProperties::GetValue KeyName: %s/ Requested type: %s", keyName_c_str, typeName));
				return defaultValue;
			}
		}
	}
	else {
		return defaultValue;
	}
}

template<>
inline Vec3 NamedProperties::GetValue<Vec3>(const std::string& keyName, const Vec3& defaultValue)
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		NamedPropertyBase* property = foundPair->second;
		NamedPropertyOfType<Vec3>* typedProperty = dynamic_cast<NamedPropertyOfType<Vec3>*>(property);
		if (typedProperty) {
			return typedProperty->m_value;
		}
		else {
			//Convert it to a string
			NamedPropertyOfType<std::string>* stringProperty = dynamic_cast<NamedPropertyOfType<std::string>*>(property);
			if (stringProperty) {
				std::string const& value = stringProperty->m_value;
				Vec3 Vec3;
				Vec3.SetFromText(value.c_str());
				return Vec3;
			}
			else {
				std::type_info const& typeInfoForDefaultVal = typeid(defaultValue);
				const char* keyName_c_str = keyName.c_str();
				const char* typeName = typeInfoForDefaultVal.name();
				ERROR_RECOVERABLE(Stringf("NamedProperties::GetValue KeyName: %s/ Requested type: %s", keyName_c_str, typeName));
				return defaultValue;
			}
		}
	}
	else {
		return defaultValue;
	}
}

template<>
inline IntVec2 NamedProperties::GetValue<IntVec2>(const std::string& keyName, const IntVec2& defaultValue)
{
	auto foundPair = m_keyValuePairs.find(keyName);
	if (foundPair != m_keyValuePairs.end()) {
		NamedPropertyBase* property = foundPair->second;
		NamedPropertyOfType<IntVec2>* typedProperty = dynamic_cast<NamedPropertyOfType<IntVec2>*>(property);
		if (typedProperty) {
			return typedProperty->m_value;
		}
		else {
			//Convert it to a string
			NamedPropertyOfType<std::string>* stringProperty = dynamic_cast<NamedPropertyOfType<std::string>*>(property);
			if (stringProperty) {
				std::string const& value = stringProperty->m_value;
				IntVec2 IntVec2;
				IntVec2.SetFromText(value.c_str());
				return IntVec2;
			}
			else {
				std::type_info const& typeInfoForDefaultVal = typeid(defaultValue);
				const char* keyName_c_str = keyName.c_str();
				const char* typeName = typeInfoForDefaultVal.name();
				ERROR_RECOVERABLE(Stringf("NamedProperties::GetValue KeyName: %s/ Requested type: %s", keyName_c_str, typeName));
				return defaultValue;
			}
		}
	}
	else {
		return defaultValue;
	}
}

template<typename T>
inline T NamedPropertyOfType<T>::GetValue() const
{
	return m_value;
}

template<typename T>
NamedPropertyOfType<T>::NamedPropertyOfType(const T& value)
	: m_value(value)
{
}
