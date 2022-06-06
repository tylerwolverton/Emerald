#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Physics/PhysicsScene.hpp"
#include "Engine/Physics/CollisionResolvers/CollisionPolicies.hpp"


//-----------------------------------------------------------------------------------------------
class Clock;
class Timer;


//-----------------------------------------------------------------------------------------------
class PhysicsSystemBase
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
	virtual void AdvanceSimulation( PhysicsScene& scene, float deltaSeconds ) = 0;
	void ApplyAffectors( RigidbodyVector& rigidbodies, const AffectorMap& affectors );
	void MoveRigidbodies( RigidbodyVector& rigidbodies, float deltaSeconds );

protected:
	Clock* m_gameClock = nullptr;
	Clock* m_physicsClock = nullptr;
	Timer* m_stepTimer = nullptr;
	uint m_frameNum = 0;
};


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
class PhysicsSystem : public PhysicsSystemBase
{
protected:
	virtual void AdvanceSimulation( PhysicsScene& scene, float deltaSeconds ) override
	{
		ApplyAffectors( scene.rigidbodies, scene.affectors ); 									// apply gravity (or other scene wide effects) to all dynamic objects
		MoveRigidbodies( scene.rigidbodies, deltaSeconds ); 									// apply an euler step to all rigidbodies, and reset per-frame data
		CollisionPolicy::ResolveCollisions( scene.colliders, scene.collisions, m_frameNum );	// resolve all collisions, firing appropriate events
		scene.CleanupDestroyedObjects();  														// destroy objects 

		++m_frameNum;
	}
};
