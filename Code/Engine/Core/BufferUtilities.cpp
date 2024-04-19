#include "Engine/Core/BufferUtilities.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Core/FileUtils.hpp"

BufferWriter::BufferWriter(std::vector<unsigned char>& buffer) : m_buffer(buffer)
{
	unsigned int endianCheckingInt = 0x12345678;
	unsigned int* addrOfEndianCheckingInt = &endianCheckingInt;
	unsigned char* addressOfEndianCheckingIntAsByteArray = reinterpret_cast<unsigned char*>(addrOfEndianCheckingInt);
	if (addressOfEndianCheckingIntAsByteArray[0] == 0x78) {
		m_isNativeEndiannessLittle = true;
	}
	else {
		m_isNativeEndiannessLittle = false;
	}
}

void BufferWriter::SetEndianness(bool isLittleEndian)
{
	m_isLittleEndian = isLittleEndian;
}

void BufferWriter::AppendByte(unsigned char byteToPush)
{
	m_buffer.push_back(byteToPush);
}

void BufferWriter::AppendChar(char c)
{
	m_buffer.push_back((unsigned char)c);
}

void BufferWriter::AppendUShort(unsigned short uShortToPush)
{
	GUARANTEE_OR_DIE(sizeof(unsigned short) == 2, "size of unsigned short != 2 in this architecture");
	unsigned short* addressOfUShort = &uShortToPush;
	unsigned char* addressOfUShortAsByteArray = reinterpret_cast<unsigned char*>(addressOfUShort);
	GUARANTEE_OR_DIE(addressOfUShortAsByteArray != nullptr, "reinterprest_cast<unsigned char*>(addressOfUShort) failed");
	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse2BytesInPlace(addressOfUShortAsByteArray);
	}
	AppendByte(addressOfUShortAsByteArray[0]);
	AppendByte(addressOfUShortAsByteArray[1]);
}

void BufferWriter::AppendShort(short shortToPush)
{
	GUARANTEE_OR_DIE(sizeof(short) == 2, "size of short != 2 in this architecture");
	short* addressOfShort = &shortToPush;
	unsigned char* addressOfShortAsByteArray = reinterpret_cast<unsigned char*>(addressOfShort);
	GUARANTEE_OR_DIE(addressOfShortAsByteArray != nullptr, "reinterprest_cast<unsigned char*>(addressOfShort) failed");
	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse2BytesInPlace(addressOfShortAsByteArray);
	}
	AppendByte(addressOfShortAsByteArray[0]);
	AppendByte(addressOfShortAsByteArray[1]);
}

void BufferWriter::AppendInt(int intToPush)
{
	GUARANTEE_OR_DIE(sizeof(int) == 4, "size of int != 4 in this architecture");
	int* addressOfInt = &intToPush;
	unsigned char* addressOfIntAsByteArray = reinterpret_cast<unsigned char*>(addressOfInt);
	GUARANTEE_OR_DIE(addressOfIntAsByteArray != nullptr, "reinterprest_cast<unsigned char*>(addressOfInt) failed");

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse4BytesInPlace(addressOfIntAsByteArray);
	}
	AppendByte(addressOfIntAsByteArray[0]);
	AppendByte(addressOfIntAsByteArray[1]);
	AppendByte(addressOfIntAsByteArray[2]);
	AppendByte(addressOfIntAsByteArray[3]);
}

void BufferWriter::AppendUInt(unsigned int uIntToPush)
{
	GUARANTEE_OR_DIE(sizeof(unsigned int) == 4, "size of unsigned int != 4 in this architecture");
	unsigned int* addressOfUInt = &uIntToPush;
	unsigned char* addressOfUIntAsByteArray = reinterpret_cast<unsigned char*>(addressOfUInt);
	GUARANTEE_OR_DIE(addressOfUIntAsByteArray != nullptr, "reinterprest_cast<unsigned char*>(addressOfUInt) failed");

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse4BytesInPlace(addressOfUIntAsByteArray);
	}
	AppendByte(addressOfUIntAsByteArray[0]);
	AppendByte(addressOfUIntAsByteArray[1]);
	AppendByte(addressOfUIntAsByteArray[2]);
	AppendByte(addressOfUIntAsByteArray[3]);
}

void BufferWriter::AppendInt64(long long int64ToPush)
{
	GUARANTEE_OR_DIE(sizeof(long long) == 8, "size of long long != 8 in this architecture");
	long long* addressOfInt64 = &int64ToPush;
	unsigned char* addressOfInt64AsByteArray = reinterpret_cast<unsigned char*>(addressOfInt64);
	GUARANTEE_OR_DIE(addressOfInt64AsByteArray != nullptr, "reinterprest_cast<unsigned char*>(addressOfInt64) failed");

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse8BytesInPlace(addressOfInt64AsByteArray);
	}

	AppendByte(addressOfInt64AsByteArray[0]);
	AppendByte(addressOfInt64AsByteArray[1]);
	AppendByte(addressOfInt64AsByteArray[2]);
	AppendByte(addressOfInt64AsByteArray[3]);
	AppendByte(addressOfInt64AsByteArray[4]);
	AppendByte(addressOfInt64AsByteArray[5]);
	AppendByte(addressOfInt64AsByteArray[6]);
	AppendByte(addressOfInt64AsByteArray[7]);
}

void BufferWriter::AppendUInt64(size_t uInt64ToPush)
{
	GUARANTEE_OR_DIE(sizeof(size_t) == 8, "size of unsigned long long != 8 in this architecture");
	size_t* addressOfUInt64 = &uInt64ToPush;
	unsigned char* addressOfUInt64AsByteArray = reinterpret_cast<unsigned char*>(addressOfUInt64);
	GUARANTEE_OR_DIE(addressOfUInt64AsByteArray != nullptr, "reinterprest_cast<unsigned char*>(addressOfUInt64) failed");

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse8BytesInPlace(addressOfUInt64AsByteArray);
	}

	AppendByte(addressOfUInt64AsByteArray[0]);
	AppendByte(addressOfUInt64AsByteArray[1]);
	AppendByte(addressOfUInt64AsByteArray[2]);
	AppendByte(addressOfUInt64AsByteArray[3]);
	AppendByte(addressOfUInt64AsByteArray[4]);
	AppendByte(addressOfUInt64AsByteArray[5]);
	AppendByte(addressOfUInt64AsByteArray[6]);
	AppendByte(addressOfUInt64AsByteArray[7]);
}

void BufferWriter::AppendFloat(float f)
{
	GUARANTEE_OR_DIE(sizeof(float) == 4, "size of float != 4 in this architecture");
	float* addressOfFloat = &f;
	unsigned char* addressOfFloatAsByteArray = reinterpret_cast<unsigned char*>(addressOfFloat);
	GUARANTEE_OR_DIE(addressOfFloatAsByteArray != nullptr, "reinterprest_cast<unsigned char*>(addressOfFloat) failed");

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse4BytesInPlace(addressOfFloatAsByteArray);
	}
	AppendByte(addressOfFloatAsByteArray[0]);
	AppendByte(addressOfFloatAsByteArray[1]);
	AppendByte(addressOfFloatAsByteArray[2]);
	AppendByte(addressOfFloatAsByteArray[3]);
}

void BufferWriter::AppendDouble(double doubleToPush)
{
	GUARANTEE_OR_DIE(sizeof(double) == 8, "size of double != 8 in this architecture");
	double* addressOfDouble = &doubleToPush;
	unsigned char* addressOfDoubleAsByteArray = reinterpret_cast<unsigned char*>(addressOfDouble);
	GUARANTEE_OR_DIE(addressOfDoubleAsByteArray != nullptr, "reinterprest_cast<unsigned char*>(addressOfDouble) failed");

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse8BytesInPlace(addressOfDoubleAsByteArray);
	}

	AppendByte(addressOfDoubleAsByteArray[0]);
	AppendByte(addressOfDoubleAsByteArray[1]);
	AppendByte(addressOfDoubleAsByteArray[2]);
	AppendByte(addressOfDoubleAsByteArray[3]);
	AppendByte(addressOfDoubleAsByteArray[4]);
	AppendByte(addressOfDoubleAsByteArray[5]);
	AppendByte(addressOfDoubleAsByteArray[6]);
	AppendByte(addressOfDoubleAsByteArray[7]);
}

void BufferWriter::AppendString(const std::string& stringToPush)
{
	for (char c : stringToPush) {
		AppendChar(c);
	}
	AppendChar('\0');
}

void BufferWriter::AppendString(unsigned int numOfChars, const char* charsToPush)
{
	for (unsigned int i = 0; i < numOfChars; ++i) {
		AppendChar(charsToPush[i]); // Appends each character byte to the buffer
	}
}

void BufferWriter::AppendVec2(const Vec2& vecToPush)
{
	AppendFloat(vecToPush.x);
	AppendFloat(vecToPush.y);
}

void BufferWriter::AppendVec3(const Vec3& vecToPush)
{
	AppendFloat(vecToPush.x);
	AppendFloat(vecToPush.y);
	AppendFloat(vecToPush.z);
}

void BufferWriter::AppendRgba8(const Rgba8& rgba8ToPush)
{
	AppendByte(rgba8ToPush.r);
	AppendByte(rgba8ToPush.g);
	AppendByte(rgba8ToPush.b);
	AppendByte(rgba8ToPush.a);
}

void BufferWriter::AppendAABB2(const AABB2& aabb2ToPush)
{
	AppendVec2(aabb2ToPush.m_mins);
	AppendVec2(aabb2ToPush.m_maxs);
}

void BufferWriter::AppendPlane2D(const Plane2D& planeToPush)
{
	AppendVec2(planeToPush.m_normal);
	AppendFloat(planeToPush.m_distFromOrigin);
}

void BufferWriter::AppendVertex_PCU(const Vertex_PCU& vertex_PCUToPush)
{
	AppendVec3(vertex_PCUToPush.m_position);
	AppendRgba8(vertex_PCUToPush.m_color);
	AppendVec2(vertex_PCUToPush.m_uvTexCoords);
}

void BufferWriter::WriteBytesToFilePath(const std::string& directoryPath, const std::string& fileNameWithExtension) const
{
	GUARANTEE_OR_DIE(fileNameWithExtension.length() > 0, "fileNameWithExtension should be provided!");
	if (FileWriteFromBuffer(m_buffer, directoryPath + "/" + fileNameWithExtension) == false) {
		ERROR_AND_DIE(Stringf("BufferWriterFileWriteFromBuffer(%s, %s)!", directoryPath.c_str(), fileNameWithExtension.c_str()));
	}
}

void BufferWriter::OverwriteUIntAtOffset(unsigned int offset, unsigned int uIntValueToOverwrite)
{
	GUARANTEE_OR_DIE(offset + sizeof(uIntValueToOverwrite) - 1 < m_buffer.size(), Stringf("OverwriteUIntAtOffset() called for invalid parameters, offset: %u, m_bufferSize: %u", (unsigned int)offset, (unsigned int)m_buffer.size()));
	unsigned int* ptrToUIntValue = &uIntValueToOverwrite;
	unsigned char* ptrAsUnsignedChars = reinterpret_cast<unsigned char*>(ptrToUIntValue);
	GUARANTEE_OR_DIE(ptrAsUnsignedChars != nullptr, "reinterpret_cast<unsigned char*>(ptrToUIntValue) failed!");
	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse4BytesInPlace(ptrAsUnsignedChars);
	}
	m_buffer[offset] = ptrAsUnsignedChars[0];
	m_buffer[offset + 1] = ptrAsUnsignedChars[1];
	m_buffer[offset + 2] = ptrAsUnsignedChars[2];
	m_buffer[offset + 3] = ptrAsUnsignedChars[3];
}

unsigned int BufferWriter::GetNumBytesSoFar() const
{
	return (unsigned int)m_buffer.size();
}

BufferParser::BufferParser(unsigned char const* bufferToParser, size_t bufferSizeInBytes) : m_bufferStart(bufferToParser), m_bufferSizeInBytes(bufferSizeInBytes)
{
	unsigned int endianCheckingInt = 0x12345678;
	unsigned int* addrOfEndianCheckingInt = &endianCheckingInt;
	unsigned char* addressOfEndianCheckingIntAsByteArray = reinterpret_cast<unsigned char*>(addrOfEndianCheckingInt);
	if (addressOfEndianCheckingIntAsByteArray[0] == 0x78) {
		m_isNativeEndiannessLittle = true;
	}
	else {
		m_isNativeEndiannessLittle = false;
	}
}

BufferParser::BufferParser(const std::vector<unsigned char>& buffer) : BufferParser(buffer.data(), buffer.size())
{
}

void BufferParser::SetEndianness(bool isLittleEndian)
{
	m_isLittleEndian = isLittleEndian;
}

void BufferParser::SetReadPos(size_t newReadPos)
{
	GUARANTEE_OR_DIE(newReadPos < m_bufferSizeInBytes, Stringf("newReadPos: %u whereas m_bufferSizeInBytes: u", newReadPos, m_bufferSizeInBytes));
	m_currentReadPos = newReadPos;
}

unsigned char BufferParser::ParseByte()
{
	if (m_currentReadPos > m_bufferSizeInBytes - 1) {
		ERROR_AND_DIE(Stringf("m_currentReadPos(%d) > m_bufferSizeInBytes(%d) - 1 for BufferParser", m_currentReadPos, m_bufferSizeInBytes));
	}
	return m_bufferStart[m_currentReadPos++];
}

char BufferParser::ParseChar()
{
	if (m_currentReadPos > m_bufferSizeInBytes - 1) {
		ERROR_AND_DIE(Stringf("m_currentReadPos(%d) > m_bufferSizeInBytes(%d) - 1 for BufferParser", m_currentReadPos, m_bufferSizeInBytes));
	}
	return (char)m_bufferStart[m_currentReadPos++];
}

unsigned short BufferParser::ParseUShort()
{
	GUARANTEE_OR_DIE(sizeof(unsigned short) == 2, "size of ushort != 2 on this architecture!");
	GUARANTEE_OR_DIE(m_currentReadPos + 1 < m_bufferSizeInBytes, Stringf("m_currentReadPos (%d) + 1 >= m_buffersizeInBytes (%d)!", m_currentReadPos, m_bufferSizeInBytes));
	unsigned char uShortBytes[2] = { m_bufferStart[m_currentReadPos], m_bufferStart[m_currentReadPos + 1] };
	m_currentReadPos += sizeof(unsigned short);

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse2BytesInPlace(uShortBytes);
	}

	unsigned short* bytesAsUShort = reinterpret_cast<unsigned short*>(uShortBytes);
	return *bytesAsUShort;
}

short BufferParser::ParseShort()
{
	GUARANTEE_OR_DIE(sizeof(short) == 2, "size of short != 2 on this architecture!");
	GUARANTEE_OR_DIE(m_currentReadPos + 1 < m_bufferSizeInBytes, Stringf("m_currentReadPos (%d) + 1 >= m_buffersizeInBytes (%d)!", m_currentReadPos, m_bufferSizeInBytes));
	unsigned char shortBytes[2] = { m_bufferStart[m_currentReadPos], m_bufferStart[m_currentReadPos + 1] };
	m_currentReadPos += sizeof(short);

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse2BytesInPlace(shortBytes);
	}

	short* bytesAsShort = reinterpret_cast<short*>(shortBytes);
	return *bytesAsShort;
}

int BufferParser::ParseInt()
{
	GUARANTEE_OR_DIE(sizeof(int) == 4, "size of int != 4 on this architecture!");
	GUARANTEE_OR_DIE(m_currentReadPos + 3 < m_bufferSizeInBytes, Stringf("m_currentReadPos (%d) + 3 >= m_buffersizeInBytes (%d)!", m_currentReadPos, m_bufferSizeInBytes));
	unsigned char intBytes[4] = { m_bufferStart[m_currentReadPos], m_bufferStart[m_currentReadPos + 1], m_bufferStart[m_currentReadPos + 2], m_bufferStart[m_currentReadPos + 3] };
	m_currentReadPos += sizeof(int);

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse4BytesInPlace(intBytes);
	}

	int* bytesAsInt = reinterpret_cast<int*>(intBytes);
	return *bytesAsInt;
}

unsigned int BufferParser::ParseUInt()
{
	GUARANTEE_OR_DIE(sizeof(unsigned int) == 4, "size of uint != 4 on this architecture!");
	GUARANTEE_OR_DIE(m_currentReadPos + 3 < m_bufferSizeInBytes, Stringf("m_currentReadPos (%d) + 3 >= m_buffersizeInBytes (%d)!", m_currentReadPos, m_bufferSizeInBytes));
	unsigned char uIntBytes[4] = { m_bufferStart[m_currentReadPos], m_bufferStart[m_currentReadPos + 1], m_bufferStart[m_currentReadPos + 2], m_bufferStart[m_currentReadPos + 3] };
	m_currentReadPos += sizeof(unsigned int);

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse4BytesInPlace(uIntBytes);
	}

	unsigned int* bytesAsUInt = reinterpret_cast<unsigned int*>(uIntBytes);
	return *bytesAsUInt;
}

long long BufferParser::ParseInt64()
{
	GUARANTEE_OR_DIE(sizeof(long long) == 8, "size of long long != 8 on this architecture!");
	GUARANTEE_OR_DIE(m_currentReadPos + 7 < m_bufferSizeInBytes, Stringf("m_currentReadPos (%d) + 7 >= m_buffersizeInBytes (%d)!", m_currentReadPos, m_bufferSizeInBytes));
	unsigned char int64Bytes[8] = { m_bufferStart[m_currentReadPos], m_bufferStart[m_currentReadPos + 1], m_bufferStart[m_currentReadPos + 2], m_bufferStart[m_currentReadPos + 3], m_bufferStart[m_currentReadPos + 4], m_bufferStart[m_currentReadPos + 5], m_bufferStart[m_currentReadPos + 6], m_bufferStart[m_currentReadPos + 7] };
	m_currentReadPos += sizeof(long long);

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse8BytesInPlace(int64Bytes);
	}

	long long* bytesAsInt64 = reinterpret_cast<long long*>(int64Bytes);
	return *bytesAsInt64;
}

size_t BufferParser::ParseUInt64()
{
	GUARANTEE_OR_DIE(sizeof(size_t) == 8, "size of unsigned long long != 8 on this architecture!");
	GUARANTEE_OR_DIE(m_currentReadPos + 7 < m_bufferSizeInBytes, Stringf("m_currentReadPos (%d) + 7 >= m_buffersizeInBytes (%d)!", m_currentReadPos, m_bufferSizeInBytes));
	unsigned char uInt64Bytes[8] = { m_bufferStart[m_currentReadPos], m_bufferStart[m_currentReadPos + 1], m_bufferStart[m_currentReadPos + 2], m_bufferStart[m_currentReadPos + 3], m_bufferStart[m_currentReadPos + 4], m_bufferStart[m_currentReadPos + 5], m_bufferStart[m_currentReadPos + 6], m_bufferStart[m_currentReadPos + 7] };
	m_currentReadPos += sizeof(size_t);

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse8BytesInPlace(uInt64Bytes);
	}

	size_t* bytesAsInt64 = reinterpret_cast<size_t*>(uInt64Bytes);
	return *bytesAsInt64;
}

float BufferParser::ParseFloat()
{
	GUARANTEE_OR_DIE(sizeof(float) == 4, "size of float != 4 on this architecture!");
	GUARANTEE_OR_DIE(m_currentReadPos + 3 < m_bufferSizeInBytes, Stringf("m_currentReadPos (%d) + 3 >= m_buffersizeInBytes (%d)!", m_currentReadPos, m_bufferSizeInBytes));
	unsigned char floatBytes[4] = {m_bufferStart[m_currentReadPos], m_bufferStart[m_currentReadPos + 1], m_bufferStart[m_currentReadPos + 2], m_bufferStart[m_currentReadPos + 3]};
	m_currentReadPos += sizeof(float);

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse4BytesInPlace(floatBytes);
	}

	float* bytesAsFloat = reinterpret_cast<float *>(floatBytes);
	return *bytesAsFloat;
}

double BufferParser::ParseDouble()
{
	GUARANTEE_OR_DIE(sizeof(double) == 8, "size of double != 8 on this architecture!");
	GUARANTEE_OR_DIE(m_currentReadPos + 7 < m_bufferSizeInBytes, Stringf("m_currentReadPos (%d) + 7 >= m_bufferSizeInBytes (%d)!", m_currentReadPos, m_bufferSizeInBytes));
	unsigned char doubleBytes[8] = {
		m_bufferStart[m_currentReadPos],
		m_bufferStart[m_currentReadPos + 1],
		m_bufferStart[m_currentReadPos + 2],
		m_bufferStart[m_currentReadPos + 3],
		m_bufferStart[m_currentReadPos + 4],
		m_bufferStart[m_currentReadPos + 5],
		m_bufferStart[m_currentReadPos + 6],
		m_bufferStart[m_currentReadPos + 7]
	};
	m_currentReadPos += sizeof(double);

	if (m_isLittleEndian != m_isNativeEndiannessLittle) {
		Reverse8BytesInPlace(doubleBytes);
	}

	double* bytesAsDouble = reinterpret_cast<double*>(doubleBytes);
	return *bytesAsDouble;
}

std::string BufferParser::ParseString()
{
	std::string result;
	while (m_currentReadPos < m_bufferSizeInBytes && m_bufferStart[m_currentReadPos] != '\0') {
		result += static_cast<char>(m_bufferStart[m_currentReadPos]);
		m_currentReadPos++;
	}
	m_currentReadPos++; // Skip the null terminator
	return result;
}

std::string BufferParser::ParseString(unsigned int stringLength)
{
	std::string result;
	for (unsigned int i = 0; i < stringLength; i++) {
		result += static_cast<char>(m_bufferStart[m_currentReadPos]);
		m_currentReadPos++;
	}
	return result;
}

Vec2 BufferParser::ParseVec2()
{
	float f1 = ParseFloat();
	float f2 = ParseFloat();
	return Vec2(f1, f2);
	//return Vec2(ParseFloat(), ParseFloat());	<- Does NOT guarantee that the left ParseFloat() will be executed prior to the right ParseFloat()
}

Vec3 BufferParser::ParseVec3()
{
	float f1 = ParseFloat();
	float f2 = ParseFloat();
	float f3 = ParseFloat();
	return Vec3(f1, f2, f3);
	//return Vec3(ParseFloat(), ParseFloat(), ParseFloat());
}

Rgba8 BufferParser::ParseRgba8()
{
	unsigned char byte1 = ParseByte();
	unsigned char byte2 = ParseByte();
	unsigned char byte3 = ParseByte();
	unsigned char byte4 = ParseByte();
	return Rgba8(byte1, byte2, byte3, byte4);
	//return Rgba8(ParseByte(), ParseByte(), ParseByte(), ParseByte());
}

AABB2 BufferParser::ParseAABB2()
{
	Vec2 mins = ParseVec2();
	Vec2 maxs = ParseVec2();
	return AABB2(mins, maxs);
}

Plane2D BufferParser::ParsePlane2D()
{
	Vec2 planeNormal = ParseVec2();
	float planeDist = ParseFloat();
	return Plane2D(planeNormal, planeDist);
}

Vertex_PCU BufferParser::ParseVertex_PCU()
{
	Vec3 pos = ParseVec3();
	Rgba8 color = ParseRgba8();
	Vec2 uv = ParseVec2();
	return Vertex_PCU(pos, color, uv);
}

void Reverse2BytesInPlace(void* ptrTo16BitWord)
{
	unsigned short u = *reinterpret_cast<unsigned short*>(ptrTo16BitWord);
	*(unsigned short*)ptrTo16BitWord = (u & 0x00ff) << 8 | 
									   (u & 0xff00) >> 8;
}

void Reverse4BytesInPlace(void* ptrTo4Bytes)
{
	unsigned int u = *reinterpret_cast<unsigned int*>(ptrTo4Bytes);
	*(unsigned int*)ptrTo4Bytes = (u & 0x000000ff) << 24 |
								  (u & 0x0000ff00) << 8 |
								  (u & 0x00ff0000) >> 8 |
								  (u & 0xff000000) >> 24;
}

void Reverse8BytesInPlace(void* ptrTo8Bytes)
{
	unsigned long long u = *reinterpret_cast<unsigned long long*>(ptrTo8Bytes);
	*(unsigned long long*)ptrTo8Bytes = 
		(u & 0x00000000000000ff) << 56 |
		(u & 0x000000000000ff00) << 40 |
		(u & 0x0000000000ff0000) << 24 |
		(u & 0x00000000ff000000) << 8  |
		(u & 0x000000ff00000000) >> 8  |
		(u & 0x0000ff0000000000) << 24 |
		(u & 0x00ff000000000000) >> 40 |
		(u & 0xff00000000000000) >> 56;
}
