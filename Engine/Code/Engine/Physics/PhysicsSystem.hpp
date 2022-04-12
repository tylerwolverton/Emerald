#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Collision.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
//template<typename VecType>
struct Manifold;
class Clock;
class Timer;
class Rigidbody;
class Collider;

//using Collision2D = Collision<Collider, Vec2>;
//using Manifold2D = Manifold<Vec2>;


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
	void Update();
	void EndFrame() {}
	void Shutdown();

	void Reset();

	bool DoLayersInteract( uint layer0, uint layer1 ) const;
	void EnableLayerInteraction( uint layer0, uint layer1 );
	void DisableLayerInteraction( uint layer0, uint layer1 );
	void DisableAllLayerInteraction( uint layer );

	float GetFixedDeltaSeconds() const;
	void SetFixedDeltaSeconds( float newDeltaSeconds );
	void ResetFixedDeltaSecondsToDefault();

	static bool SetPhysicsUpdateRate( EventArgs* args );

protected:
	void AdvanceSimulation( float deltaSeconds );
	virtual void ApplyEffectors() = 0;
	void MoveRigidbodies( float deltaSeconds );
	void DetectCollisions(); 	
	void ClearOldCollisions(); 	
	void ResolveCollisions(); 	
	void ResolveCollision( const Collision& collision ); 	
	bool DoesCollisionInvolveATrigger( const Collision& collision ) const;
	void InvokeCollisionEvents( const Collision& collision, eCollisionEventType collisionType ) const;
	// TODO: Rename to my and theirs or something else that's clear
	virtual void CorrectCollidingRigidbodies( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold ) = 0;
	virtual void ApplyCollisionImpulses( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold ) = 0;
	   
	void AddOrUpdateCollision( const Collision& collision );

	void DestroyAllRigidbodies();
	void DestroyAllColliders();
	void CleanupDestroyedObjects();  	
	
private:
	Clock* m_gameClock = nullptr;
	Clock* m_physicsClock = nullptr;
	Timer* m_stepTimer = nullptr;
	uint m_frameNum = 0;

	std::vector<Rigidbody*> m_rigidbodies;
	std::vector<int> m_garbageRigidbodyIndexes;
	std::vector<Collider*> m_colliders;
	std::vector<int> m_garbageColliderIndexes;

	std::vector<Collision> m_collisions;

	uint m_layerInteractions[32];
};
