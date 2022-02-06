#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Physics/Collision2D.hpp"

#include <vector>

//-----------------------------------------------------------------------------------------------
struct Manifold2;
class Polygon2;
class Clock;
class Timer;
class Rigidbody2D;
class Collider2D;
class DiscCollider2D;
class PolygonCollider2D;


//-----------------------------------------------------------------------------------------------
enum class eCollisionEventType
{
	ENTER,
	STAY,
	LEAVE
};


//-----------------------------------------------------------------------------------------------
class Physics2D
{
public:
	void Startup( Clock* gameClock );
	void BeginFrame() {}
	void Update();
	void EndFrame() {}
	void Shutdown();

	void Reset();

	// Factory style create/destroy
	Rigidbody2D* CreateRigidbody();
	void DestroyRigidbody( Rigidbody2D* rigidbodyToDestroy );

	DiscCollider2D* CreateDiscCollider( const Vec2& localPosition, float radius );
	DiscCollider2D* CreateDiscTrigger( const Vec2& localPosition, float radius );
	PolygonCollider2D* CreatePolygon2Collider( const Polygon2& polygon );
	PolygonCollider2D* CreatePolygon2Trigger( const Polygon2& polygon );
	void DestroyCollider( Collider2D* colliderToDestroy );

	const Vec2 GetSceneGravity()															{ return m_forceOfGravity; }
	void SetSceneGravity( const Vec2& forceOfGravity );
	void SetSceneGravity( float forceOfGravityY );

	bool DoLayersInteract( uint layer0, uint layer1 ) const;
	void EnableLayerInteraction( uint layer0, uint layer1 );
	void DisableLayerInteraction( uint layer0, uint layer1 );
	void DisableAllLayerInteraction( uint layer );

	float GetFixedDeltaSeconds() const;
	void SetFixedDeltaSeconds( float newDeltaSeconds );
	void ResetFixedDeltaSecondsToDefault();

	static bool SetPhysicsUpdateRate( EventArgs* args );

private:
	void AdvanceSimulation( float deltaSeconds );
	void ApplyEffectors(); 	
	void MoveRigidbodies( float deltaSeconds );
	void DetectCollisions(); 	
	void ClearOldCollisions(); 	
	void ResolveCollisions(); 	
	void ResolveCollision( const Collision2D& collision ); 	
	bool DoesCollisionInvolveATrigger( const Collision2D& collision ) const;
	void InvokeCollisionEvents( const Collision2D& collision, eCollisionEventType collisionType ) const;
	// TODO: Rename to my and theirs or something else that's clear
	void CorrectCollidingRigidbodies( Rigidbody2D* rigidbody1, Rigidbody2D* rigidbody2, const Manifold2& collisionManifold );
	void ApplyCollisionImpulses( Rigidbody2D* rigidbody1, Rigidbody2D* rigidbody2, const Manifold2& collisionManifold );
	float CalculateImpulseAgainstImmoveableObject( Rigidbody2D* moveableRigidbody, Rigidbody2D* immoveableRigidbody, const Manifold2& collisionManifold );
	float CalculateImpulseBetweenMoveableObjects( Rigidbody2D* rigidbody1, Rigidbody2D* rigidbody2, const Manifold2& collisionManifold );
	   
	float GetRotationalThingOverMomentOfInertia( Rigidbody2D* rigidbody, const Manifold2& collisionManifold );

	void AddOrUpdateCollision( const Collision2D& collision );

	void DestroyAllRigidbodies();
	void DestroyAllColliders();

	void CleanupDestroyedObjects();  	
	
private:
	Clock* m_gameClock = nullptr;
	Clock* m_physicsClock = nullptr;
	Timer* m_stepTimer = nullptr;
	uint m_frameNum = 0;

	std::vector<Rigidbody2D*> m_rigidbodies;
	std::vector<int> m_garbageRigidbodyIndexes;
	std::vector<Collider2D*> m_colliders;
	std::vector<int> m_garbageColliderIndexes;

	std::vector<Collision2D> m_collisions;

	Vec2 m_forceOfGravity = Vec2( 0.f, -9.8f );

	uint m_layerInteractions[32];
};