// SPDX-FileCopyrightText: 2023 Admer Šuko
// SPDX-License-Identifier: MIT

#pragma once

struct ConsoleMessage;

class Network final
{
public:
	// Called whenever a log message is received
	using OnReceiveMessageFn = void( const ConsoleMessage& message );
	// Called whenever a list of autocomplete options is received
	using OnReceiveAutocompleteFn = void( const std::vector<std::string>& autocompleteStrings );

	enum class State
	{
		Inactive,
		Connecting,
		Connected,
		Disconnecting
	};

public:
	bool Init( std::function<OnReceiveMessageFn> receiveMessage,
		std::function<OnReceiveAutocompleteFn> receiveAutocomplete );
	void Shutdown();
	void Update();

	void RequestAutocompleteUpdate( std::string_view command );

	bool IsActive() const
	{
		return state == State::Inactive;
	}

	void SubmitCommand( std::string_view command );

private:
	void UpdateWhileConnecting();
	void UpdateWhileConnected();
	void UpdateWhileDisconnecting();

private:
	std::string command{};
	bool sendCommand{ false };
	std::string autocompleteCommand{};
	bool requestAutocomplete{ false };
	State state{ State::Inactive };

	std::thread networkThread;
	ENetAddress consoleBridgeAddress{};
	ENetHost* consoleAppHost{ nullptr };
	ENetPeer* consoleBridgePeer{ nullptr };

	std::function<OnReceiveMessageFn> onReceiveMessage{};
	std::function<OnReceiveAutocompleteFn> onReceiveAutocomplete{};
};
