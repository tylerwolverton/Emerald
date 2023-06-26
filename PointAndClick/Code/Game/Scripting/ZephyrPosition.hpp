#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrPosition : public IZephyrType
{
public: 
	// IZephyrType Overrides
	virtual std::string ToString()		const override { return m_position.ToString(); }
	virtual IZephyrType* ChildCloneSelf()	const override;
	static void CreateAndRegisterMetadata();
	// IZephyrType Overrides

private:
	void GetDistFromOrigin( EventArgs* args );

private:
	Vec2 m_position = Vec2(3.f, 4.f);
	//Vec2 m_position = Vec2::ZERO;
};
