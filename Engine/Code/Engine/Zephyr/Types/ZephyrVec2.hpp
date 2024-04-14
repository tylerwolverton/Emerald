#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrVec2 : public ZephyrType<ZephyrVec2>
{
	friend class ZephyrSubsystem;
	friend class ZephyrVirtualMachine;

public:
	// ZephyrType Overrides
	ZephyrVec2();
	virtual ~ZephyrVec2() {}
	virtual std::string ToString() const override								{ return m_value.ToString(); }
	virtual void FromString( const std::string& dataStr )						{ m_value.SetFromText( dataStr.c_str() ); }
	virtual bool EvaluateAsBool() const override								{ return !IsNearlyEqual( m_value, Vec2::ZERO ); }
	virtual ZephyrTypeBase& operator=( ZephyrTypeBase const& other ) override;

	Vec2 GetValue() const { return m_value; }

// ZephyrType Overrides accessed by friend ZephyrVirtualMachine
protected:
	virtual bool SetMembersFromArgs( ZephyrArgs* args ) override;
	virtual bool SetMember( const std::string& memberName, ZephyrHandle value ) override;
	virtual ZephyrHandle GetMember( const std::string& memberName ) override;

	virtual eZephyrComparatorResult Equal( ZephyrHandle other ) override;

	virtual ZephyrHandle Negate() override;
	virtual ZephyrHandle Add( ZephyrHandle other ) override;
	virtual ZephyrHandle Subtract( ZephyrHandle other ) override;
	virtual ZephyrHandle Multiply( ZephyrHandle other ) override;
	virtual ZephyrHandle Divide( ZephyrHandle other ) override;

private:
	// static creation accessed by friend ZephyrSubsystem
	static void CreateAndRegisterMetadata();
	static ZephyrHandle CreateAsZephyrType( ZephyrArgs* args );

private:
	Vec2 m_value = Vec2::ZERO;
};


//-----------------------------------------------------------------------------------------------
typedef ChildSmartPtr<ZephyrTypeBase, ZephyrVec2> ZephyrVec2Ptr;
