#include "Game/Map.hpp"
#include "Engine/Math/Transform.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct Vertex_PCUTBN;
class Polygon3;


//-----------------------------------------------------------------------------------------------
class LineMap : public Map
{
public:
	LineMap( const MapData& mapData, World* world );
	virtual ~LineMap();

	virtual void		Load() override;
	virtual void		Unload() override;

	virtual void		Update( float deltaSeconds ) override;
	virtual void		UpdateMeshes() override;
	virtual void		Render() const override;
	virtual void		DebugRender() const override;

protected:
	virtual RaycastResult Raycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const override;

private:
	Transform					m_raytraceTransform;

	std::vector<Vertex_PCUTBN>	m_mesh;
	std::vector<Polygon3>		m_walls;
};
