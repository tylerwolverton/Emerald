#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrPosition : public ZephyrTypeTemplate<ZephyrPosition>
{
public: 
	// ZephyrType Overrides
	ZephyrPosition();
	virtual ~ZephyrPosition() {}
	virtual std::string ToString() const override								{ return m_position.ToString(); }
	virtual void FromString( const std::string& dataStr )						{ m_position.SetFromText( dataStr.c_str()); }
	virtual bool EvaluateAsBool() const override;
	// IZephyrType Overrides

	// static creation
	static void CreateAndRegisterMetadata();
	static ZephyrType* CreateAsZephyrType( ZephyrArgs* args );

private:
	void GetDistFromOrigin( ZephyrArgs* args );
	void Set_x( ZephyrArgs* args );
	void Set_y( ZephyrArgs* args );

private:
	//Vec2 m_position = Vec2(3.f, 4.f);
	Vec2 m_position = Vec2::ZERO;
};
