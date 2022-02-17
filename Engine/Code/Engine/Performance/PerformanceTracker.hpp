#pragma once
constexpr int FRAME_HISTORY_COUNT = 10;

//-----------------------------------------------------------------------------------------------
class Clock;


//-----------------------------------------------------------------------------------------------
struct PerformanceTrackerParams
{
	Clock* clock = nullptr;
};


//-----------------------------------------------------------------------------------------------
class PerformanceTracker
{
public:
	void StartUp( const PerformanceTrackerParams& params );
	void Update();
	void Render() const;

private:
	float GetAverageFPS() const;
	void UpdateFramesPerSecond();

	void RenderFPSCounter() const;

private:
	float m_fpsHistory[FRAME_HISTORY_COUNT];
	int m_fpsNextIdx = 0;
	float m_fpsHistorySum = 0.f;
	Clock* m_clock = nullptr;
};
