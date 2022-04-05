#include "Game/Map.hpp"
#include "Engine/Math/Transform.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct OBB3;
struct Vertex_PCUTBN;
class Entity;
class MapRegionTypeDefinition;


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

	void				  ResolveEntityVsWallCollisions();
	void				  ResolveEntityVsWallCollision( Entity& entity );

	void				ApplyGravityToEntities();

private:
	Transform								m_raytraceTransform;

	bool									m_isMeshDirty = true;
	std::vector<Vertex_PCUTBN>				m_mesh;
	std::vector<OBB3>						m_walls;
	std::vector<MapRegionTypeDefinition*>	m_regionTypeDefs;	

	Vec3									m_gravityVec = Transform::GetWorldUpVector() * -1.5f;
};
