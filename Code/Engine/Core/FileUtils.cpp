#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

int FileReadToBuffer(std::vector<uint8_t>& outBuffer, const std::string& filename)
{
	FILE* file = nullptr;
	errno_t err = fopen_s(&file, filename.c_str(), "rb");
	if (err == 0 && file) {
		fseek(file, 0, SEEK_END);
		long fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		outBuffer.resize(fileSize);
		size_t bytesRead = fread(outBuffer.data(), 1, fileSize, file);

		if (bytesRead != static_cast<size_t>(fileSize)) {
			fclose(file);
			ERROR_RECOVERABLE(Stringf("Reading file: %s was unsuccessful", filename.c_str()));
			return -1;
		}
		fclose(file);
		return static_cast<int>(outBuffer.size());
	}
	else {
		ERROR_RECOVERABLE(Stringf("Unable to open file: %s when calling FileReadToBuffer()!", filename.c_str()));
		return -1;
	}
}

int FileReadToString(std::string& outString, const std::string& filename)
{
	std::vector<uint8_t> readFileBuffer;
	int readNumBytes = FileReadToBuffer(readFileBuffer, filename);
	outString.assign(readFileBuffer.begin(), readFileBuffer.end());
	return readNumBytes;
}

bool FileWriteFromBuffer(std::vector<uint8_t>& inBuffer, const std::string& fileName)
{
	FILE* file;
	errno_t err = fopen_s(&file, fileName.c_str(), "wb");
	if (err == 0 && file) {
		size_t bytesWritten = fwrite(inBuffer.data(), 1, inBuffer.size(), file);
		fclose(file);

		if (bytesWritten == inBuffer.size()) {
			return true;
		}
		else {
			ERROR_RECOVERABLE(Stringf("Writing to file: %s was unsuccessful!", fileName.c_str()));
			return false;
		}
	}
	else {
		ERROR_RECOVERABLE(Stringf("Unable to open file: %s when calling FileWriteFromBuffer()!", fileName.c_str()));
		return false;
	}
}

bool DoesFileExistOnDisk(const std::string& filePath)
{
	FILE* file;
	errno_t err = fopen_s(&file, filePath.c_str(), "rb");
	if (err == 0 && file) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}