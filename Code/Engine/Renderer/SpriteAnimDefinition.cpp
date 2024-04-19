#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

SpriteAnimDefinition::SpriteAnimDefinition(SpriteSheet const& sheet, int startSpriteIndex, int endSpriteIndex, float framesPerSecond, SpriteAnimPlaybackType playbackType) :
	m_spriteSheet(sheet), m_startSpriteIndex(startSpriteIndex), m_endSpriteIndex(endSpriteIndex), m_secondsPerFrame(1.0f/framesPerSecond), m_playbackType(playbackType)
{
}

SpriteDefinition const& SpriteAnimDefinition::GetSpriteDefAtTime(float seconds) const
{
	float framesPerSecond = 1.0f / m_secondsPerFrame;
	int spriteIndexAfterSeconds = m_startSpriteIndex + int((framesPerSecond * seconds));
	if ((spriteIndexAfterSeconds <= m_endSpriteIndex) && (spriteIndexAfterSeconds >= m_startSpriteIndex))
		return m_spriteSheet.GetSpriteDef(spriteIndexAfterSeconds);

	int numSprites = m_endSpriteIndex - m_startSpriteIndex + 1;
	switch (m_playbackType) {
		case SpriteAnimPlaybackType::ONCE:
			if (spriteIndexAfterSeconds < 0)
				return m_spriteSheet.GetSpriteDef(m_startSpriteIndex);
			else
				return m_spriteSheet.GetSpriteDef(m_endSpriteIndex);
		case SpriteAnimPlaybackType::LOOP:
			return m_spriteSheet.GetSpriteDef((spriteIndexAfterSeconds - m_startSpriteIndex) % numSprites + m_startSpriteIndex);
		case SpriteAnimPlaybackType::PINGPONG: {
			//Imagine traversing back and forth (lap 0, 1, 2, ...)
			int lapIndex = (spriteIndexAfterSeconds - m_startSpriteIndex) / (numSprites - 1);
			int spriteIndexInThatLap = (spriteIndexAfterSeconds - m_startSpriteIndex) % (numSprites - 1);
			if (lapIndex % 2 == 0)
				return m_spriteSheet.GetSpriteDef(spriteIndexInThatLap + m_startSpriteIndex);
			else
				return m_spriteSheet.GetSpriteDef(numSprites - 1 - spriteIndexInThatLap + m_startSpriteIndex);
		}
		default:
			ERROR_AND_DIE("You have to define a SpriteAnimPlaybackType for SpriteAnimDefinition!");
	}
}

float SpriteAnimDefinition::GetSecondsPerFrame() const
{
	return m_secondsPerFrame;
}

SpriteAnimPlaybackType	SpriteAnimDefinition::GetSpriteAnimPlaybackType() const
{
	return m_playbackType;
}

int SpriteAnimDefinition::GetNumTotalFrames() const
{
	return m_endSpriteIndex - m_startSpriteIndex + 1;
}

void SpriteAnimDefinition::SetSpriteAnimPlaybackType(SpriteAnimPlaybackType type)
{
	m_playbackType = type;
}

void SpriteAnimDefinition::SetSpriteAnimStartIndex(int startSpriteIndex)
{
	m_startSpriteIndex = startSpriteIndex;
}

void SpriteAnimDefinition::SetSpriteAnimEndIndex(int endSpriteIndex)
{
	m_endSpriteIndex = endSpriteIndex;
}

void SpriteAnimDefinition::SetSecondsPerFrame(float secondsPerFrame)
{
	m_secondsPerFrame = secondsPerFrame;
}
