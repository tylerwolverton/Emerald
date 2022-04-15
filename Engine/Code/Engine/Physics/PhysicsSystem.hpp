#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/PhysicsScene.hpp"
#include "Engine/Physics/Collision.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct Manifold;
class Clock;
class Timer;
class Rigidbody;
class Collider;


//-----------------------------------------------------------------------------------------------
enum class eCollisionEventType
{
	ENTER,
	STAY,
	LEAVE
};


//-----------------------------------------------------------------------------------------------
class PhysicsSystem
{
public:
	void Startup( Clock* gameClock );
	void BeginFrame() {}
	void Update( PhysicsScene& scene );
	void EndFrame() {}
	void Shutdown();

	void Reset();

	float GetFixedDeltaSeconds() const;
	void SetFixedDeltaSeconds( float newDeltaSeconds );
	void ResetFixedDeltaSecondsToDefault();

	static bool SetPhysicsUpdateRate( EventArgs* args );

protected:
	void AdvanceSimulation( PhysicsScene& scene, float deltaSeconds );
	virtual void ApplyEffectors() = 0;
	void MoveRigidbodies( std::vector<Rigidbody>& rigidbodies, float deltaSeconds );
	void DetectCollisions(); 	
	void ClearOldCollisions(); 	
	void ResolveCollisions(); 	
	void ResolveCollision( const Collision& collision ); 	
	bool DoesCollisionInvolveATrigger( const Collision& collision ) const;
	void InvokeCollisionEvents( const Collision& collision, eCollisionEventType collisionType ) const;
	// TODO: Rename to my and theirs or something else that's clear
	/*virtual void CorrectCollidingRigidbodies( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold ) = 0;
	virtual void ApplyCollisionImpulses( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold ) = 0;*/

	void CleanupDestroyedObjects();
	void AddOrUpdateCollision( const Collision& collision );
	
private:
	Clock* m_gameClock = nullptr;
	Clock* m_physicsClock = nullptr;
	Timer* m_stepTimer = nullptr;
	uint m_frameNum = 0;
};
