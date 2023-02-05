// SPDX-FileCopyrightText: 2023 Admer Šuko
// SPDX-License-Identifier: MIT

#include "Precompiled.hpp"

#include "View/ConsoleView.hpp"

namespace chrono = std::chrono;
namespace this_thread = std::this_thread;

constexpr float UpdateInterval = 1.0f / 20.0f;
constexpr float PingInterval = 1.0f;

void Wait( float seconds )
{
	this_thread::sleep_for( chrono::milliseconds( int( seconds * 1000.0f ) ) );
}

int main()
{
	ConsoleView view{};
	view.Init();

	if ( enet_initialize() < 0 )
	{
		view.OnLog( { "$r[DevConsoleApp] [FATAL] Failed to initialise ENet", 0.0f, ConsoleMessageType::Fatal } );
		Wait( 0.5f );
		view.Shutdown();
		return -1;
	}

	// Currently hardcoded to a single port
	ENetAddress consoleBridgeAddress{};
	enet_address_set_host_ip( &consoleBridgeAddress, "127.0.0.1" );
	consoleBridgeAddress.port = 23005;

	ENetHost* consoleAppHost = enet_host_create( nullptr, 1, 2, 0, 0 );
	ENetPeer* consoleBridgePeer = nullptr;

	bool networkConnected = false;
	float networkScanTimer = 2.0f;

	while ( view.OnUpdate( UpdateInterval ) )
	{
		Wait( UpdateInterval );

		networkScanTimer -= UpdateInterval;
		if ( networkScanTimer <= 0.0f )
		{
			if ( !networkConnected )
			{
				view.OnLog( { "$y[DevConsoleApp] Trying connection... (127.0.0.1:23005)" } );
				
				if ( nullptr == consoleBridgePeer )
				{
					consoleBridgePeer = enet_host_connect( consoleAppHost, &consoleBridgeAddress, 1, 0 );

					ENetEvent netEvent;
					if ( enet_host_service( consoleAppHost, &netEvent, 2000 ) > 0 )
					{
						view.OnLog( { "$y[DevConsoleApp] $gSuccessfully connected to an instance of Elegy Engine" } );
						networkConnected = true;
					}
					else
					{
						view.OnLog( { "$y[DevConsoleApp] Connection failed" } );
						enet_peer_reset( consoleBridgePeer );
						consoleBridgePeer = nullptr;
					}
				}

				if ( nullptr != consoleBridgePeer )
				{
					switch ( consoleBridgePeer->state )
					{
					case ENET_PEER_STATE_DISCONNECTED: view.OnLog( { "Debug: ENET_PEER_STATE_DISCONNECTED" } ); break;
					case ENET_PEER_STATE_CONNECTING: view.OnLog( { "Debug: ENET_PEER_STATE_CONNECTING" } ); break;
					case ENET_PEER_STATE_ACKNOWLEDGING_CONNECT: view.OnLog( { "Debug: ENET_PEER_STATE_ACKNOWLEDGING_CONNECT" } ); break;
					case ENET_PEER_STATE_CONNECTION_PENDING: view.OnLog( { "Debug: ENET_PEER_STATE_CONNECTION_PENDING" } ); break;
					case ENET_PEER_STATE_CONNECTION_SUCCEEDED: view.OnLog( { "Debug: ENET_PEER_STATE_CONNECTION_SUCCEEDED" } ); break;
					case ENET_PEER_STATE_CONNECTED: view.OnLog( { "Debug: ENET_PEER_STATE_CONNECTED" } ); break;
					case ENET_PEER_STATE_DISCONNECT_LATER: view.OnLog( { "Debug: ENET_PEER_STATE_DISCONNECT_LATER" } ); break;
					case ENET_PEER_STATE_DISCONNECTING: view.OnLog( { "Debug: ENET_PEER_STATE_DISCONNECTING" } ); break;
					case ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT: view.OnLog( { "Debug: ENET_PEER_STATE_ACKNOWLEDGING_DISCONNECT" } ); break;
					case ENET_PEER_STATE_ZOMBIE: view.OnLog( { "Debug: ENET_PEER_STATE_ZOMBIE" } ); break;
					}
				}
			}

			// When connected to a console bridge, update at 20 Hz
			// When seeking for them, update at 1 Hz
			networkScanTimer = networkConnected ? UpdateInterval : PingInterval;

			if ( networkConnected )
			{
				ENetEvent netEvent;
				enet_host_service( consoleAppHost, &netEvent, 0 );
			}
		}
	}

	view.OnLog( { "$y[DevConsoleApp] $gGracefully shutting down..." } );
	
	ENetEvent netEvent;
	enet_peer_disconnect_now( consoleBridgePeer, 0 );

	for ( int i = 0; i < 10; i++ )
	{
		view.OnUpdate( UpdateInterval );
		Wait( UpdateInterval * 0.5f );
		enet_host_service( consoleAppHost, &netEvent, UpdateInterval * 1000.0f * 0.5f );
	}
	
	enet_host_destroy( consoleAppHost );
	enet_deinitialize();

	view.Shutdown();
	return 0;
}
