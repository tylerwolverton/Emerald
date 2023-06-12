#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


void StandAloneFunc( EventArgs* );

class ZephyrPosition : public IZephyrType
{
	friend class Game;

public: 
	ZephyrPosition()
	{
		typeMetadata.typeName = "Position";
		typeMetadata.memberNames = { "x", "y" };
		//ZephyrTypeMethod method( "GetDistFromOrigin", &StandAloneFunc );
		using std::placeholders::_1;
		//ZephyrTypeMethod method( "GetDistFromOrigin", std::bind(&ZephyrPosition::GetDistFromOrigin, this, _1 ) );
		typeMetadata.methods.emplace_back( "GetDistFromOrigin", std::bind(&ZephyrPosition::GetDistFromOrigin, this, _1 ) );
	}

	virtual std::string ToString() const override { return m_position.ToString(); }
	virtual IZephyrType* CloneSelf() const override { return new ZephyrPosition(); } // TODO: Support actual clones and prototype objects

private:
	// Example function for testing
	void GetDistFromOrigin( EventArgs* args )
	{
		args->SetValue( "dist", m_position.GetLength() );
	}

private:
	Vec2 m_position = Vec2(3.f, 4.f);
	//Vec2 m_position = Vec2::ZERO;
};
