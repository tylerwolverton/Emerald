#pragma once
constexpr int FRAME_HISTORY_COUNT = 10;

//-----------------------------------------------------------------------------------------------
class Clock;


//-----------------------------------------------------------------------------------------------
struct PerformanceTrackingSystemParams
{
	Clock* clock = nullptr;
};


//-----------------------------------------------------------------------------------------------
class PerformanceTracker
{
public:
	void StartUp( const PerformanceTrackingSystemParams& params );
	void Update();
	void Render() const;

private:
	float GetAverageFPS() const;
	void UpdateFramesPerSecond();

	void RenderFPSCounter() const;

private:
	float m_fpsHistory[FRAME_HISTORY_COUNT];
	Clock* m_clock = nullptr;
};
