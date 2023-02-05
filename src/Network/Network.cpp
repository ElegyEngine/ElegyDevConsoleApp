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
	onReceiveMessage( { "$y[DevConsoleApp] Trying connection... (127.0.0.1:23005)" } );
	
	if ( nullptr == consoleBridgePeer )
	{
		consoleBridgePeer = enet_host_connect( consoleAppHost, &consoleBridgeAddress, 1, 0 );
	}

	if ( enet_host_service( consoleAppHost, &netEvent, 1500 ) > 0 )
	{
		onReceiveMessage( { "$y[DevConsoleApp] $gSuccessfully connected to an instance of Elegy Engine" } );
		state = State::Connected;
		return;
	}
	
	onReceiveMessage( { "$y[DevConsoleApp] Connection failed" } );
	enet_peer_reset( consoleBridgePeer );
	consoleBridgePeer = nullptr;
}

// ============================
// Network::UpdateWhileConnected
// ============================
void Network::UpdateWhileConnected()
{
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
				ConsoleMessage message{};
				message.type = static_cast<ConsoleMessageType::Enum>( data[1] );
				message.timeSubmitted = 0.0f;

				size_t messageLength = size_t( data[2] ) + (size_t( data[3] ) << 8);
				message.text = std::string( reinterpret_cast<const char*>(&data[4]), messageLength );

				onReceiveMessage( message );
			}
		}
	}
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
		enet_host_service( consoleAppHost, &netEvent, 20 );
	}

	state = State::Connecting;
}