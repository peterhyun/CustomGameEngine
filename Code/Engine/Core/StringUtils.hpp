#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

typedef std::vector<std::string> Strings;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... );
const std::string Stringf( int maxLength, char const* format, ... );

Strings SplitStringOnDelimeter(std::string const& originalString, char delimiterToSplitOn);
Strings SplitStringWithQuotes(const std::string& originalString, char delimiterToSplitOn);	//Split string but ignores delimiters inside double quotes
std::string FormatFloatWithSign(float value);
std::string RemoveAllSubstringsIfExists(const std::string& originalString, const std::string& substringToErase);
void TrimString(std::string& stringToTrim, char delimiterToTrim);	//Removes any occurene of delimiter from the front and back of the string

int ConvertStringToInt(const std::string& text, bool& wasSuccessful);