#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/PhysicsScene.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct Manifold;
class Clock;
class Timer;
class Rigidbody;
class Collider;


//-----------------------------------------------------------------------------------------------
class PhysicsSystem
{
public:
	void Startup( Clock* gameClock );
	void BeginFrame() {}
	void Update( PhysicsSceneBase& scene );
	void EndFrame() {}
	void Shutdown();

	float GetFixedDeltaSeconds() const;
	void SetFixedDeltaSeconds( float newDeltaSeconds );
	void ResetFixedDeltaSecondsToDefault();

	static bool SetPhysicsUpdateRate( EventArgs* args );

protected:
	void AdvanceSimulation( PhysicsSceneBase& scene, float deltaSeconds );
	void ApplyAffectors( RigidbodyVector& rigidbodies, const AffectorVector& affectors );
	void MoveRigidbodies( RigidbodyVector& rigidbodies, float deltaSeconds );
	
private:
	Clock* m_gameClock = nullptr;
	Clock* m_physicsClock = nullptr;
	Timer* m_stepTimer = nullptr;
	uint m_frameNum = 0;
};
