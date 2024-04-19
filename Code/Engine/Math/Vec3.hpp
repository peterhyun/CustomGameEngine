#pragma once

struct Vec2;
struct Vec4;
//-----------------------------------------------------------------------------------------------
struct Vec3
{
public: // NOTE: this is one of the few cases where we break both the "m_" naming rule AND the avoid-public-members rule
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;

public:
	// Construction/Destruction
	~Vec3() = default;												// destructor (do nothing)
	Vec3() = default;												// default constructor (do nothing)
	Vec3( const Vec3& copyFrom );							// copy constructor (from another vec3)
	explicit Vec3( float initialX, float initialY, float initialZ);		// explicit constructor (from x, y)
	explicit Vec3(const Vec2& copyFrom, float initialZ = 0.0f);
	explicit Vec3(const Vec4& copyFrom);

	static const Vec3 MakeFromPolarRadians(float latitudeRadians, float longtitudeRadians, float length = 1.0f);
	static const Vec3 MakeFromPolarDegrees(float latitudeDegrees, float longtitudeDegrees, float length = 1.0f);

	//Accessors (const methods)
	float GetLength() const;
	float GetLengthXY() const;
	float GetLengthSquared() const;
	float GetLengthXYSquared() const;
	float GetAngleAboutZRadians() const;
	float GetAngleAboutZDegrees() const;
	Vec3 const GetRotatedAboutZRadians(float deltaRadians) const;
	Vec3 const GetRotatedAboutZDegrees(float deltaDegrees) const;
	Vec3 const GetClamped(float maxLength) const;
	Vec3 const GetNormalized() const;

	void Normalize();

	void SetFromText(char const* text);

	// Operators (const)
	bool		operator==( const Vec3& compare ) const;		// vec3 == vec3
	bool		operator!=( const Vec3& compare ) const;		// vec3 != vec3
	const Vec3	operator+( const Vec3& vecToAdd ) const;		// vec3 + vec3
	const Vec3	operator-( const Vec3& vecToSubtract ) const;	// vec3 - vec3
	const Vec3	operator-() const;								// -vec3, i.e. "unary negation"
	const Vec3	operator*( float uniformScale ) const;			// vec3 * float
	const Vec3	operator*( const Vec3& vecToMultiply ) const;	// vec3 * vec3
	const Vec3	operator/( float inverseScale ) const;			// vec3 / float

	// Operators (self-mutating / non-const)
	void		operator+=( const Vec3& vecToAdd );				// vec3 += vec3
	void		operator-=( const Vec3& vecToSubtract );		// vec3 -= vec3
	void		operator*=( const float uniformScale );			// vec3 *= float
	void		operator/=( const float uniformDivisor );		// vec3 /= float
	void		operator=( const Vec3& copyFrom );				// vec3 = vec3
	void		operator=( const Vec2& copyFrom );

	// Standalone "friend" functions that are conceptually, but not actually, part of Vec3::
	friend const Vec3 operator*( float uniformScale, const Vec3& vecToScale );	// float * vec3

	// Operator I hacked to use it for an std::map
	bool		operator<(const Vec3& other) const;
};


