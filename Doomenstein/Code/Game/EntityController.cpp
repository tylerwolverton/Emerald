#include "Game/EntityController.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/Entity.hpp"
#include "Game/FPSCamera.hpp"
#include "Game/FreeCamera.hpp"
#include "Game/GameCommon.hpp"
#include "Game/World.hpp"


//-----------------------------------------------------------------------------------------------
EntityController::EntityController()
{
	GameCameraSettings cameraSettings;
	cameraSettings.outputSize = Vec2( 16.f, 9.f );

	m_debugCamera = new FreeCamera( cameraSettings );
	m_gameCameras.push_back( m_debugCamera );

	m_currentWorldCamera = m_debugCamera;
}


//-----------------------------------------------------------------------------------------------
EntityController::~EntityController()
{
	PTR_VECTOR_SAFE_DELETE( m_gameCameras );
}


//-----------------------------------------------------------------------------------------------
void EntityController::PushCamera( const GameCameraSettings& cameraSettings )
{
	GameCamera* newCamera = nullptr;

	switch ( cameraSettings.gameCameraType )
	{
		case eGameCameraType::FREE:
		{
			newCamera = new FreeCamera( cameraSettings );
		}
		break;

		case eGameCameraType::FPS:
		{
			newCamera = new FPSCamera( cameraSettings );
		}
		break;

		default:
		{
			g_devConsole->PrintError( "Unsupported camera type" );
			return;
		}
	}


	if ( m_gameCameraStackTop < (int)m_gameCameras.size() - 1 )
	{
		++m_gameCameraStackTop;
		m_gameCameras[m_gameCameraStackTop] = newCamera;
	}
	else
	{
		m_gameCameras.push_back( newCamera );
		++m_gameCameraStackTop;
	}

	m_currentWorldCamera = newCamera;
}


//-----------------------------------------------------------------------------------------------
void EntityController::PopCamera()
{
	if ( m_currentWorldCamera == m_debugCamera )
	{
		g_devConsole->PrintError( "Can't pop debug camera" );
		return;
	}

	PTR_SAFE_DELETE( m_gameCameras[m_gameCameraStackTop] );
	--m_gameCameraStackTop;
	
	m_currentWorldCamera = m_gameCameras[m_gameCameraStackTop];
}


//-----------------------------------------------------------------------------------------------
void EntityController::PossessNearestEntity( const World& world )
{
	Transform cameraTransform = m_currentWorldCamera->GetTransform();

	Entity* entity = world.GetClosestEntityInSector( cameraTransform.GetPosition().XY(), cameraTransform.GetYawDegrees(), 90.f, 2.f );
	if ( entity == nullptr )
	{
		return;
	}

	if ( GetDistance3D( cameraTransform.GetPosition(), entity->GetPosition() + Vec3( 0.f, 0.f, entity->GetHeight() * .5f ) ) < 2.f )
	{
		m_possessedEntity = entity;
		m_currentWorldCamera->SetPitchRollYawOrientationDegrees( 0.f, 0.f, m_possessedEntity->GetOrientationDegrees() );

		m_possessedEntity->Possess();
	}
}


//-----------------------------------------------------------------------------------------------
void EntityController::Unpossess()
{
	m_possessedEntity->Unpossess();
	m_possessedEntity = nullptr;
}


//-----------------------------------------------------------------------------------------------
const Vec3 EntityController::GetPosition() const
{
	return m_currentWorldCamera->GetTransform().GetPosition();
}


//-----------------------------------------------------------------------------------------------
const Vec3 EntityController::GetForwardVector() const
{
	return m_currentWorldCamera->GetTransform().GetForwardVector();
}


//-----------------------------------------------------------------------------------------------
const Transform EntityController::GetTransform() const
{
	return m_currentWorldCamera->GetTransform();
}


//-----------------------------------------------------------------------------------------------
const float EntityController::GetYawDegrees() const
{
	return m_currentWorldCamera->GetTransform().GetYawDegrees();
}


//-----------------------------------------------------------------------------------------------
void EntityController::Update()
{
	m_currentWorldCamera->Update( m_possessedEntity );
}


