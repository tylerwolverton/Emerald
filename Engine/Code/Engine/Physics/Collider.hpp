#pragma once
#include "Engine/Core/Delegate.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"


//-----------------------------------------------------------------------------------------------
struct Vec2;
struct Rgba8;
struct Manifold;
class PhysicsSystem;
class Rigidbody;
class RenderContext;
class Collider;


//-----------------------------------------------------------------------------------------------
enum eColliderType
{
	COLLIDER_NONE = -1,

	// 2D
	COLLIDER_DISC = 0,
	COLLIDER_POLYGON,
	
	NUM_2D_COLLIDER_TYPES,

	// 3D
	COLLIDER_SPHERE = 0,
	COLLIDER_OBB3,

	NUM_3D_COLLIDER_TYPES
};


//-----------------------------------------------------------------------------------------------
// Interface for all Collider objects used with our Physics system
class Collider
{
	friend class PhysicsScene;
	friend class Rigidbody;

public: 
	int GetId()	const															{ return m_id; }
	eColliderType GetType()	const												{ return m_type; }
	Rigidbody* GetRigidbody() const												{ return m_rigidbody; }
	Vec3 GetWorldPosition() const												{ return m_worldPosition; }
	void SetPhysicsMaterial( const PhysicsMaterial& material )					{ m_material = material; }
	bool IsTrigger() const														{ return m_isTrigger; }

	// cache off the world shape representation of this object
	// taking into account the owning rigidbody (if no owner, local is world)
	virtual void UpdateWorldShape() = 0;

	// queries 
	virtual const Vec3 GetClosestPoint( const Vec2& pos ) const;
	virtual const Vec3 GetClosestPoint( const Vec3& pos ) const = 0;
	virtual bool Contains( const Vec2& pos ) const;
	virtual bool Contains( const Vec3& pos ) const = 0;

	//virtual Manifold GetCollisionManifold( const Collider* other ) const = 0;
	float GetBounceWith( const Collider* otherCollider ) const;
	float GetFrictionWith( const Collider* otherCollider ) const;

	virtual float CalculateMoment( float mass ) = 0;

	//virtual Vec3 GetFarthestPointInDirection( const Vec2& direction ) const;
	//virtual Vec3 GetFarthestPointInDirection( const Vec3& direction ) const = 0;

	void Enable()																{ m_isEnabled = true; }
	void Disable()																{ m_isEnabled = false; }
	bool IsEnabled() const														{ return m_isEnabled; }

	// TODO: Move this to a generic AABB2 method
	/*virtual unsigned int CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const = 0;
	virtual const AABB2 GetWorldBounds() const																				{ return m_worldBounds; };*/

	// debug helpers
	virtual void DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) const = 0;

protected:
	virtual ~Collider() {}; // private - make sure this is virtual so correct deconstructor gets called

public:
	// TODO: Privatize these?
	/*Delegate<Collision> m_onOverlapEnterDelegate;
	Delegate<Collision> m_onOverlapStayDelegate;
	Delegate<Collision> m_onOverlapLeaveDelegate;

	Delegate<Collision> m_onTriggerEnterDelegate;
	Delegate<Collision> m_onTriggerStayDelegate;
	Delegate<Collision> m_onTriggerLeaveDelegate;*/

protected:
	PhysicsSystem* m_system		= nullptr;			
	Rigidbody* m_rigidbody		= nullptr;			// owning rigidbody, used for calculating world shape
	eColliderType m_type		= COLLIDER_NONE;	// keep track of the type - will help with collision later
	PhysicsMaterial m_material;
	int m_id					= -1;
	bool m_isEnabled			= true;
	bool m_isTrigger			= false;

	//AABB2 m_worldBounds;
	Vec3	m_localPosition; // my local offset from my parent
	Vec3	m_worldPosition; // calculated from local position and owning rigidbody if present
};
