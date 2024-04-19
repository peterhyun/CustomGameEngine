#include "Engine/Core/Clock.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"

Clock Clock::s_systemClock;

Clock::Clock()
{
	if (this != &s_systemClock) {
		s_systemClock.AddChild(*this);
	}
}

Clock::Clock(Clock& parent)
{
	parent.AddChild(*this);
}

void Clock::Reset()
{
	m_totalSeconds = 0.0f;
	m_deltaSeconds = 0.0f;
	m_frameCount = 0;

	//Get the current time as the last updated time
	m_lastUpdateTimeInSeconds = (float)GetCurrentTimeSeconds();
}

bool Clock::IsPaused() const
{
	return m_isPaused;
}

void Clock::Pause()
{
	m_isPaused = true;
}

void Clock::Unpause()
{
	m_isPaused = false;
}

void Clock::TogglePause()
{
	m_isPaused = !m_isPaused;
}

void Clock::StepSingleFrame()
{
	m_isPaused = false;
	m_stepSingleFrame = true;
}

void Clock::SetTimeScale(float timeScale)
{
	m_timeScale = timeScale;
}

float Clock::GetTimeScale() const
{
	return m_timeScale;
}

float Clock::GetDeltaSeconds() const
{
	return m_deltaSeconds;
}

float Clock::GetTotalSeconds() const
{
	return m_totalSeconds;
}

size_t Clock::GetFrameCount() const
{
	return m_frameCount;
}

void Clock::SetTotalSeconds(float newTotalSeconds)
{
	m_totalSeconds = newTotalSeconds;
	m_lastUpdateTimeInSeconds = (float)GetCurrentTimeSeconds();
}

Clock* Clock::GetParent() const
{
	return m_parent;
}

Clock& Clock::GetSystemClock()
{
	return s_systemClock;
}

void Clock::TickSystemClock()
{
	s_systemClock.Tick();
}

void Clock::Tick()
{
	static float timePrevious = static_cast<float>(GetCurrentTimeSeconds());	//This line only gets called for the first time. That's the thing of initialization of static variables.
	float timeNow = static_cast<float>(GetCurrentTimeSeconds());
	float deltaSeconds = timeNow - timePrevious;
	if (deltaSeconds > m_maxDeltaSeconds)
	{
		deltaSeconds = m_maxDeltaSeconds;
	}
	timePrevious = timeNow;

	Advance(deltaSeconds);
}

void Clock::Advance(float deltaTimeSeconds)
{
	if (m_isPaused) {
		m_deltaSeconds = 0.0f;
	}
	else {
		m_deltaSeconds = deltaTimeSeconds * m_timeScale;
		m_lastUpdateTimeInSeconds = (float)GetCurrentTimeSeconds();
		m_totalSeconds += m_deltaSeconds;
	}
	m_frameCount++;
	
	for (int i = 0; i < m_children.size() ; i++) {
		if (m_children[i]) {
			m_children[i]->Advance(m_deltaSeconds);
		}
	}
	//Have to do it last (for obvious reasons)
	if (m_stepSingleFrame) {
		m_stepSingleFrame = false;
		m_isPaused = true;
	}
}

void Clock::AddChild(Clock& childClock)
{
	if (childClock.m_parent)
		return;

	//Don't add the child if it's already registered as a child 
	if (std::find(m_children.begin(), m_children.end(), &childClock) != m_children.end())
		return;

	childClock.m_parent = this;
	m_children.push_back(&childClock);
}

void Clock::RemoveChild(Clock& childClock)
{
	if (childClock.m_parent == this) {
		childClock.m_parent = nullptr;
		auto foundChildClockIterator = std::find(m_children.begin(), m_children.end(), &childClock);
		if (foundChildClockIterator != m_children.end()) {
			m_children.erase(foundChildClockIterator);
		}
	}
}

Clock::~Clock()
{
	if (m_parent) {
		m_parent->RemoveChild(*this);
	}
	for (int i = 0; i < m_children.size() ; i++) {
		if (m_children[i]) {
			m_children[i]->m_parent = nullptr;
			//Reconnect parent and child now
			if (m_parent) {
				m_parent->AddChild(*m_children[i]);
			}
		}
	}
	m_parent = nullptr;
}
