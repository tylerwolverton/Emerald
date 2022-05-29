#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct PhysicsScene;
class Clock;
class Timer;


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
class PhysicsSystem
{
public:
	void Startup( Clock* gameClock );
	void Update( PhysicsScene& scene );
	void Shutdown();

	float GetFixedDeltaSeconds() const;
	void SetFixedDeltaSeconds( float newDeltaSeconds );
	void ResetFixedDeltaSecondsToDefault();

	static bool SetPhysicsUpdateRate( EventArgs* args );

protected:
	void AdvanceSimulation( PhysicsScene& scene, float deltaSeconds );
	void ApplyAffectors( RigidbodyVector& rigidbodies, const AffectorVector& affectors );
	void MoveRigidbodies( RigidbodyVector& rigidbodies, float deltaSeconds );
	
private:
	Clock* m_gameClock = nullptr;
	Clock* m_physicsClock = nullptr;
	Timer* m_stepTimer = nullptr;
	uint m_frameNum = 0;
};

#include "Engine/Physics/PhysicsSystem.inl"
