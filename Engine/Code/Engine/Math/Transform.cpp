#include "Engine/Math/Transform.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------
// Static member definitions
AxisOrientation Transform::s_axisOrientation;
Mat44 Transform::s_identityOrientation;


//-----------------------------------------------------------------------------------------------
void Transform::SetPosition( const Vec3& position )
{
	m_position = position;
}


//-----------------------------------------------------------------------------------------------
void Transform::Translate( const Vec3& translation )
{
	m_position += translation;
}


//-----------------------------------------------------------------------------------------------
void Transform::SetOrientationFromPitchRollYawDegrees( float pitchDegrees, float rollDegrees, float yawDegrees )
{
	m_pitchDegrees = GetRotationInRangeDegrees( pitchDegrees, -180.f, 180.f );
	m_rollDegrees = GetRotationInRangeDegrees( rollDegrees, -180.f, 180.f );
	m_yawDegrees = GetRotationInRangeDegrees( yawDegrees, 0.f, 360.f );
}


//-----------------------------------------------------------------------------------------------
void Transform::RotatePitchRollYawDegrees( float pitchDegrees, float rollDegrees, float yawDegrees )
{
	SetOrientationFromPitchRollYawDegrees( m_pitchDegrees + pitchDegrees, 
										   m_rollDegrees + rollDegrees, 
										   m_yawDegrees + yawDegrees );
}


//-----------------------------------------------------------------------------------------------
void Transform::SetScale( const Vec3& scale )
{
	m_scale = scale;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Transform::GetAsMatrix() const
{
	Mat44 translationMatrix = Mat44::CreateTranslation3D( m_position );
	Mat44 rotationMatrix = GetWorldOrientationAsMatrix();
	Mat44 scaleMatrix = Mat44::CreateNonUniformScale3D( m_scale );

	Mat44 modelMatrix = translationMatrix;
	modelMatrix.PushTransform( rotationMatrix );
	modelMatrix.PushTransform( scaleMatrix );

	return modelMatrix;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Transform::GetAsAbsoluteMatrix() const
{
	Mat44 translationMatrix = Mat44::CreateTranslation3D( m_position );
	Mat44 rotationMatrix = GetOrientationAsMatrix();
	Mat44 scaleMatrix = Mat44::CreateNonUniformScale3D( m_scale );

	Mat44 modelMatrix = translationMatrix;
	modelMatrix.PushTransform( rotationMatrix );
	modelMatrix.PushTransform( scaleMatrix );

	return modelMatrix;
}


//-----------------------------------------------------------------------------------------------
Vec3 Transform::GetForwardVector() const
{
	Vec3 forwardVec;

	switch ( s_axisOrientation.m_axisYawPitchRollOrder )
	{
		case eAxisYawPitchRollOrder::YXZ:
		{
			forwardVec = GetAsMatrix().TransformVector3D( Vec3( 0.f, 0.f, s_axisOrientation.GetZAxisDirectionFactor() ) ).GetNormalized();
		} 
		break;

		case eAxisYawPitchRollOrder::ZYX:
		{
			Mat44 orientation = GetOrientationAsMatrix();
			forwardVec = orientation.TransformVector3D( -s_identityOrientation.GetKBasis3D() );
		} 
		break;
	}

	return forwardVec;
}


//-----------------------------------------------------------------------------------------------
Vec3 Transform::GetRightVector() const
{
	Vec3 rightVec;

	switch ( s_axisOrientation.m_axisYawPitchRollOrder )
	{
		case eAxisYawPitchRollOrder::YXZ:
		{
			rightVec = GetAsMatrix().TransformVector3D( Vec3( s_axisOrientation.GetXAxisDirectionFactor(), 0.f, 0.f) ).GetNormalized();
		}
		break;

		case eAxisYawPitchRollOrder::ZYX:
		{
			Mat44 orientation = GetOrientationAsMatrix();
			rightVec = orientation.TransformVector3D( s_identityOrientation.GetIBasis3D() );
		}
		break;
	}

	return rightVec;
}


//-----------------------------------------------------------------------------------------------
Vec3 Transform::GetUpVector() const
{
	Vec3 upVec;

	switch ( s_axisOrientation.m_axisYawPitchRollOrder )
	{
		case eAxisYawPitchRollOrder::YXZ:
		{
			upVec = GetAsMatrix().TransformVector3D( Vec3( 0.f, s_axisOrientation.GetYAxisDirectionFactor(), 0.f ) ).GetNormalized();
		}
		break;

		case eAxisYawPitchRollOrder::ZYX:
		{
			Mat44 orientation = GetOrientationAsMatrix();
			upVec = orientation.TransformVector3D( s_identityOrientation.GetJBasis3D() );
		}
		break;
	}

	return upVec;
}


//-----------------------------------------------------------------------------------------------
const Vec3 Transform::GetWorldUpVector()
{
	switch ( s_axisOrientation.m_axisYawPitchRollOrder )
	{
		case eAxisYawPitchRollOrder::YXZ:
		{
			return Vec3( 0.f, 1.f, 0.f );
		}
		break;

		case eAxisYawPitchRollOrder::ZYX:
		{
			return Vec3( 0.f, 0.f, 1.f );
		}
		break;
	}

	return Vec3::ZERO;
}


//-----------------------------------------------------------------------------------------------
const Vec3 Transform::GetWorldRightVector()
{
	switch ( s_axisOrientation.m_axisYawPitchRollOrder )
	{
		case eAxisYawPitchRollOrder::YXZ:
		case eAxisYawPitchRollOrder::ZYX:
		{
			return Vec3( 1.f, 0.f, 0.f );
		}
		break;
	}

	return Vec3::ZERO;
}


//-----------------------------------------------------------------------------------------------
const Vec3 Transform::GetWorldForwardVector()
{
	switch ( s_axisOrientation.m_axisYawPitchRollOrder )
	{
		case eAxisYawPitchRollOrder::YXZ:
		{
			return Vec3( 0.f, 0.f, -1.f );
		}
		break;

		case eAxisYawPitchRollOrder::ZYX:
		{
			return Vec3( 0.f, 1.f, 0.f );
		}
		break;
	}

	return Vec3::ZERO;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Transform::GetOrientationAsMatrix() const
{
	Mat44 rotationMatrix;

	switch ( s_axisOrientation.m_axisYawPitchRollOrder )
	{
		case eAxisYawPitchRollOrder::YXZ:
		{
			rotationMatrix.PushTransform( Mat44::CreateYRotationDegrees( m_yawDegrees   * s_axisOrientation.GetYAxisDirectionFactor() ) );
			rotationMatrix.PushTransform( Mat44::CreateXRotationDegrees( m_pitchDegrees * s_axisOrientation.GetXAxisDirectionFactor() ) );
			rotationMatrix.PushTransform( Mat44::CreateZRotationDegrees( m_rollDegrees  * s_axisOrientation.GetZAxisDirectionFactor() ) );
		} 
		break;

		case eAxisYawPitchRollOrder::ZYX:
		{
			rotationMatrix.PushTransform( Mat44::CreateZRotationDegrees( m_yawDegrees   * s_axisOrientation.GetZAxisDirectionFactor() ) );
			rotationMatrix.PushTransform( Mat44::CreateYRotationDegrees( m_pitchDegrees * s_axisOrientation.GetYAxisDirectionFactor() ) );
			rotationMatrix.PushTransform( Mat44::CreateXRotationDegrees( m_rollDegrees  * s_axisOrientation.GetXAxisDirectionFactor() ) );
		} 
		break;
	}

	return rotationMatrix;
}


//-----------------------------------------------------------------------------------------------
const Mat44 Transform::GetWorldOrientationAsMatrix() const
{
	Mat44 rotationMatrix = GetOrientationAsMatrix();
	rotationMatrix.PushTransform( s_identityOrientation );
	return rotationMatrix;
}
