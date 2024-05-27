#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrPosition : public ZephyrType<ZephyrPosition>
{
	friend class ZephyrTypeRegistrator;
	friend class ZephyrVirtualMachine;

public: 
	ZephyrPosition();
	virtual ~ZephyrPosition() {}

	// ZephyrType Overrides
	virtual std::string ToString() const override								{ return m_position.ToString(); }
	virtual void FromString( const std::string& dataStr )						{ m_position.SetFromText( dataStr.c_str()); }
	virtual bool EvaluateAsBool() const override;
	virtual ZephyrTypeBase& operator=( ZephyrTypeBase const& other ) override;

	static const std::string TYPE_NAME;
	
// ZephyrType Overrides accessed by friend ZephyrVirtualMachine
protected:
	virtual bool SetMembersFromArgs( ZephyrArgs* args ) override;
	virtual bool SetMember( const std::string& memberName, ZephyrValue& value ) override;
	virtual ZephyrValue GetMember( const std::string& memberName ) override;

private:
	// static creation accessed by friend ZephyrTypeRegistrator
	static void CreateAndRegisterMetadata();
	static ZephyrHandle CreateAsZephyrType( ZephyrArgs* args );

	void GetDistFromOrigin( ZephyrArgs* args );

private:
	//Vec2 m_position = Vec2(3.f, 4.f);
	Vec2 m_position = Vec2::ZERO;
};


//-----------------------------------------------------------------------------------------------
typedef ChildSmartPtr<ZephyrTypeBase, ZephyrPosition> ZephyrPositionPtr;
