// SPDX-FileCopyrightText: 2023 Admer Šuko
// SPDX-License-Identifier: MIT

#pragma once

struct ConsoleMessageType final
{
	enum Enum
	{
		Info = 0,
		Developer = 1,
		Verbose = 2,
		Warning = 3,
		Error = 4,
		Fatal = 5
	};
};

struct ConsoleMessage
{
	ConsoleMessage( std::string messageText = "", float messageTime = 0.0f, ConsoleMessageType::Enum messageType = ConsoleMessageType::Info )
		: text( messageText ), timeSubmitted( messageTime ), type( messageType )
	{
	}

	ConsoleMessage( const ConsoleMessage& message ) = default;
	ConsoleMessage( ConsoleMessage&& message ) = default;

	ConsoleMessage& operator=( const ConsoleMessage& message ) = default;
	ConsoleMessage& operator=( ConsoleMessage&& message ) = default;

	std::string text;
	float timeSubmitted;
	ConsoleMessageType::Enum type;
};
