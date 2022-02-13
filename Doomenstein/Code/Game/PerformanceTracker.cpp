#include "Game/PerformanceTracker.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Time/Clock.hpp"


//-----------------------------------------------------------------------------------------------
void PerformanceTracker::StartUp( const PerformanceTrackingSystemParams& params )
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
	float cummulativeFPS = 0.f;
	for ( int frameNum = 0; frameNum < FRAME_HISTORY_COUNT; ++frameNum )
	{
		cummulativeFPS += m_fpsHistory[frameNum];
	}

	return cummulativeFPS / (float)FRAME_HISTORY_COUNT;
}


//-----------------------------------------------------------------------------------------------
void PerformanceTracker::UpdateFramesPerSecond()
{
	for ( int frameNum = 0; frameNum < FRAME_HISTORY_COUNT - 1; ++frameNum )
	{
		m_fpsHistory[frameNum] = m_fpsHistory[frameNum + 1];
	}

	m_fpsHistory[FRAME_HISTORY_COUNT - 1] = 1.f / (float)m_clock->GetLastDeltaSeconds();
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
