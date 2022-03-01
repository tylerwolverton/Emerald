#pragma once
#include "Engine/ZephyrCore/ZephyrEntity.hpp"
#include "Game/GameCamera.hpp"


//-----------------------------------------------------------------------------------------------
struct Vec3;
class Entity;
class World;


//-----------------------------------------------------------------------------------------------
class EntityController //: ZephyrEntity
{
public:
	EntityController();
	~EntityController();

	GameCamera* GetCurrentWorldCamera() const								{ return m_currentWorldCamera; }
	const Transform GetTransform() const;
	const Vec3	GetPosition() const;
	const float	GetYawDegrees() const;
	const Vec3	GetForwardVector() const;

	void PushCamera( const GameCameraSettings& cameraSettings );
	void PopCamera();

	bool IsPossessing() const											{ return m_possessedEntity != nullptr; }
	Entity* GetPossessedEntity() const									{ return m_possessedEntity; }
	void PossessNearestEntity( const World& world );
	void Unpossess();
	// ZephyrEntity overrides
	//virtual const Vec3	GetPosition() const override;
	//virtual bool			IsDead() const override							{ return false; }
	//virtual void			AddGameEventParams( EventArgs* args ) const		{ UNUSED( args ); }

private:
	// EntityController will exist at camera location
	std::vector<GameCamera*> m_gameCameras;
	int m_gameCameraStackTop = 0;
	GameCamera* m_currentWorldCamera = nullptr;
	GameCamera* m_debugCamera = nullptr;
	Entity* m_possessedEntity = nullptr;
};
