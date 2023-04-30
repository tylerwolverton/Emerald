#include "Game/Graphics/GameCamera.hpp"
#include "Engine/Renderer/Camera.hpp"


//-----------------------------------------------------------------------------------------------
GameCamera::GameCamera( const EntityId& parentId )
	: EntityComponent( parentId )
{
	m_engineCamera = new Camera();
}


//-----------------------------------------------------------------------------------------------
GameCamera::~GameCamera()
{
	PTR_SAFE_DELETE( m_engineCamera );
}
