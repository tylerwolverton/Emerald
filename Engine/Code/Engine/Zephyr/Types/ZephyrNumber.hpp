#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrNumber : public ZephyrType<ZephyrNumber>
{
	friend class ZephyrSubsystem;
	friend class ZephyrVirtualMachine;

public:
	// ZephyrType Overrides
	ZephyrNumber();
	virtual ~ZephyrNumber() {}
	virtual std::string ToString() const override				{ return std::string( Stringf( "%.2f", m_value ) ); }
	virtual void FromString( const std::string& dataStr )		{ m_value = (float)atof( dataStr.c_str() ); }
	virtual bool EvaluateAsBool() const override				{ return !IsNearlyEqual( m_value, 0.f ); }
	virtual ZephyrTypeBase& operator=( ZephyrTypeBase const& other ) override;

	NUMBER_TYPE GetValue() const								{ return m_value; }

	static const std::string TYPE_NAME;

// ZephyrType Overrides accessed by friend ZephyrVirtualMachine
protected:
	virtual bool SetMembersFromArgs( ZephyrArgs* args ) override;
	virtual bool SetMember( const std::string& memberName, ZephyrValue& value ) override;
	virtual ZephyrValue GetMember( const std::string& memberName ) override;

	virtual eZephyrComparatorResult Greater( ZephyrValue& other ) override;
	virtual eZephyrComparatorResult GreaterEqual( ZephyrValue& other ) override;
	virtual eZephyrComparatorResult Less( ZephyrValue& other ) override;
	virtual eZephyrComparatorResult LessEqual( ZephyrValue& other ) override;
	virtual eZephyrComparatorResult Equal( ZephyrValue& other ) override;

	virtual ZephyrValue Negate() override;
	virtual ZephyrValue Add( ZephyrValue& other ) override;
	virtual ZephyrValue Subtract( ZephyrValue& other ) override;
	virtual ZephyrValue Multiply( ZephyrValue& other ) override;
	virtual ZephyrValue Divide( ZephyrValue& other ) override;

private:
	// static creation accessed by friend ZephyrSubsystem
	static void CreateAndRegisterMetadata();
	static ZephyrHandle CreateAsZephyrType( ZephyrArgs* args );

private:
	NUMBER_TYPE m_value = 0.f;
};


//-----------------------------------------------------------------------------------------------
typedef ChildSmartPtr<ZephyrTypeBase, ZephyrNumber> ZephyrNumberPtr;
