#pragma once
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------
struct Rgba8;
template <class CollisionPolicy>
class PhysicsScene;
class Collider;
class RenderContext;


//-----------------------------------------------------------------------------------------------
enum eSimulationMode : unsigned int
{
	SIMULATION_MODE_NONE,
	SIMULATION_MODE_STATIC,
	SIMULATION_MODE_KINEMATIC,
	SIMULATION_MODE_DYNAMIC
};


//-----------------------------------------------------------------------------------------------
class Rigidbody
{
	template <class CollisionPolicy>
	friend class PhysicsScene;

public:
	void Update( float deltaSeconds );

	void Destroy(); // helper for destroying myself (uses owner to destroy self)

	Collider* GetCollider()															{ return m_collider; }
	void TakeCollider( Collider* collider ); // takes ownership of a collider (destroying my current one if present)

	uint GetLayer() const															{ return m_layer; }
	void SetLayer( uint layer )														{ m_layer = layer; }
	void SetLayer( const std::string& collisionLayerStr );

	Vec3 GetVelocity()																{ return m_velocity; }
	void SetVelocity( const Vec3& velocity );
	
	Vec3 GetWorldPosition()															{ return m_worldPosition; }
	Vec3 GetCenterOfMass() const;
	void SetPosition( const Vec3& position );
	void Translate( const Vec3& translation );

	Vec3 GetImpactVelocityAtPoint( const Vec3& point );

	void SetRotationDegrees( float newRotationDegrees );
	void RotateDegrees( float deltaDegrees );
	void ChangeAngularVelocity( float deltaRadians );
	void SetAngularVelocity( float newAngularVelocity );

	float GetMass() const															{ return m_mass; }
	void SetMass( float mass );
	void ChangeMass( float deltaMass );
	float GetInverseMass() const													{ return m_inverseMass; }

	float GetDrag() const															{ return m_drag; }
	void SetDrag( float drag )														{ m_drag = drag; }
	void ChangeDrag( float deltaDrag );

	void AddForce( const Vec3& force );
	void AddImpulse( const Vec3& impulse );
	void ApplyImpulseAt( const Vec3& impulse, const Vec3& worldPosition );
	void ApplyDragForce();

	void DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) const;

	bool IsEnabled() const															{ return m_isEnabled; }
	void Enable();
	void Disable();

	eSimulationMode GetSimulationMode()	const										{ return m_simulationMode; }
	void SetSimulationMode( eSimulationMode mode )									{ m_simulationMode = mode; }

	float GetAngularVelocity() const												{ return m_angularVelocity; }
	float GetOrientationDegrees() const;
	float GetOrientationRadians() const												{ return m_orientationRadians; }
	float GetMomentOfInertia() const												{ return m_moment; }

public:
	NamedProperties m_userProperties;

private:
	PhysicsScene<CollisionPolicy>* m_scene = nullptr;			// which scene created/owns this object
	Vec3 m_worldPosition = Vec3::ZERO;							// where in the world is this rigidbody
	Collider* m_collider = nullptr;

	Vec3 m_sumOfForces = Vec3::ZERO;
	Vec3 m_velocity = Vec3::ZERO;
	//Vec2 m_verletVelocity = Vec2::ZERO;

	float m_mass = 1.f;
	float m_inverseMass = 1.f;
	float m_drag = 0.f;

	float m_orientationRadians = 0.f;
	float m_angularVelocity = 0.f;
	float m_frameTorque = 0.f;
	float m_moment = 0.f;
	float m_inverseMoment = 1.f;

	bool m_isEnabled = true;
	eSimulationMode m_simulationMode = SIMULATION_MODE_DYNAMIC;

	uint m_layer = 0;

private:
	~Rigidbody(); // destroys the collider
};
