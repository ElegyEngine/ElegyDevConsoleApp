// SPDX-FileCopyrightText: 2023 Admer Šuko
// SPDX-License-Identifier: MIT

#ifdef WIN32
#pragma		comment(lib, "Winmm.lib")
#pragma		comment(lib, "Ws2_32.lib")
#endif

#include "Precompiled.hpp"
#include "Network.hpp"

// ============================
// Network::Init
// ============================
bool Network::Init( std::function<OnReceiveMessageFn> receiveMessage,
	std::function<OnReceiveAutocompleteFn> receiveAutocomplete )
{
	onReceiveMessage = receiveMessage;
	onReceiveAutocomplete = receiveAutocomplete;

	if ( enet_initialize() < 0 )
	{
		onReceiveMessage( { "$y[DevConsoleApp] $rFailed to initialise ENet" } );
		return false;
	}

	enet_address_set_host_ip( &consoleBridgeAddress, "127.0.0.1" );
	consoleBridgeAddress.port = 23005;

	consoleAppHost = enet_host_create( nullptr, 1, 2, 0, 0 );
	consoleBridgePeer = nullptr;

	if ( nullptr == consoleAppHost )
	{
		onReceiveMessage( { "$y[DevConsoleApp] $rFailed to create host" } );
		return false;
	}

	state = State::Connecting;
	networkThread = std::thread( [this]()
		{
			// There needs to be a delay here, otherwise it'll crash
			Wait( 1.0f );
			while ( state != State::Inactive )
			{
				Update();
			}
		} );

	return true;
}

// ============================
// Network::Shutdown
// ============================
void Network::Shutdown()
{
	state = State::Inactive;
	networkThread.join();
	UpdateWhileDisconnecting();

	consoleAppHost = nullptr;
	consoleBridgePeer = nullptr;

	onReceiveMessage = nullptr;
	onReceiveAutocomplete = nullptr;

	enet_host_destroy( consoleAppHost );
	enet_deinitialize();
}

// ============================
// Network::Update
// ============================
void Network::Update()
{
	switch ( state )
	{
	case State::Connecting: return UpdateWhileConnecting();
	case State::Connected: return UpdateWhileConnected();
	case State::Disconnecting: return UpdateWhileDisconnecting();
	}
}

// ============================
// Network::RequestAutocompleteUpdate
// ============================
void Network::RequestAutocompleteUpdate( std::string_view command )
{
	requestAutocomplete = true;
	autocompleteCommand = command;
}

// ============================
// Network::SubmitCommand
// ============================
void Network::SubmitCommand( std::string_view command )
{
	this->command = command;
	sendCommand = true;
}

// ============================
// Network::UpdateWhileConnecting
// ============================
void Network::UpdateWhileConnecting()
{
	ENetEvent netEvent{};
	onReceiveMessage( { "$y[DevConsoleApp] Trying connection... (127.0.0.1:23005)", Now() } );
	
	if ( nullptr == consoleBridgePeer )
	{
		consoleBridgePeer = enet_host_connect( consoleAppHost, &consoleBridgeAddress, 1, 0 );
	}

	if ( enet_host_service( consoleAppHost, &netEvent, 1500 ) > 0 )
	{
		onReceiveMessage( { "$y[DevConsoleApp] $gSuccessfully connected to an instance of Elegy Engine", Now() } );
		state = State::Connected;
		return;
	}
	
	onReceiveMessage( { "$y[DevConsoleApp] Connection failed", Now() } );
	Wait( 0.5f );

	enet_peer_reset( consoleBridgePeer );
	consoleBridgePeer = nullptr;
}

// ============================
// Network::UpdateWhileConnected
// ============================
void Network::UpdateWhileConnected()
{
	if ( sendCommand )
	{
		std::vector<byte> packetBytes = EncodeMessage( command );

		ENetPacket* packet = enet_packet_create( packetBytes.data(), packetBytes.size(), ENET_PACKET_FLAG_RELIABLE);
		enet_host_broadcast( consoleAppHost, 0, packet );

		sendCommand = false;
	}

	int numMessagesAdded = 0;
	ENetEvent netEvent{};
	while ( enet_host_service( consoleAppHost, &netEvent, 0 ) > 0 )
	{
		if ( netEvent.type == ENET_EVENT_TYPE_RECEIVE )
		{
			const auto* data = netEvent.packet->data;
			if ( data[0] == 'X' )
			{
				onReceiveMessage( { "$y[DevConsoleApp] Disconnected!" } );
				onReceiveAutocomplete( {} );
				state = State::Disconnecting;
				return;
			}
			else if ( data[0] == 'M' )
			{
				const int TypeOffset = 1;
				const int TimeOffset = TypeOffset + sizeof( byte );
				const int LengthOffset = TimeOffset + sizeof( float );
				const int TextOffset = LengthOffset + sizeof( uint16_t );

				ConsoleMessage message{};
				message.type = static_cast<ConsoleMessageType::Enum>( data[TypeOffset] );
				message.timeSubmitted = *reinterpret_cast<const float*>( &data[TimeOffset] );

				size_t messageLength = *reinterpret_cast<const uint16_t*>( &data[LengthOffset] );
				message.text = std::string( reinterpret_cast<const char*>( &data[TextOffset] ), messageLength );

				onReceiveMessage( message );
				
				// It's aesthetically pleasing to see them coming in batches
				if ( ++numMessagesAdded >= 15 )
				{
					Wait( 0.015f );
					numMessagesAdded = 0;
				}
			}
		}
		else if ( netEvent.type == ENET_EVENT_TYPE_DISCONNECT )
		{
			onReceiveMessage( { "$y[DevConsoleApp] Disconnected!" } );
			onReceiveAutocomplete( {} );
			state = State::Disconnecting;
			return;
		}
	}

	// Do not burn the CPU
	Wait( 0.1f );
}

// ============================
// Network::UpdateWhileDisconnecting
// ============================
void Network::UpdateWhileDisconnecting()
{
	if ( nullptr != consoleBridgePeer )
	{
		enet_peer_disconnect( consoleBridgePeer, 0 );
	}

	ENetEvent netEvent{};
	for ( int i = 0; i < 10; i++ )
	{
		enet_host_service( consoleAppHost, &netEvent, 5 );
	}

	state = State::Connecting;
}

// ============================
// Network::EncodeMessage
// ============================
std::vector<byte> Network::EncodeMessage( std::string_view message )
{
	std::vector<byte> bytes;
	bytes.reserve( 1 + 1 + message.size() );

	// 1st byte: message type (C = concommand)
	bytes.push_back( 'C' );
	// 3rd byte: string length
	bytes.push_back( message.size() );

	// rest: string data
	for ( int i = 0; i < message.size(); i++ )
	{
		bytes.push_back( message[i] );
	}

	return bytes;
}
