#include "Engine/Math/Polygon3.hpp"
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
Polygon3::Polygon3( const std::vector<Vec3>& points )
	: m_points( points )
{

}


//-----------------------------------------------------------------------------------------------
Polygon3::~Polygon3()
{
	m_points.clear();
}


//-----------------------------------------------------------------------------------------------
void Polygon3::SetPoints( Vec3* points, int numPoints )
{
	m_points.clear();
	m_points.reserve( numPoints );

	for ( int pointIdx = 0; pointIdx < numPoints; ++pointIdx )
	{
		m_points.push_back( points[pointIdx] );
	}
}


//-----------------------------------------------------------------------------------------------
void Polygon3::SetPoints( const std::vector<Vec3>& points )
{
	m_points.clear();

	m_points = points;
}


//-----------------------------------------------------------------------------------------------
bool Polygon3::IsValid() const
{
	// TODO: Really calculate this
	return true;
}


//-----------------------------------------------------------------------------------------------
bool Polygon3::IsConvex() const
{
	// TODO: Really calculate this
	return true;
}


//-----------------------------------------------------------------------------------------------
bool Polygon3::Contains( const Vec3& point ) const
{
	UNUSED( point );

	// TODO: Really calculate this
	return false;
}


//-----------------------------------------------------------------------------------------------
int Polygon3::GetVertexCount() const
{
	return (int)m_points.size();
}


//-----------------------------------------------------------------------------------------------
void Polygon3::Translate( const Vec3& translation )
{
	for ( int pointIdx = 0; pointIdx < (int)m_points.size(); ++pointIdx )
	{
		m_points[pointIdx] += translation;
	}
}
