#include "FrameTimer.h"
#include "System.h"
#include "gs/Math/MathEx.h"
#include <cassert>

FrameTimer::FrameTimer()
	: m_isPaused(false)
	, m_minFrameDeltaTime(0.f)
	, m_maxFrameDeltaTime(0.f)
	, m_firstTimeStamp(0.f)
	, m_lastTimeStamp(0.f)
	, m_simFrameDeltaTime(0.f)
	, m_simElapsedTime(0.f)
	, m_realElapsedTime(0.f)
	, m_fps(0.f)
{
	SetMinFPS(NoLimit);
	SetMaxFPS(NoLimit);
}

void FrameTimer::Update()
{
	float64 currTimeStamp = System::GetElapsedSeconds();

	if (m_firstTimeStamp == 0.f)
		m_firstTimeStamp = currTimeStamp;

	if (m_lastTimeStamp == 0.f)
		m_lastTimeStamp = currTimeStamp;

	float32 frameDeltaTime = static_cast<float32>(currTimeStamp - m_lastTimeStamp); //@TODO: lossless_cast<float32>(...)

	while (frameDeltaTime < m_minFrameDeltaTime)
	{
		currTimeStamp = System::GetElapsedSeconds();
		frameDeltaTime = static_cast<float32>(currTimeStamp - m_lastTimeStamp);
	}

	m_lastTimeStamp = currTimeStamp;

	m_simFrameDeltaTime = m_isPaused? 0.f : frameDeltaTime;	
	m_simFrameDeltaTime = MathEx::Min(m_simFrameDeltaTime, m_maxFrameDeltaTime);

	m_realElapsedTime += frameDeltaTime;
	m_simElapsedTime += m_simFrameDeltaTime;

	static float32 FACTOR = 0.1f;
	const float32 instantFps = frameDeltaTime > 0.f? 1.0f / frameDeltaTime : 0.f;
	m_fps = m_fps + FACTOR * (instantFps - m_fps);
}
