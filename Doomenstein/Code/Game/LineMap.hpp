#include "Game/Map.hpp"
#include "Engine/Math/Transform.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class MapRegionTypeDefinition;
struct OBB3;
struct Vertex_PCUTBN;


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
	
	void AddWallFace( const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topLeft, const Vec3& topRight );

private:
	Transform					m_raytraceTransform;

	bool m_isMeshDirty = true;
	std::vector<Vertex_PCUTBN>	m_mesh;
	std::vector<OBB3>		m_walls;
	std::vector<MapRegionTypeDefinition*> m_regionTypeDefs;	
};
