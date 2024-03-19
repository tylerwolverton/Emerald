#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <map>


//-----------------------------------------------------------------------------------------------
enum class eBytecodeChunkType
{
	NONE,
	GLOBAL,
	STATE,
	EVENT,
};

std::string ToString( eBytecodeChunkType type );


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk
{
public:
	ZephyrBytecodeChunk( const std::string& name, ZephyrBytecodeChunk* parent = nullptr );
	~ZephyrBytecodeChunk();

	// Accessors
	std::string						GetName() const									{ return m_name; }
	std::vector<byte>				GetCode() const									{ return m_bytes; }
	int								GetNumBytes() const								{ return (int)m_bytes.size(); }
	int								GetNumConstants() const							{ return (int)m_constants.size(); }
	byte							GetByte( int idx ) const						{ return m_bytes[idx]; }
	ZephyrValue						GetConstant( int idx ) const					{ return m_constants[idx]; }
	bool							TryToGetVariable( const std::string& identifier, ZephyrValue& out_value ) const;
	ZephyrValueMap					GetVariables() const							{ return m_variables; }
	ZephyrValueMap*					GetUpdateableVariables() 						{ return &m_variables; }
	const ZephyrBytecodeChunkMap&	GetEventBytecodeChunks() const					{ return m_eventBytecodeChunks; }
	eBytecodeChunkType				GetType() const									{ return m_type; }
	bool							IsInitialState() const							{ return m_isInitialState; }
	ZephyrBytecodeChunk*			GetParentChunk() const							{ return m_parentChunk; }

	// Methods to write data to chunk
	void WriteByte( byte newByte );
	void WriteByte( eOpCode opCode );
	void WriteByte( int constantIdx );
	void WriteConstant( const ZephyrValue& constant );
	int AddConstant( const ZephyrValue& constant );
	void SetConstantAtIdx( int idx, const ZephyrValue& constant );
	void AddEventChunk( ZephyrBytecodeChunk* eventBytecodeChunk );

	void SetVariable( const std::string& identifier, const ZephyrValue& value );
	void SetType( eBytecodeChunkType type )											{ m_type = type; }
	void SetAsInitialState()														{ m_isInitialState = true; }

	// Debug methods
	void Disassemble() const;

private:
	std::string m_name;
	ZephyrBytecodeChunk* m_parentChunk = nullptr;
	std::vector<byte> m_bytes;
	std::vector<ZephyrValue> m_constants;
	ZephyrValueMap m_variables;
	ZephyrBytecodeChunkMap m_eventBytecodeChunks;
	eBytecodeChunkType m_type = eBytecodeChunkType::NONE;
	bool m_isInitialState = false;
};
