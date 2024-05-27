#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrEntity : public ZephyrType<ZephyrEntity>
{
	friend class ZephyrSubsystem;
	friend class ZephyrVirtualMachine;

public:
	// ZephyrType Overrides
	ZephyrEntity();
	virtual ~ZephyrEntity() {}
	virtual std::string ToString() const override								{ return ::ToString( m_value ); }
	virtual void FromString( const std::string& dataStr )						{ m_value = ::FromString( dataStr, ERROR_ZEPHYR_ENTITY_ID ); }
	virtual bool EvaluateAsBool() const override								{ return m_value != ERROR_ZEPHYR_ENTITY_ID; }
	virtual ZephyrTypeBase& operator=( ZephyrTypeBase const& other ) override;

	EntityId GetValue() const { return m_value; }

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
	EntityId m_value = ERROR_ZEPHYR_ENTITY_ID;
};


//-----------------------------------------------------------------------------------------------
typedef ChildSmartPtr<ZephyrTypeBase, ZephyrEntity> ZephyrEntityPtr;
