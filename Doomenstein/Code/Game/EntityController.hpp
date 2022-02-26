#pragma once
#include "Engine/ZephyrCore/ZephyrEntity.hpp"
#include "Engine/Renderer/Camera.hpp"


//-----------------------------------------------------------------------------------------------
class Entity;
struct Vec3;


//-----------------------------------------------------------------------------------------------
class EntityController : ZephyrEntity
{
public:
	EntityController() = default;
	~EntityController() = default;

	// ZephyrEntity overrides
	virtual const Vec3	GetPosition() const override		{ m_worldCamera.GetTransform().GetPosition(); }
	virtual bool		IsDead() const override				{ return false; }

	// 

private:
	// EntityController will exist at camera location
	Camera m_worldCamera;
	Entity* m_possessedEntity = nullptr;
};
