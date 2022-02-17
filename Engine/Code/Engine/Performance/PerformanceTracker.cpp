#include "Engine/Performance/PerformanceTracker.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Time/Clock.hpp"


//-----------------------------------------------------------------------------------------------
void PerformanceTracker::StartUp( const PerformanceTrackerParams& params )
{
	if ( params.clock == nullptr )
	{
		m_clock = Clock::GetMaster();
	}
	else
	{
		m_clock = params.clock;
	}

	for ( int frameNum = 0; frameNum < FRAME_HISTORY_COUNT - 1; ++frameNum )
	{
		m_fpsHistory[frameNum] = 60.f;
		m_fpsHistorySum += 60.f;
	}
}


//-----------------------------------------------------------------------------------------------
void PerformanceTracker::Update()
{
	UpdateFramesPerSecond();
}


//-----------------------------------------------------------------------------------------------
void PerformanceTracker::Render() const
{
	RenderFPSCounter();
}


//-----------------------------------------------------------------------------------------------
float PerformanceTracker::GetAverageFPS() const
{
	constexpr float oneOverFrameCount = 1.f / (float)FRAME_HISTORY_COUNT;

	return m_fpsHistorySum * oneOverFrameCount;
}


//-----------------------------------------------------------------------------------------------
void PerformanceTracker::UpdateFramesPerSecond()
{
	if ( m_clock->IsPaused() )
	{
		return;
	}

	float curFPS = 1.f / (float)m_clock->GetLastDeltaSeconds();

	if ( curFPS < 0 )
	{
		curFPS = 0;
	}
	else if ( curFPS > 99999.f )
	{
		curFPS = 99999.f;
	}

	m_fpsHistorySum -= m_fpsHistory[m_fpsNextIdx];
	m_fpsHistory[m_fpsNextIdx] = curFPS;
	m_fpsHistorySum += curFPS;

	++m_fpsNextIdx;
	if ( m_fpsNextIdx >= FRAME_HISTORY_COUNT )
	{
		m_fpsNextIdx = 0;
	}
}


//-----------------------------------------------------------------------------------------------
void PerformanceTracker::RenderFPSCounter() const
{
	float fps = GetAverageFPS();

	Rgba8 fpsCountercolor = Rgba8::GREEN;

	if ( fps < 30.f )
	{
		fpsCountercolor = Rgba8::RED;
	}
	if ( fps < 55.f )
	{
		fpsCountercolor = Rgba8::YELLOW;
	}

	float frameTime = (float)m_clock->GetLastDeltaSeconds() * 1000.f;

	DebugAddScreenTextf( Vec4( 0.75f, .97f, 0.f, 0.f ), Vec2::ZERO, 15.f, fpsCountercolor, 0.f,
						 "FPS: %.2f ( %.2f ms/frame )",
						 fps, frameTime );
}
