#pragma once
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
class EntityDefinition;
class Collision;


//-----------------------------------------------------------------------------------------------
class Pickup : public Entity
{
public:
	Pickup( const EntityDefinition& entityDef, Map* map );
	~Pickup();

private:
	void EnterCollisionEvent( Collision collision );
};
