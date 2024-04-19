#pragma once

class SpriteSheet;
class SpriteDefinition;

typedef enum class SpriteAnimPlaybackType
{
	INVALID = -1,
	ONCE,
	LOOP,
	PINGPONG,
	NUM_SPRITEANIMPLAYBACKTYPE
} SpriteAnimPlaybackType;

class SpriteAnimDefinition
{
public:
	SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex, float framesPerSecond = 20.f, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP);
	SpriteDefinition const& GetSpriteDefAtTime(float seconds) const;
	float					GetSecondsPerFrame() const;
	SpriteAnimPlaybackType	GetSpriteAnimPlaybackType() const;
	int						GetNumTotalFrames() const;
	void					SetSpriteAnimPlaybackType(SpriteAnimPlaybackType type);
	void					SetSpriteAnimStartIndex(int startSpriteIndex);
	void					SetSpriteAnimEndIndex(int endSpriteIndex);
	void					SetSecondsPerFrame(float secondsPerFrame);

private:
	SpriteSheet const&		m_spriteSheet;
	int						m_startSpriteIndex = -1;
	int						m_endSpriteIndex = -1;
	float					m_secondsPerFrame = 0.05f;
	SpriteAnimPlaybackType	m_playbackType = SpriteAnimPlaybackType::LOOP;
};