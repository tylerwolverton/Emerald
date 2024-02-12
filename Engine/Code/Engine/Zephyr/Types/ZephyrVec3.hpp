#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrVec3 : public ZephyrType<ZephyrVec3>
{
public:
	// ZephyrType Overrides
	ZephyrVec3();
	virtual ~ZephyrVec3() {}
	virtual std::string ToString() const override									{ return m_value.ToString(); }
	virtual void FromString( const std::string& dataStr )							{ m_value.SetFromText( dataStr.c_str() ); }
	virtual bool EvaluateAsBool() const override									{ return !IsNearlyEqual( m_value, Vec3::ZERO ); }
	virtual ZephyrTypeBase& operator=( ZephyrTypeBase const& other ) override;

	virtual eZephyrComparatorResult Equal( ZephyrTypeBase* other ) override;

	virtual ZephyrTypeBase* Add( ZephyrTypeBase* other ) override;
	virtual ZephyrTypeBase* Subtract( ZephyrTypeBase* other ) override;
	virtual ZephyrTypeBase* Multiply( ZephyrTypeBase* other ) override;
	virtual ZephyrTypeBase* Divide( ZephyrTypeBase* other ) override;
	// IZephyrType Overrides

	// static creation
	static void CreateAndRegisterMetadata();
	static ZephyrTypeBase* CreateAsZephyrType( ZephyrArgs* args );

	Vec3 GetValue() const { return m_value; }

private:
	void Set_x( ZephyrArgs* args );
	void Set_y( ZephyrArgs* args );
	void Set_z( ZephyrArgs* args );

private:
	Vec3 m_value = Vec3::ZERO;
};
