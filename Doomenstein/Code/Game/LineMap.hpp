#include "Game/Map.hpp"


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
};
