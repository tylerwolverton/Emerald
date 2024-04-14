#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrString : public ZephyrType<ZephyrString>
{
	friend class ZephyrSubsystem;
	friend class ZephyrVirtualMachine;

public:
	// ZephyrType Overrides
	ZephyrString();
	virtual ~ZephyrString() {}
	virtual std::string ToString() const override								{ return m_value; }
	virtual void FromString( const std::string& dataStr )						{ m_value = dataStr; }
	virtual bool EvaluateAsBool() const override								{ return !IsEmpty(); }
	virtual ZephyrTypeBase& operator=( ZephyrTypeBase const& other ) override;

	std::string GetValue() const												{ return m_value; }
	bool IsEmpty() const														{ return m_value.empty(); }

// ZephyrType Overrides accessed by friend ZephyrVirtualMachine
protected:
	virtual bool SetMembersFromArgs( ZephyrArgs* args ) override;
	virtual bool SetMember( const std::string& memberName, ZephyrHandle value ) override;
	virtual ZephyrHandle GetMember( const std::string& memberName ) override;

	virtual eZephyrComparatorResult Equal( ZephyrHandle other ) override;

private:
	// static creation accessed by friend ZephyrSubsystem
	static void CreateAndRegisterMetadata();
	static ZephyrHandle CreateAsZephyrType( ZephyrArgs* args );

private:
	std::string m_value;
};


//-----------------------------------------------------------------------------------------------
typedef ChildSmartPtr<ZephyrTypeBase, ZephyrString> ZephyrStringPtr;
