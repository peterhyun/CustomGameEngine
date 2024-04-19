#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include <string>
#include <vector>

int FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& fileName);
int FileReadToString(std::string& outString, const std::string& fileName);
bool FileWriteFromBuffer(std::vector<unsigned char>& inBuffer, const std::string& fileName);
bool DoesFileExistOnDisk(const std::string& filePath);