#include "Engine/Core/StopWatch.hpp"
#include "Engine/Core/Clock.hpp"

Stopwatch::Stopwatch(float duration) :m_clock(&Clock::GetSystemClock()), m_duration(duration)
{
	if(m_duration != 0.0f)
		m_invDuration = 1.0f / m_duration;
}

Stopwatch::Stopwatch(const Clock* clock, float duration) : m_clock(clock), m_duration(duration)
{
	if(m_duration != 0.0f)
		m_invDuration = 1.0f / m_duration;
}

void Stopwatch::Start()
{
	m_startTime = m_clock->GetTotalSeconds();
}

void Stopwatch::Restart()
{
	if (!IsStopped()) {
		m_startTime = m_clock->GetTotalSeconds();
	}
}

void Stopwatch::Stop()
{
	m_startTime = -1.0f;
}

float Stopwatch::GetElapsedTime() const
{
	if (IsStopped()) {
		return 0.0f;
	}
	else {
		return m_clock->GetTotalSeconds() - m_startTime;
	}
}

float Stopwatch::GetDuration() const
{
	return m_duration;
}

float Stopwatch::GetElapsedFraction() const
{
	return GetElapsedTime() * m_invDuration;
}

bool Stopwatch::IsStopped() const
{
	return (m_startTime == -1.0f);
}

bool Stopwatch::HasDurationElapsed() const
{
	return GetElapsedTime() >= m_duration;
}

//Call this in a loop
bool Stopwatch::DecrementDurationIfElapsed()
{
	if (HasDurationElapsed()) {
		m_startTime += m_duration;
		return true;
	}
	return false;
}
