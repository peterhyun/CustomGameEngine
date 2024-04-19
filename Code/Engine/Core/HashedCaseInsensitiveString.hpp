#pragma once
#include <string>

class HashedCaseInsensitiveString
{
public:
	HashedCaseInsensitiveString() {};
	HashedCaseInsensitiveString(const std::string& originalText);
	HashedCaseInsensitiveString(const char* text);	//This is better than the above cause it avoids unnecessary memory allocation
	HashedCaseInsensitiveString(const HashedCaseInsensitiveString& copyFrom) = default;

	bool operator<(const HashedCaseInsensitiveString& comparedRHS) const;
	bool operator==(const HashedCaseInsensitiveString& comparedRHS) const;
	bool operator!=(const HashedCaseInsensitiveString& comparedRHS) const;

	void Clear();
	size_t Length() const;

	const char* c_str() const;

	operator std::string() const;

private:
	static unsigned int CalcHashForText(const char* text);

private:
	std::string m_originalText;
	unsigned int m_hash = 0;

};
typedef HashedCaseInsensitiveString HCIString;