#pragma once
#include "Engine/Networking/NetworkingCommon.hpp"
#include "Engine/Networking/MessageProtocols.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class TCPData
{
public:
	TCPData() 
		: m_length( 0 ), m_data( NULL ) {}
	TCPData( size_t length, char* dataPtr, const std::string& fromAddress  ) 
		: m_length( length )
		, m_data( dataPtr )
		, m_fromAddress( fromAddress )
	{}

	~TCPData() = default;

	size_t		GetLength() const				{ return m_length; }
	const char* GetData() const					{ return m_data; }
	const char* GetPayload() const				{ return m_data + sizeof( MessageHeader ); }

	std::string GetFromAddress() const			{ return m_fromAddress; }
	std::string GetFromIPAddress() const;
	int			GetFromPort() const;
	
	std::string GetDataAsString() const			{ return std::string( m_data, m_length ); }

private:
	size_t m_length;
	char* m_data;

	std::string m_fromAddress;
};


//-----------------------------------------------------------------------------------------------
class TCPSocket
{
public:
	TCPSocket();
	TCPSocket( SOCKET socket, eBlockingMode mode = eBlockingMode::BLOCKING, size_t bufferSize = 256 );
	~TCPSocket();

	TCPSocket& operator=( const TCPSocket& src );

	bool IsValid()											{ return m_socket != INVALID_SOCKET; }

	std::string GetAddress();
	
	void Send( const char* data, size_t length );
	TCPData Receive();

	void Close();

	bool IsDataAvailable();

private:
	eBlockingMode m_blockingMode = eBlockingMode::INVALID;
	SOCKET m_socket = INVALID_SOCKET;

	FD_SET m_fdSet;
	timeval m_timeval = {0l,0l};

	size_t m_bufferSize = 0;
	size_t m_receiveSize = 0;
	char* m_buffer = nullptr;
};
