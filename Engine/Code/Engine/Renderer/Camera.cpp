#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/RenderBuffer.hpp"
#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
Camera::Camera()
{
}


//-----------------------------------------------------------------------------------------------
Camera::~Camera()
{
	PTR_SAFE_DELETE( m_cameraUBO );
}


//-----------------------------------------------------------------------------------------------
Vec3 Camera::ClientToWorldPosition( const Vec2& clientPos, float ndcZ ) const
{
	Vec3 ndc = RangeMapVec3( Vec3::ZERO, Vec3::ONE,
							  Vec3( -1.f, -1.f, 0.f ), Vec3::ONE,
							  Vec3( clientPos, ndcZ ) );

	Mat44 proj = GetProjectionMatrix();
	Mat44 worldToClip = proj;
	worldToClip.PushTransform( GetViewMatrix() );

	Mat44 clipToWorld = worldToClip;
	InvertMatrix( clipToWorld );

	Vec4 worldHomogeneous = clipToWorld.TransformHomogeneousPoint3D( Vec4( ndc, 1.f ) );
	Vec3 worldPos = worldHomogeneous.XYZ() / worldHomogeneous.w;

	return worldPos;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetPosition( const Vec3& position )
{
	m_transform.SetPosition( position );
}


//-----------------------------------------------------------------------------------------------
void Camera::Translate( const Vec3& translation )
{
	m_transform.Translate( translation );
}


//-----------------------------------------------------------------------------------------------
void Camera::Translate2D( const Vec2& translation )
{
	m_transform.Translate( Vec3( translation, 0.f ) );
}


//-----------------------------------------------------------------------------------------------
Vec2 Camera::GetOrthoMin() const
{
	return ClientToWorldPosition( Vec2::ZERO, 0 ).XY();
}


//-----------------------------------------------------------------------------------------------
Vec2 Camera::GetOrthoMax() const
{
	return ClientToWorldPosition( Vec2::ONE, 0 ).XY();
}


//-----------------------------------------------------------------------------------------------
void Camera::SetPitchRollYawOrientationDegrees( float pitch, float roll, float yaw )
{
	pitch = ClampMinMax( pitch, -89.9f, 89.9f );

	m_transform.SetOrientationFromPitchRollYawDegrees( pitch, roll, yaw );
}


//-----------------------------------------------------------------------------------------------
void Camera::SetYawPitchRollOrientationDegrees( float yawDegrees, float pitchDegrees, float rollDegrees )
{
	SetPitchRollYawOrientationDegrees( pitchDegrees, rollDegrees, yawDegrees );
}


//-----------------------------------------------------------------------------------------------
void Camera::SetYawOrientationDegrees( float yawDegrees )
{
	SetYawPitchRollOrientationDegrees( yawDegrees, m_transform.GetPitchDegrees(), m_transform.GetRollDegrees() );
}


//-----------------------------------------------------------------------------------------------
void Camera::RotateYawPitchRoll( float yawDegrees, float pitchDegrees, float rollDegrees )
{
	SetPitchRollYawOrientationDegrees( m_transform.GetPitchDegrees() + pitchDegrees,
									   m_transform.GetRollDegrees() + rollDegrees, 
									   m_transform.GetYawDegrees() + yawDegrees );
}


//-----------------------------------------------------------------------------------------------
void Camera::SetProjectionOrthographic( float height, float nearZ, float farZ )
{
	float aspect = GetAspectRatio();
	float halfHeight = height * .5f;
	float halfWidth = halfHeight * aspect;

	Vec2 mins( -halfWidth, -halfHeight );
	Vec2 maxs( halfWidth, halfHeight );

	m_projectionMatrix = MakeOrthographicProjectionMatrixD3D( mins.x, maxs.x,
															  mins.y, maxs.y,
															  nearZ, farZ );
}


//-----------------------------------------------------------------------------------------------
void Camera::SetProjectionPerspective( float fovDegrees, float nearZClip, float farZClip )
{
	m_projectionMatrix = MakePerspectiveProjectionMatrixD3D( fovDegrees,
															 GetAspectRatio(),
															 nearZClip, farZClip );
}


//-----------------------------------------------------------------------------------------------
void Camera::SetClearMode( unsigned int clearFlags, Rgba8 color, float depth, unsigned int stencil )
{
	UNUSED( stencil );

	m_clearMode = clearFlags;
	m_clearColor = color;
	m_clearDepth = depth;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetColorTarget( Texture* texture )
{
	SetColorTarget( 0, texture );
}


//-----------------------------------------------------------------------------------------------
void Camera::SetColorTarget( int index, Texture* texture )
{
	if ( index >= (int)m_colorTargets.size() )
	{
		m_colorTargets.resize( index + 1 );
	}

	m_colorTargets[index] = texture;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetDepthStencilTarget( Texture* texture )
{
	m_depthStencilTarget = texture;
}


//-----------------------------------------------------------------------------------------------
Texture* Camera::GetColorTarget() const
{
	if ( m_colorTargets.size() == 0 )
	{
		return nullptr;
	}

	return m_colorTargets[0];
}


//-----------------------------------------------------------------------------------------------
Texture* Camera::GetColorTarget( int index ) const
{
	if ( index < (int)m_colorTargets.size() )
	{
		return m_colorTargets[index];
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
uint Camera::GetColorTargetCount() const
{
	// Backwards compatibility with SD1
	uint count = (uint)m_colorTargets.size();
	if ( count == 0 )
	{
		count = 1;
	}

	return count;
}


//-----------------------------------------------------------------------------------------------
void Camera::SetOutputSize( const Vec2& size )
{
	m_outputSize = size;
}


//-----------------------------------------------------------------------------------------------
float Camera::GetAspectRatio() const
{
	if ( m_outputSize.y == 0.f )
	{
		return 0.f;
	}

	return m_outputSize.x / m_outputSize.y;
}


//-----------------------------------------------------------------------------------------------
void Camera::UpdateCameraUBO()
{
	CameraData cameraData;
	cameraData.projection = m_projectionMatrix;

	Mat44 viewMatrix;

	switch ( m_cameraType )
	{
		case eCameraType::WORLD: viewMatrix = m_transform.GetAsMatrix(); break;
		case eCameraType::UI: viewMatrix = m_transform.GetAsAbsoluteMatrix(); break;
	}

	InvertMatrix( viewMatrix );

	cameraData.view = viewMatrix;
	m_viewMatrix = viewMatrix;

	cameraData.worldPosition = m_transform.GetPosition();

	m_cameraUBO->Update( &cameraData, sizeof( cameraData ), sizeof( cameraData ) );
}
