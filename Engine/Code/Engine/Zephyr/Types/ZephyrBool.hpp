#pragma once
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
class ZephyrBool : public ZephyrType<ZephyrBool>
{
public:
	// ZephyrType Overrides
	ZephyrBool();
	virtual ~ZephyrBool() {}
	virtual std::string ToString() const override								{ return ::ToString( m_value ); }
	virtual void FromString( const std::string& dataStr )						{ m_value = ::FromString(dataStr, false); }
	virtual bool EvaluateAsBool() const override								{ return m_value; }
	virtual ZephyrTypeBase& operator=( ZephyrTypeBase const& other ) override;
	// IZephyrType Overrides

	// static creation
	static void CreateAndRegisterMetadata();
	static ZephyrTypeBase* CreateAsZephyrType( ZephyrArgs* args );

private:
	bool m_value = false;
};
