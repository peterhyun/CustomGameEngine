#pragma once
#include <vector>

class Clock
{
public:
	Clock();

	explicit Clock(Clock& parent);

	~Clock();
	Clock(const Clock& copy) = delete;

	void Reset();
	bool IsPaused() const;
	void Pause();
	void Unpause();
	void TogglePause();

	void StepSingleFrame();
	void SetTimeScale(float timeScale);
	float GetTimeScale() const;

	float GetDeltaSeconds() const;
	float GetTotalSeconds() const;
	size_t GetFrameCount() const;

	void SetTotalSeconds(float newTotalSeconds);	//Manually change the clock

	Clock* GetParent() const;

public:
	static Clock& GetSystemClock();

	static void TickSystemClock();

protected:
	void Tick();

	void Advance(float deltaTimeSeconds);

	void AddChild(Clock& childClock);

	void RemoveChild(Clock& childClock);

protected:
	//I declared this one here just like Windows class
	static Clock s_systemClock;

	Clock* m_parent = nullptr;
	std::vector<Clock*> m_children;

	float m_lastUpdateTimeInSeconds = 0.0f;
	float m_totalSeconds = 0.0f;
	float m_deltaSeconds = 0.0f;
	size_t m_frameCount = 0;

	float m_timeScale = 1.0f;
	bool m_isPaused = false;
	bool m_stepSingleFrame = false;
	const float m_maxDeltaSeconds = 0.1f;
};