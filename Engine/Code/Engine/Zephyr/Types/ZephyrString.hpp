#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrString : public ZephyrType<ZephyrString>
{
public:
	// ZephyrType Overrides
	ZephyrString();
	virtual ~ZephyrString() {}
	virtual std::string ToString() const override								{ return m_value; }
	virtual void FromString( const std::string& dataStr )						{ m_value = dataStr; }
	virtual bool EvaluateAsBool() const override								{ return !m_value.empty(); }
	virtual ZephyrTypeBase& operator=( ZephyrTypeBase const& other ) override;

	virtual eZephyrComparatorResult Equal( ZephyrTypeBase* other ) override;

	// TODO: Explicit concatenate instead?
	//virtual ZephyrTypeBase* Add( ZephyrTypeBase* other ) override;
	// IZephyrType Overrides

	// static creation
	static void CreateAndRegisterMetadata();
	static ZephyrTypeBase* CreateAsZephyrType( ZephyrArgs* args );

	std::string GetValue() const { return m_value; }

private:
	std::string m_value;
};
