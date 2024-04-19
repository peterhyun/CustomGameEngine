#include "Engine/Core/HashedCaseInsensitiveString.hpp"
#include "thirdParty/Squirrel/RawNoise.hpp"

HashedCaseInsensitiveString::HashedCaseInsensitiveString(const std::string& originalText) : m_originalText(originalText), m_hash( CalcHashForText(originalText.c_str()))
{
}

HashedCaseInsensitiveString::HashedCaseInsensitiveString(const char* originalText) : m_originalText(originalText), m_hash (CalcHashForText(originalText))
{
}

bool HashedCaseInsensitiveString::operator<(const HashedCaseInsensitiveString& comparedRHS) const
{
	if (m_hash < comparedRHS.m_hash)
	{
		return true;
	}
	else if (m_hash > comparedRHS.m_hash)
	{
		return false;
	}
	else
	{
		//We need this jic we have colliding hashes
		return (_stricmp(m_originalText.c_str(), comparedRHS.m_originalText.c_str()) < 0);
	}
}

bool HashedCaseInsensitiveString::operator==(const HashedCaseInsensitiveString& comparedRHS) const
{
	if (m_hash != comparedRHS.m_hash) {
		return false;
	}
	else {
		return (_stricmp(m_originalText.c_str(), comparedRHS.m_originalText.c_str() ) == 0);
	}
}

bool HashedCaseInsensitiveString::operator!=(const HashedCaseInsensitiveString& comparedRHS) const
{
	if (m_hash != comparedRHS.m_hash) {
		return true;
	}
	else {
		return (_stricmp(m_originalText.c_str(), comparedRHS.m_originalText.c_str()) != 0);
	}
}

void HashedCaseInsensitiveString::Clear()
{
	m_originalText.clear();
	m_hash = 0;
}

size_t HashedCaseInsensitiveString::Length() const
{
	return m_originalText.length();
}

const char* HashedCaseInsensitiveString::c_str() const
{
	return m_originalText.c_str();
}

HashedCaseInsensitiveString::operator std::string() const
{
	return m_originalText;
}

unsigned int HashedCaseInsensitiveString::CalcHashForText(const char* text)
{
	unsigned int hash = 0;
	char const* readPos = text;
	while (*readPos != '\0') {
		hash *= 31;	//The 'best' hash function according to Squirrel
		hash += (unsigned int)tolower(*readPos);
		readPos++;
	}
	return hash;
}
