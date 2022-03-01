#include "Game/GameCamera.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
GameCamera::GameCamera( const GameCameraSettings& gameCameraSettings )
{
	m_engineCamera = new Camera();

	switch ( gameCameraSettings.cameraType )
	{
		case eCameraType::WORLD:
		{
			Texture* depthTexture = g_renderer->GetOrCreateDepthStencil( g_renderer->GetDefaultBackBufferSize() );
			m_engineCamera->SetDepthStencilTarget( depthTexture );

			m_engineCamera->SetOutputSize( gameCameraSettings.outputSize );
			m_engineCamera->SetProjectionPerspective( gameCameraSettings.fovDegrees,
													 gameCameraSettings.nearClipZ,
													 gameCameraSettings.farClipZ );

			Rgba8 backgroundColor( 10, 10, 10, 255 );
			m_engineCamera->SetClearMode( CLEAR_COLOR_BIT | CLEAR_DEPTH_BIT, backgroundColor );
		}
		break;

		case eCameraType::UI:
		{
			g_devConsole->PrintWarning( "UI game cameras are not implemented" );
			//m_engineCamera->SetOutputSize( gameCameraSettings.outputSize );
			//m_engineCamera->SetType( eCameraType::UI );
			//m_engineCamera->SetPosition( Vec3( gameCameraSettings.outputSize * .5f, 0.f ) );
			//m_engineCamera->SetProjectionOrthographic( gameCameraSettings.outputSize.y );
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
GameCamera::~GameCamera()
{
	PTR_SAFE_DELETE( m_engineCamera );
}


//-----------------------------------------------------------------------------------------------
void GameCamera::BeginCamera()
{
	g_renderer->BeginCamera( *m_engineCamera );
}


//-----------------------------------------------------------------------------------------------
void GameCamera::EndCamera()
{
	g_renderer->EndCamera( *m_engineCamera );
}


//-----------------------------------------------------------------------------------------------
void GameCamera::DebugRenderWorld()
{
	DebugRenderWorldToCamera( m_engineCamera );
}

