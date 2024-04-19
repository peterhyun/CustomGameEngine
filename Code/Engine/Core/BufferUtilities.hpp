#pragma once
#include <vector>
#include <string>

struct Vec2;
struct Vec3;
struct Rgba8;
struct AABB2;
struct Plane2D;
struct Vertex_PCU;

void Reverse2BytesInPlace(void* ptrTo16BitWord);
void Reverse4BytesInPlace(void* ptrTo4Bytes);
void Reverse8BytesInPlace(void* ptrTo8Bytes);

class BufferWriter
{
public:
	BufferWriter(std::vector<unsigned char>& buffer);

	void SetEndianness(bool isLittleEndian);
	void AppendByte(unsigned char byteToPush);
	void AppendChar(char charToPush);
	void AppendUShort(unsigned short uShortToPush);
	void AppendShort(short shortToPush);
	void AppendInt(int intToPush);
	void AppendUInt(unsigned int uIntToPush);
	void AppendInt64(long long int64ToPush);
	void AppendUInt64(size_t uInt64ToPush);
	void AppendFloat(float floatToPush);
	void AppendDouble(double doubleToPush);
	void AppendString(const std::string& stringToPush);	//Also have to push the null char (zero-byte) at the endd
	void AppendString(unsigned int numOfChars, const char* charsToPush); //No need to push the null char at the end

	void AppendVec2(const Vec2& vecToPush);
	void AppendVec3(const Vec3& vecToPush);
	void AppendRgba8(const Rgba8& rgba8ToPush);
	void AppendAABB2(const AABB2& aabb2ToPush);
	void AppendPlane2D(const Plane2D& planeToPush);
	void AppendVertex_PCU(const Vertex_PCU& vertex_PCUToPush);

	void WriteBytesToFilePath(const std::string& directoryPath, const std::string& fileNameWithExtension) const;

	void OverwriteUIntAtOffset(unsigned int offset, unsigned int uIntValueToOverwrite);

	unsigned int GetNumBytesSoFar() const;

private:
	std::vector<unsigned char>& m_buffer;
	bool m_isLittleEndian = true;
	bool m_isNativeEndiannessLittle = false;
};

class BufferParser
{
public:
	BufferParser(unsigned char const* bufferToParser, size_t bufferSizeInBytes);
	BufferParser(const std::vector<unsigned char>& buffer);

	void SetEndianness(bool isLittleEndian);

	void SetReadPos(size_t newReadPos);

	unsigned char ParseByte();
	char ParseChar();
	unsigned short ParseUShort();
	short ParseShort();
	int ParseInt();
	unsigned int ParseUInt();
	long long ParseInt64();
	size_t ParseUInt64();
	float ParseFloat();
	double ParseDouble();
	std::string ParseString();	//Reads until null character
	std::string ParseString(unsigned int stringLength);

	Vec2 ParseVec2();
	Vec3 ParseVec3();
	Rgba8 ParseRgba8();
	AABB2 ParseAABB2();
	Plane2D ParsePlane2D();
	Vertex_PCU ParseVertex_PCU();

private:
	unsigned char const* m_bufferStart = nullptr;
	size_t m_bufferSizeInBytes = 0;
	size_t m_currentReadPos = 0;
	bool m_isLittleEndian = true;
	bool m_isNativeEndiannessLittle = false;
};