#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Vec3.hpp"
//#include "Engine/Physics/3D/Collision2D.hpp"

#include <vector>

//-----------------------------------------------------------------------------------------------
struct Manifold2;
class Polygon2;
class Clock;
class Timer;
class Rigidbody3D;
class Collider3D;


//-----------------------------------------------------------------------------------------------
enum class eCollisionEventType
{
	ENTER,
	STAY,
	LEAVE
};


//-----------------------------------------------------------------------------------------------
class PhysicsSystem3D
{
public:
	void Startup( Clock* gameClock );
	void BeginFrame() {}
	void Update();
	void EndFrame() {}
	void Shutdown();

	void Reset();

	// Factory style create/destroy
	Rigidbody3D* CreateRigidbody( float mass = 1.f );
	void DestroyRigidbody( Rigidbody3D* rigidbodyToDestroy );

	/*DiscCollider2D* CreateDiscCollider( const Vec2& localPosition, float radius );
	PolygonCollider2D* CreatePolygon2Collider( const Polygon2& polygon );
	void DestroyCollider( Collider2D* colliderToDestroy );*/

	//const Vec2 GetSceneGravity() { return m_forceOfGravity; }
	//void SetSceneGravity( const Vec3& forceOfGravity );
	//void SetSceneGravity( float forceOfGravityY );

	//bool DoLayersInteract( uint layer0, uint layer1 ) const;
	//void EnableLayerInteraction( uint layer0, uint layer1 );
	//void DisableLayerInteraction( uint layer0, uint layer1 );
	//void DisableAllLayerInteraction( uint layer );

	//float GetFixedDeltaSeconds() const;
	//void SetFixedDeltaSeconds( float newDeltaSeconds );
	//void ResetFixedDeltaSecondsToDefault();

	//static bool SetPhysicsUpdateRate( EventArgs* args );

private:
	void AdvanceSimulation( float deltaSeconds );
	void ApplyEffectors();
	void MoveRigidbodies( float deltaSeconds );
	void DetectCollisions();
	void ClearOldCollisions();
	void ResolveCollisions();
	//void ResolveCollision( const Collision3D& collision );
	//void InvokeCollisionEvents( const Collision2D& collision, eCollisionEventType collisionType ) const;
	// TODO: Rename to my and theirs or something else that's clear
	//void CorrectCollidingRigidbodies( Rigidbody2D* rigidbody1, Rigidbody2D* rigidbody2, const Manifold2& collisionManifold );
	//void ApplyCollisionImpulses( Rigidbody2D* rigidbody1, Rigidbody2D* rigidbody2, const Manifold2& collisionManifold );
	//float CalculateImpulseAgainstImmoveableObject( Rigidbody2D* moveableRigidbody, Rigidbody2D* immoveableRigidbody, const Manifold2& collisionManifold );
	//float CalculateImpulseBetweenMoveableObjects( Rigidbody2D* rigidbody1, Rigidbody2D* rigidbody2, const Manifold2& collisionManifold );

	//void AddOrUpdateCollision( const Collision3D& collision );

	void DestroyAllRigidbodies();
	void DestroyAllColliders();

	void CleanupDestroyedObjects();

private:
	Clock* m_gameClock = nullptr;
	Clock* m_physicsClock = nullptr;
	Timer* m_stepTimer = nullptr;
	uint m_frameNum = 0;

	std::vector<Rigidbody3D*> m_rigidbodies;
	std::vector<int> m_garbageRigidbodyIndexes;
	std::vector<Collider3D*> m_colliders;
	std::vector<int> m_garbageColliderIndexes;

	//std::vector<Collision3D> m_collisions;

	Vec3 m_forceOfGravity = Transform::GetWorldUpVector() * -9.8f;

	//uint m_layerInteractions[32];
};