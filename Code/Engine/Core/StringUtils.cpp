#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include <stdarg.h>
#include <sstream>
#include <iomanip>


//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

Strings SplitStringOnDelimeter(std::string const& originalString, char delimiterToSplitOn)
{
	Strings splitStrings;

	size_t stringStartIndex = 0;
	size_t stringEndIndex = originalString.find(delimiterToSplitOn);
	while (true) {
		splitStrings.push_back(originalString.substr(stringStartIndex, stringEndIndex - stringStartIndex));
		if (stringEndIndex == std::string::npos)
			break;
		stringStartIndex = stringEndIndex + 1;
		stringEndIndex = originalString.find(delimiterToSplitOn, stringStartIndex);
	}

	return splitStrings;
}

Strings SplitStringWithQuotes(const std::string& originalString, char delimiterToSplitOn)
{
	Strings splitStrings;
	size_t stringStartIndex = 0;
	bool isInsideQuotes = false;

	for (size_t i = 0; i < originalString.length(); ++i) {
		if (originalString[i] == '"') {
			isInsideQuotes = !isInsideQuotes;
		}

		if (!isInsideQuotes && originalString[i] == delimiterToSplitOn) {
			splitStrings.push_back(originalString.substr(stringStartIndex, i - stringStartIndex));
			stringStartIndex = i + 1;
		}
	}

	splitStrings.push_back(originalString.substr(stringStartIndex));

	return splitStrings;
}

std::string FormatFloatWithSign(float value)
{
	std::ostringstream oss;
	if (value >= 0.0f) {
		oss << "+";
	}
	oss << std::fixed << std::setprecision(2) << value;

	return oss.str();
}

std::string RemoveAllSubstringsIfExists(const std::string& originalString, const std::string& substringToErase)
{
	std::string copyOriginalString = originalString;
	size_t position = copyOriginalString.find(substringToErase);  // Find the starting position of the substring

	while (position != std::string::npos) {
		copyOriginalString.erase(position, substringToErase.length());  // Erase the substring
		position = copyOriginalString.find(substringToErase, position);  // Find the next occurrence of the substring
	}

	return copyOriginalString;
}

void TrimString(std::string& stringToTrim, char delimiterToTrim)
{
	if (stringToTrim == "") {
		return;
	}

	size_t originalStringLength = stringToTrim.length();
	size_t subStringStartIndex = 0;

	while (subStringStartIndex <= (originalStringLength - 1) && stringToTrim[subStringStartIndex] == delimiterToTrim) {
		subStringStartIndex++;
	}

	size_t subStringEndIndex = originalStringLength - 1;
	while (subStringEndIndex > subStringStartIndex && stringToTrim[subStringEndIndex] == delimiterToTrim) {
		subStringEndIndex--;
	}

	if (subStringStartIndex > subStringEndIndex) {
		stringToTrim = ""; // Set the string to an empty string
	}
	else {
		stringToTrim = stringToTrim.substr(subStringStartIndex, subStringEndIndex - subStringStartIndex + 1);
	}
}

int ConvertStringToInt(const std::string& text, bool& wasSuccessful)
{
	size_t pos = 0;
	int returnVal = 0;
	try {
		returnVal = std::stoi(text, &pos);
	}
	catch (const std::invalid_argument& e) {
		GUARANTEE_OR_DIE(text.size() != (size_t)-1, "Text is too long");
		pos = (size_t)-1;
		DebuggerPrintf(e.what());
	}
	wasSuccessful = (text.size() == pos);
	return returnVal;
}