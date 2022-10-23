#pragma once
#include "Game/GameCamera.hpp"


//-----------------------------------------------------------------------------------------------
struct Vec3;
class GameEntity;
class World;


//-----------------------------------------------------------------------------------------------
class EntityController
{
public:
	EntityController();
	~EntityController();

	void UpdateRotation();
	void UpdateTranslation();

	GameCamera* GetCurrentWorldCamera() const							{ return m_currentWorldCamera; }
	const Transform GetTransform() const;
	const Vec3	GetPosition() const;
	const float	GetYawDegrees() const;
	const Vec3	GetForwardVector() const;

	void PushCamera( const GameCameraSettings& cameraSettings );
	void PopCamera();

	bool IsPossessing() const											{ return m_possessedEntity != nullptr; }
	GameEntity* GetPossessedEntity() const									{ return m_possessedEntity; }
	void PossessNearestEntity( const World& world );
	void Unpossess();

private:
	// EntityController will exist at camera location
	std::vector<GameCamera*> m_gameCameras;
	int m_gameCameraStackTop = 0;
	GameCamera* m_currentWorldCamera = nullptr;
	GameCamera* m_debugCamera = nullptr;
	GameEntity* m_possessedEntity = nullptr;
};
