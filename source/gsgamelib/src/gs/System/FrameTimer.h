#ifndef _GAME_CLOCK_H_
#define _GAME_CLOCK_H_

#include "gs/Base/Base.h"

class FrameTimer
{
public:
	FrameTimer();

	enum { NoLimit = 0 };

	// Set the lowest FPS allowed. This effectively caps the frame delta time so that it can't
	// get too large. On slow machines, the net effect is slow motion.
	void SetMinFPS(float32 minFPS = NoLimit)	{ m_maxFrameDeltaTime = minFPS == NoLimit? NUM_MAX(m_maxFrameDeltaTime) : 1.f / minFPS; }

	// Set the largest FPS allowed. If the frame delta is very small (high FPS), the clock will
	// wait to achieve the desired max fps.
	void SetMaxFPS(float32 maxFPS = NoLimit)	{ m_minFrameDeltaTime = maxFPS == NoLimit? NUM_MIN(m_minFrameDeltaTime): 1.f / maxFPS; }

	// Call once per frame, even when paused
	void Update();

	// Simulation pause control
	void SetPaused(bool paused)					{ m_isPaused = paused; }
	void TogglePaused()							{ SetPaused(!IsPaused()); }
	bool IsPaused() const						{ return m_isPaused; }

	// Time since last call to Update
	float32 GetFrameDeltaTime() const			{ return m_simFrameDeltaTime; }

	// Total elapsed simulation time (real time - paused time)
	float64 GetElapsedTime() const				{ return m_simElapsedTime; }

	// Total elapsed time
	float64 GetRealElapsedTime() const			{ return m_realElapsedTime; }

	// Frame rate
	float32 GetFPS() const						{ return m_fps; }

private:
	bool m_isPaused;
	float32 m_minFrameDeltaTime;
	float32 m_maxFrameDeltaTime;
	float64 m_firstTimeStamp;
	float64 m_lastTimeStamp;
	float32 m_simFrameDeltaTime;
	float64 m_simElapsedTime;
	float64 m_realElapsedTime;
	float32 m_fps;
};

#endif // _GAME_CLOCK_H_
