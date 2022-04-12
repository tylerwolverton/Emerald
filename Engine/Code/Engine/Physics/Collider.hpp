#pragma once
#include "Engine/Core/Delegate.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Physics/PhysicsMaterial.hpp"


//-----------------------------------------------------------------------------------------------
struct Vec2;
struct Rgba8;
//template<typename VecType> 
struct Manifold;
class PhysicsSystem;
class Rigidbody;
class RenderContext;
class Collider;
//template<typename ColliderType, typename VecType>
class Collision;

//using Collision2D = Collision<Collider, Vec2>;
//using Manifold2D = Manifold<Vec2>;

//-----------------------------------------------------------------------------------------------
enum eCollider2DType
{
	COLLIDER2D_NONE = -1,
	COLLIDER2D_DISC,
	COLLIDER2D_POLYGON,

	NUM_COLLIDER_TYPES
};


//-----------------------------------------------------------------------------------------------
// Interface for all Collider objects used with our Physics system
class Collider
{
	friend class PhysicsSystem;
	friend class Rigidbody;

public: // Interface 
	int GetId()																	{ return m_id; }
	// cache off the world shape representation of this object
	// taking into account the owning rigidbody (if no owner, local is world)
	virtual void UpdateWorldShape() = 0;

	// queries 
	virtual const Vec3 GetClosestPoint( const Vec3& pos ) const = 0;
	virtual const Vec3 GetClosestPoint( const Vec2& pos ) const = 0;
	virtual bool Contains( const Vec2& pos ) const = 0;
	virtual bool Contains( const Vec3& pos ) const = 0;
	bool Intersects( const Collider* other ) const;

	virtual Manifold GetCollisionManifold( const Collider* other ) const = 0;
	float GetBounceWith( const Collider* otherCollider ) const;
	float GetFrictionWith( const Collider* otherCollider ) const;

	virtual float CalculateMoment( float mass ) = 0;

	virtual Vec3 GetFarthestPointInDirection( const Vec3& direction ) const = 0;
	virtual Vec3 GetFarthestPointInDirection( const Vec2& direction ) const = 0;

	void Enable()																{ m_isEnabled = true; }
	void Disable()																{ m_isEnabled = false; }
	bool IsEnabled() const														{ return m_isEnabled; }

	// TODO: Move this to a generic AABB2 method
	/*virtual unsigned int CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const = 0;
	virtual const AABB2 GetWorldBounds() const																				{ return m_worldBounds; };*/

	// debug helpers
	virtual void DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const = 0;

protected:
	virtual ~Collider() {}; // private - make sure this is virtual so correct deconstructor gets called

public:
	// TODO: Privatize these?
	Delegate<Collision> m_onOverlapEnterDelegate;
	Delegate<Collision> m_onOverlapStayDelegate;
	Delegate<Collision> m_onOverlapLeaveDelegate;

	Delegate<Collision> m_onTriggerEnterDelegate;
	Delegate<Collision> m_onTriggerStayDelegate;
	Delegate<Collision> m_onTriggerLeaveDelegate;

protected:
	PhysicsSystem* m_system		= nullptr;			
	Rigidbody* m_rigidbody		= nullptr;			// owning rigidbody, used for calculating world shape
	eCollider2DType m_type		= COLLIDER2D_NONE;  // keep track of the type - will help with collision later
	PhysicsMaterial m_material;
	bool m_isEnabled			= true;
	int m_id					= -1;
	bool m_isTrigger			= false;

	//AABB2 m_worldBounds;
	Vec3	m_localPosition; // my local offset from my parent
	Vec3	m_worldPosition; // calculated from local position and owning rigidbody if present
};
