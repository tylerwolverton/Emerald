#include "Engine/Zephyr/Types/ZephyrEntity.hpp"


//-----------------------------------------------------------------------------------------------
const std::string ZephyrEntity::TYPE_NAME = "Entity";


//-----------------------------------------------------------------------------------------------
void ZephyrEntity::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrEntity::TYPE_NAME );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeHandleFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrEntity::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrEntity::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrHandle zephyrEntityHandle = g_zephyrSubsystem->AllocateNewZephyrTypeObject<ZephyrEntity>();
	ChildSmartPtr<ZephyrTypeBase, ZephyrEntity> zephyrBoolPtr( zephyrEntityHandle );

	if ( args == nullptr )
	{
		// Default constructor, don't process parameters
		return zephyrEntityHandle;
	}

	// Fill in vars from args
	zephyrBoolPtr->m_value = args->GetValue( "value", (EntityId)-1 );

	return zephyrEntityHandle;
}


//-----------------------------------------------------------------------------------------------
ZephyrEntity::ZephyrEntity()
	: ZephyrType( ZephyrEntity::TYPE_NAME )
{
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrEntity::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrEntity*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrEntity::SetMembersFromArgs( ZephyrArgs* args )
{
	UNUSED( args );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrEntity::SetMember( const std::string& memberName, ZephyrHandle value )
{
	UNUSED( memberName );
	UNUSED( value );
	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrEntity::GetMember( const std::string& memberName )
{
	UNUSED( memberName );
	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrEntity::Equal( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEntity::TYPE_NAME )
	{
		ZephyrEntityPtr otherEntity( other );
		if ( m_value == otherEntity->GetValue() )
		{
			return eZephyrComparatorResult::TRUE_VAL;
		}
	}
	
	return eZephyrComparatorResult::FALSE_VAL;
}
