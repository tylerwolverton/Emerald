#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrBool : public ZephyrType<ZephyrBool>
{
	friend class ZephyrSubsystem;
	friend class ZephyrVirtualMachine;

public:
	// ZephyrType Overrides
	ZephyrBool();
	virtual ~ZephyrBool() {}
	virtual std::string ToString() const override								{ return ::ToString( m_value ); }
	virtual void FromString( const std::string& dataStr )						{ m_value = ::FromString(dataStr, false); }
	virtual bool EvaluateAsBool() const override								{ return m_value; }
	virtual ZephyrTypeBase& operator=( ZephyrTypeBase const& other ) override;

	bool GetValue() const { return m_value; }

	static const std::string TYPE_NAME;

// ZephyrType Overrides accessed by friend ZephyrVirtualMachine
protected:
	virtual bool SetMembersFromArgs( ZephyrArgs* args ) override;
	virtual bool SetMember( const std::string& memberName, ZephyrValue& value ) override;
	virtual ZephyrValue GetMember( const std::string& memberName ) override;

	virtual eZephyrComparatorResult Equal( ZephyrValue& other ) override;

private:
	// static creation accessed by friend ZephyrSubsystem
	static void CreateAndRegisterMetadata();
	static ZephyrHandle CreateAsZephyrType( ZephyrArgs* args );

private:
	bool m_value = false;
};


//-----------------------------------------------------------------------------------------------
typedef ChildSmartPtr<ZephyrTypeBase, ZephyrBool> ZephyrBoolPtr;
