#pragma once
//-----------------------------------------------------------------------------------------------
class GameEntity;


//-----------------------------------------------------------------------------------------------
class EntityController
{
public:
	void SetControlledEntity( GameEntity* entity )		{ m_controlledEntity = entity; }
	GameEntity* GetControlledEntity() const				{ return m_controlledEntity; }

	bool IsControllingEntity() const					{ return m_controlledEntity != nullptr; }

	void Update();

private:
	GameEntity* m_controlledEntity = nullptr;
};
