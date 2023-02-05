// SPDX-FileCopyrightText: 2023 Admer Šuko
// SPDX-License-Identifier: MIT

#include "Precompiled.hpp"

#include "Network/Network.hpp"
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
	Network net{};

	view.Init( [&]( std::string_view command )
		{
			net.SubmitCommand( command );
		},
		
		[&]( std::string_view command )
		{
			net.RequestAutocompleteUpdate( command );
		} );

	Wait( 0.1f );

	const bool result = net.Init( [&]( const ConsoleMessage& message )
		{
			view.OnLog( message );
		},

		[&]( const std::vector<std::string>& autocompleteStrings )
		{
			view.SetAutocompleteBuffer( autocompleteStrings );
		} );

	if ( !result )
	{
		Wait( 0.5f );
		view.Shutdown();
		return -1;
	}

	while ( view.OnUpdate( UpdateInterval ) )
	{
		Wait( UpdateInterval );
	}

	net.Shutdown();
	view.OnLog( { "$y[DevConsoleApp] $gGracefully shutting down..." } );
	view.Shutdown();
	return 0;
}
