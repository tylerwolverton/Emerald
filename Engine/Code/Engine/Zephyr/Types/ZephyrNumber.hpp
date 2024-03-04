#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrNumber : public ZephyrType<ZephyrNumber>
{
public:
	// ZephyrType Overrides
	ZephyrNumber();
	virtual ~ZephyrNumber() {}
	virtual std::string ToString() const override				{ return std::string( Stringf( "%.2f", m_value ) ); }
	virtual void FromString( const std::string& dataStr )		{ m_value = (float)atof( dataStr.c_str() ); }
	virtual bool EvaluateAsBool() const override				{ return !IsNearlyEqual( m_value, 0.f ); }
	virtual ZephyrTypeBase& operator=( ZephyrTypeBase const& other ) override;

	virtual eZephyrComparatorResult Greater( ZephyrHandle other ) override;
	virtual eZephyrComparatorResult GreaterEqual( ZephyrHandle other ) override;
	virtual eZephyrComparatorResult Less( ZephyrHandle other ) override;
	virtual eZephyrComparatorResult LessEqual( ZephyrHandle other ) override;
	virtual eZephyrComparatorResult Equal( ZephyrHandle other ) override;

	virtual ZephyrHandle Add( ZephyrHandle other ) override;
	virtual ZephyrHandle Subtract( ZephyrHandle other ) override;
	virtual ZephyrHandle Multiply( ZephyrHandle other ) override;
	virtual ZephyrHandle Divide( ZephyrHandle other ) override;
	// IZephyrType Overrides

	// static creation
	static void CreateAndRegisterMetadata();
	static ZephyrHandle CreateAsZephyrType( ZephyrArgs* args );

	NUMBER_TYPE GetValue() const		{ return m_value; }

private:
	NUMBER_TYPE m_value = 0.f;
};


//-----------------------------------------------------------------------------------------------
typedef ChildSmartPtr<ZephyrTypeBase, ZephyrNumber> ZephyrNumberPtr;
