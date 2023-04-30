#pragma once
#include "Engine/Framework/EntityComponent.hpp"


//-----------------------------------------------------------------------------------------------
class Camera;


//-----------------------------------------------------------------------------------------------
class GameCamera : public EntityComponent
{
public:
	GameCamera( const EntityId& parentId );
	~GameCamera();

private:
	Camera* m_engineCamera = nullptr;
};
