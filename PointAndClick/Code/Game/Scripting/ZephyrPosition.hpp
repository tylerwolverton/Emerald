#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


class ZephyrPosition : public IZephyrType
{
public: 
	ZephyrPosition()
	{
		typeMetadata.typeName = "Position";
		typeMetadata.memberNames = { "x", "y" };
		typeMetadata.methodNames = { "GetDistFromOrigin" };

		g_zephyrSubsystem->RegisterZephyrType( typeMetadata );
	}

	virtual std::string ToString() const override { return m_position.ToString(); }

private:
	// Example function for testing
	void GetDistFromOrigin( EventArgs* args )
	{
		args->SetValue( "dist", m_position.GetLength() );
	}

private:
	Vec2 m_position = Vec2::ZERO;
};
